#ifndef LEECH_JIT_INCLUDE_IR_INST_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_INST_HH_INCLUDED

#include <cstdint>
#include <ostream>
#include <type_traits>
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
  std::vector<Inst *> m_users{};

public:
  auto &users()
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

  auto &inputs()
  {
    return m_inputs;
  }

public:
  LJIT_NO_COPY_SEMANTICS(Inst);
  LJIT_NO_MOVE_SEMANTICS(Inst);
  virtual ~Inst() = default;

  void addInput(Value *val)
  {
    LJIT_ASSERT(val != nullptr);
    val->users().push_back(this);
    m_inputs.push_back(val);
  }

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
  };

private:
  Oper m_oper{};

public:
  BinOp(Oper oper, Value *lhs, Value *rhs)
    : Inst(rhs->getType(), InstType::kBinOp), m_oper(oper)
  {
    LJIT_ASSERT(lhs->getType() == rhs->getType());
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
  case InstType::kCast:
  case InstType::kPhi:
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

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_INST_HH_INCLUDED */
