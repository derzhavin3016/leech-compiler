#ifndef LEECH_JIT_INCLUDE_INSTRUCTION_INST_HH_INCLUDED
#define LEECH_JIT_INCLUDE_INSTRUCTION_INST_HH_INCLUDED

#include <ostream>
#include <type_traits>

#include "common/opcodes.hh"
#include "intrusive_list/intrusive_list.hh"

namespace ljit
{

enum class Type
{
  None,
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

class Inst : public Value, public IntrusiveListNode<Inst>
{
protected:
  Inst() = default;
  explicit Inst(Type type) noexcept : Value(type)
  {}

public:
  virtual void print(std::ostream &ost) const = 0;
};

template <class... Args>
struct AlwaysFalse : std::false_type
{};

template <class T>
class ConstVal final : public Inst
{
  static_assert(AlwaysFalse<T>::value, "Unsupported const type");
};

#define LJIT_MAKE_CONST_CLASS(numBits)                                         \
  template <>                                                                  \
  class ConstVal<std::int##numBits##_t> final : public Inst                    \
  {                                                                            \
    using ValueT = std::int##numBits##_t;                                      \
                                                                               \
    ValueT m_value{};                                                          \
                                                                               \
  public:                                                                      \
    explicit ConstVal(ValueT val) : Inst(Type::I##numBits), m_value(val)       \
    {}                                                                         \
                                                                               \
    void print(std::ostream &ost) const override                               \
    {                                                                          \
      ost << "const." << magic_enum::enum_name(getType()) << ' ' << m_value    \
          << '\n';                                                             \
    }                                                                          \
  }

LJIT_MAKE_CONST_CLASS(8);
LJIT_MAKE_CONST_CLASS(16);
LJIT_MAKE_CONST_CLASS(32);
LJIT_MAKE_CONST_CLASS(64);

#undef LJIT_MAKE_CONST_CLASS

class BasicBlock;

class IfInstr final : public Inst
{
  Value *m_cond{nullptr};
  BasicBlock *m_true{nullptr};
  BasicBlock *m_false{nullptr};

public:
  IfInstr(Value *cond, BasicBlock *trueBB, BasicBlock *falseBB)
    : m_cond(cond), m_true(trueBB), m_false(falseBB)
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
  explicit JumpInstr(BasicBlock *target) : m_target(target)
  {}

  [[nodiscard]] auto getTarget() const noexcept
  {
    return m_target;
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

class BinInst : public Inst
{

public:
};

class BinOp final : public Inst
{
  enum class Oper
  {
    kAdd,
    kMul,
    kLE,
  };
  Oper m_oper{};
  Value *m_lhs{nullptr};
  Value *m_rhs{nullptr};

public:
  BinOp(Oper oper, Value *lhs, Value *rhs)
    : m_oper(oper), m_lhs(lhs), m_rhs(rhs)
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
  explicit Ret(Value *toRet) : m_toRet(toRet)
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
  Type m_destT{};
  Value *m_srcV{};

public:
  Cast(Type destT, Value *srcV) : m_destT(destT), m_srcV(srcV)
  {}

  [[nodiscard]] auto getDstTy() const noexcept
  {
    return m_destT;
  }

  [[nodiscard]] auto getSrc() const noexcept
  {
    return m_srcV;
  }

  void print([[maybe_unused]] std::ostream &ost) const override
  {}
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_INSTRUCTION_INST_HH_INCLUDED */
