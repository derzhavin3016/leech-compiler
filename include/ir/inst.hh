#ifndef LEECH_JIT_INCLUDE_INSTRUCTION_INST_HH_INCLUDED
#define LEECH_JIT_INCLUDE_INSTRUCTION_INST_HH_INCLUDED

#include <ostream>

#include "common/opcodes.hh"
#include "intrusive_list/intrusive_list.hh"

namespace ljit
{
class Inst : public IntrusiveListNode
{
  Opcodes m_opcode = Opcodes::UNKNOWN;

protected:
  explicit Inst(Opcodes opc) : m_opcode(opc)
  {}

public:
  [[nodiscard]] auto getOpcode() const noexcept
  {
    return m_opcode;
  }

  virtual void print(std::ostream &ost) const = 0;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_INSTRUCTION_INST_HH_INCLUDED */
