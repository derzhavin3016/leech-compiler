#ifndef LEECH_JIT_INCLUDE_IR_INST_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_INST_HH_INCLUDED

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <ostream>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "common/common.hh"
#include "intrusive_list/intrusive_list.hh"

namespace ljit
{

enum class InstType : std::uint8_t
{
  kUnknown,
  kIf,
  kConst,
  kJump,
  kBinOp,
  kRet,
  kCast,
  kPhi,
  kCall,
  kParam,
  kUnaryOp,
};

enum class Type
{
  None,
  I1,
  I8,
  I16,
  I32,
  I64,
};
class Inst;
class Value
{
public:
  enum class Category : bool
  {
    kInst,
    kParam,
  };

private:
  Type m_type{Type::None};
  Category m_cat{Category::kInst};
  std::unordered_set<Inst *> m_users{};

public:
  [[nodiscard]] auto &users()
  {
    return m_users;
  }

  [[nodiscard]] auto &users() const
  {
    return m_users;
  }

  Value() = default;

  explicit Value(Type type) noexcept : m_type(type)
  {}

  explicit Value(Type type, Category cat) noexcept : m_type(type), m_cat(cat)
  {}

  [[nodiscard]] auto getType() const noexcept
  {
    return m_type;
  }

  [[nodiscard]] bool isInst() const
  {
    return m_cat == Category::kInst;
  }
  [[nodiscard]] auto usersBegin() const
  {
    return m_users.begin();
  }

  [[nodiscard]] auto usersBegin()
  {
    return m_users.begin();
  }

  [[nodiscard]] auto usersEnd() const
  {
    return m_users.end();
  }

  [[nodiscard]] auto usersEnd()
  {
    return m_users.end();
  }

  void setUsersFrom(Value &other);
};

class BasicBlock;

class Inst : public Value, public IListNode
{
  InstType m_iType{};
  BasicBlock *m_bb{nullptr};
  std::size_t m_liveNum{};
  std::size_t m_linearNum{};
  std::vector<Value *> m_inputs{};

protected:
  explicit Inst(InstType iType) noexcept : m_iType(iType)
  {}
  explicit Inst(Type type, InstType iType) noexcept
    : Value(type), m_iType(iType)
  {}

  [[nodiscard]] auto &inputs()
  {
    return m_inputs;
  }

  [[nodiscard]] auto &inputs() const
  {
    return m_inputs;
  }

  void addInput(Value *val)
  {
    LJIT_ASSERT(val != nullptr);
    val->users().insert(this);
    m_inputs.push_back(val);
  }

public:
  LJIT_NO_COPY_SEMANTICS(Inst);
  LJIT_NO_MOVE_SEMANTICS(Inst);
  virtual ~Inst() = default;

  [[nodiscard]] auto getBB() const noexcept
  {
    return m_bb;
  }
  void setBB(BasicBlock *bb) noexcept
  {
    m_bb = bb;
  }
  void setLinearNum(std::size_t num)
  {
    m_linearNum = num;
  }
  void setLiveNum(std::size_t num)
  {
    m_liveNum = num;
  }

  [[nodiscard]] auto getLiveNum() const noexcept
  {
    return m_liveNum;
  }
  [[nodiscard]] auto getLinearNum() const noexcept
  {
    return m_linearNum;
  }

  [[nodiscard]] auto getInstType() const noexcept
  {
    return m_iType;
  }

  [[nodiscard]] auto inputAt(std::size_t idx) const
  {
    LJIT_ASSERT(idx < m_inputs.size());
    return m_inputs[idx];
  }

  void setInput(std::size_t idx, Value *newInput)
  {
    LJIT_ASSERT(idx < m_inputs.size());

    auto &inp = m_inputs[idx];
    if (std::count(m_inputs.begin(), m_inputs.end(), inp) == 1)
    {
      inp->users().erase(this);
    }
    inp = newInput;
    inp->users().insert(this);
  }

  void clearInputs()
  {
    for (auto *input : m_inputs)
    {
      [[maybe_unused]] const bool removed = input->users().erase(this) != 0;
      LJIT_ASSERT(removed);
    }

    m_inputs.clear();
  }

  void swapInputs(std::size_t lhs, std::size_t rhs)
  {
    LJIT_ASSERT(lhs < m_inputs.size());
    LJIT_ASSERT(rhs < m_inputs.size());
    std::swap(m_inputs[lhs], m_inputs[rhs]);
  }

  [[nodiscard]] auto inputBegin() const
  {
    return m_inputs.begin();
  }

  [[nodiscard]] auto inputBegin()
  {
    return m_inputs.begin();
  }

  [[nodiscard]] auto inputEnd() const
  {
    return m_inputs.end();
  }

  [[nodiscard]] auto inputEnd()
  {
    return m_inputs.end();
  }

  virtual void print(std::ostream &ost) const = 0;
};

inline void Value::setUsersFrom(Value &other)
{
  auto &usrs = users();
  usrs.merge(other.users());
  other.users().clear();

  for (auto *user : usrs)
  {
    std::replace(user->inputBegin(), user->inputEnd(), &other, this);
  }
}

template <class... Args>
struct AlwaysFalse : std::false_type
{};

template <class T>
class ConstVal final
{
  static_assert(AlwaysFalse<T>::value, "Unsupported const type");
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LJIT_MAKE_CONST_CLASS(valTy, ljitTy)                                   \
  template <>                                                                  \
  class ConstVal<valTy> final : public Inst                                    \
  {                                                                            \
  public:                                                                      \
    using ValueT = valTy;                                                      \
                                                                               \
  private:                                                                     \
    ValueT m_value{};                                                          \
                                                                               \
  public:                                                                      \
    explicit ConstVal(ValueT val)                                              \
      : Inst(Type::ljitTy, InstType::kConst), m_value(val)                     \
    {}                                                                         \
    [[nodiscard]] auto getVal() const noexcept                                 \
    {                                                                          \
      return m_value;                                                          \
    }                                                                          \
    void print(std::ostream &ost) const override                               \
    {                                                                          \
      ost << "const." << ' ' << m_value << '\n';                               \
    }                                                                          \
  };                                                                           \
  using ConstVal_##ljitTy = ConstVal<valTy>

LJIT_MAKE_CONST_CLASS(bool, I1);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LJIT_MAKE_INT_CONST_CLASS(numBits)                                     \
  LJIT_MAKE_CONST_CLASS(std::int##numBits##_t, I##numBits)

LJIT_MAKE_INT_CONST_CLASS(8);
LJIT_MAKE_INT_CONST_CLASS(16);
LJIT_MAKE_INT_CONST_CLASS(32);
LJIT_MAKE_INT_CONST_CLASS(64);

#undef LJIT_MAKE_CONST_CLASS
#undef LJIT_MAKE_INT_CONST_CLASS

class BasicBlock;

class IfInstr final : public Inst
{
  BasicBlock *m_true{nullptr};
  BasicBlock *m_false{nullptr};

public:
  IfInstr(Value *cond, BasicBlock *trueBB, BasicBlock *falseBB)
    : Inst(InstType::kIf), m_true(trueBB), m_false(falseBB)
  {
    addInput(cond);
  }

  [[nodiscard]] auto *getCond() const noexcept
  {
    return inputAt(0);
  }
  [[nodiscard]] auto *getTrueBB() const noexcept
  {
    return m_true;
  }
  [[nodiscard]] auto *getFalseBB() const noexcept
  {
    return m_false;
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class JumpInstr final : public Inst
{
  BasicBlock *m_target{nullptr};

public:
  explicit JumpInstr(BasicBlock *target)
    : Inst(InstType::kJump), m_target(target)
  {}

  [[nodiscard]] auto getTarget() const noexcept
  {
    return m_target;
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class Phi final : public Inst
{
  struct PhiEntry final
  {
    Value *m_val{nullptr};
    BasicBlock *bb{nullptr};
  };
  std::vector<PhiEntry> m_vars{};

public:
  using Entry = PhiEntry;

  explicit Phi(Type type) : Inst(type, InstType::kPhi)
  {}

  void addNode(Value *val, BasicBlock *bb)
  {
    m_vars.push_back(PhiEntry{val, bb});
    addInput(val);
  }

  [[nodiscard]] auto begin()
  {
    return m_vars.begin();
  }

  [[nodiscard]] auto begin() const
  {
    return m_vars.begin();
  }

  [[nodiscard]] auto end()
  {
    return m_vars.end();
  }

  [[nodiscard]] auto end() const
  {
    return m_vars.end();
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class UnaryOp final : public Inst
{
public:
  enum class Oper
  {
    kZeroCheck,
  };

private:
  Oper m_oper{};

public:
  UnaryOp(Oper oper, Value *val)
    : Inst(val->getType(), InstType::kUnaryOp), m_oper(oper)
  {
    addInput(val);
  }

  [[nodiscard]] auto getOper() const noexcept
  {
    return m_oper;
  }
  [[nodiscard]] auto getVal() const noexcept
  {
    return inputAt(0);
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class BinOp final : public Inst
{
public:
  enum class Oper
  {
    kAdd,
    kSub,
    kMul,
    kLE,
    kEQ,
    kShr,
    kOr,
    kBoundsCheck,
  };

private:
  Oper m_oper{};

public:
  BinOp(Oper oper, Value *lhs, Value *rhs)
    : Inst(rhs->getType(), InstType::kBinOp), m_oper(oper)
  {
    addInput(lhs);
    addInput(rhs);
  }

  [[nodiscard]] auto getOper() const noexcept
  {
    return m_oper;
  }
  [[nodiscard]] auto getLeft() const noexcept
  {
    return inputAt(0);
  }
  [[nodiscard]] auto getRight() const noexcept
  {
    return inputAt(1);
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class Ret final : public Inst
{
public:
  // Ret void
  Ret() : Inst(InstType::kRet)
  {}

  // Ret non-void
  explicit Ret(Value *toRet) : Inst(InstType::kRet)
  {
    addInput(toRet);
  }

  [[nodiscard]] auto getVal() const noexcept
  {
    return inputAt(0);
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class Param final : public Inst
{
  std::size_t m_idx{};

public:
  Param(std::size_t idx, Type type) : Inst(type, InstType::kParam), m_idx(idx)
  {}

  [[nodiscard]] auto getIdx() const noexcept
  {
    return m_idx;
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class Function;

class Call final : public Inst
{
  Function *m_callee{};

public:
  explicit Call(Function *callee);

  void appendArg(Value *arg)
  {
    addInput(arg);
  }

  [[nodiscard]] bool verify() const;

  [[nodiscard]] auto *getCallee() const
  {
    return m_callee;
  }
  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class Cast final : public Inst
{
public:
  Cast(Type destT, Value *srcV) : Inst(destT, InstType::kCast)
  {
    addInput(srcV);
  }

  [[nodiscard]] auto getSrc() const noexcept
  {
    return inputAt(0);
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

[[nodiscard]] inline bool producesValue(const Inst &inst)
{
  switch (inst.getInstType())
  {
  case InstType::kConst:
  case InstType::kBinOp:
  case InstType::kUnaryOp:
  case InstType::kCast:
  case InstType::kPhi:
  case InstType::kCall:
  case InstType::kParam:
    return true;
  case InstType::kJump:
  case InstType::kRet:
  case InstType::kIf:
    return false;
  case InstType::kUnknown:
  default:
    LJIT_UNREACHABLE("Unknown instruction");
  }
}

inline const Inst *tryRetrieveConst(const Value *val)
{
  // Check for instruction
  if (!val->isInst())
    return nullptr;

  const auto *const pInst = static_cast<const Inst *>(val);
  // Check for const
  if (pInst->getInstType() != InstType::kConst)
    return nullptr;

  return pInst;
}

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_INST_HH_INCLUDED */
