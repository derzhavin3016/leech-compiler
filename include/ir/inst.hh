#ifndef LEECH_JIT_INCLUDE_INSTRUCTION_INST_HH_INCLUDED
#define LEECH_JIT_INCLUDE_INSTRUCTION_INST_HH_INCLUDED

#include <ostream>

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

class Inst : public IntrusiveListNode<Inst>
{
  Type m_type{Type::None};

protected:
  explicit Inst(Type type) : m_type(type)
  {}

public:
  [[nodiscard]] auto getType() const noexcept
  {
    return m_type;
  }



  virtual void print(std::ostream &ost) const = 0;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_INSTRUCTION_INST_HH_INCLUDED */
