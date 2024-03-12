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

class Value
{
  Type m_type{Type::None};

public:
  Value() = default;
  explicit Value(Type type) noexcept : m_type(type)
  {}
  [[nodiscard]] auto getType() const noexcept
  {
    return m_type;
  }
};

class BasicBlock;

class Inst : public Value, public IListNode
{
  InstType m_iType{};
  BasicBlock *m_bb{nullptr};
  std::size_t m_liveNum{};
  std::size_t m_linearNum{};

protected:
  explicit Inst(InstType iType) noexcept : m_iType(iType)
  {}
  explicit Inst(Type type, InstType iType) noexcept
    : Value(type), m_iType(iType)
  {}

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
    using ValueT = valTy;                                                      \
                                                                               \
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
  Value *m_cond{nullptr};
  BasicBlock *m_true{nullptr};
  BasicBlock *m_false{nullptr};

public:
  IfInstr(Value *cond, BasicBlock *trueBB, BasicBlock *falseBB)
    : Inst(InstType::kIf), m_cond(cond), m_true(trueBB), m_false(falseBB)
  {}

  [[nodiscard]] auto *getCond() const noexcept
  {
    return m_cond;
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
  };

private:
  Oper m_oper{};
  Value *m_lhs{nullptr};
  Value *m_rhs{nullptr};

public:
  BinOp(Oper oper, Value *lhs, Value *rhs)
    : Inst(rhs->getType(), InstType::kBinOp),
      m_oper(oper),
      m_lhs(lhs),
      m_rhs(rhs)
  {
    LJIT_ASSERT(lhs->getType() == rhs->getType());
  }

  [[nodiscard]] auto getOper() const noexcept
  {
    return m_oper;
  }
  [[nodiscard]] auto getLeft() const noexcept
  {
    return m_lhs;
  }
  [[nodiscard]] auto getRight() const noexcept
  {
    return m_rhs;
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class Ret final : public Inst
{
  Value *m_toRet{nullptr};

public:
  explicit Ret(Value *toRet) : Inst(InstType::kRet), m_toRet(toRet)
  {}

  [[nodiscard]] auto getVal() const noexcept
  {
    return m_toRet;
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class Cast final : public Inst
{
  Value *m_srcV{};

public:
  Cast(Type destT, Value *srcV) : Inst(destT, InstType::kCast), m_srcV(srcV)
  {}

  [[nodiscard]] auto getSrc() const noexcept
  {
    return m_srcV;
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_INST_HH_INCLUDED */
