#ifndef LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED

#include <list>

#include "basic_block.hh"
#include "inst.hh"
#include "intrusive_list/intrusive_list.hh"

namespace ljit
{

struct Param final : public Value, public IListNode
{
  explicit Param(Type type) : Value(type)
  {}
};

class Function final
{
  IList<BasicBlock> m_bbs;
  IList<Param> m_params;

public:
  auto *appendBB()
  {
    return &emplaceBackToList<BasicBlock>(m_bbs);
  }

  auto appendParam(Type type)
  {
    return &emplaceBackToList<Param>(m_params, type);
  }
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED */
