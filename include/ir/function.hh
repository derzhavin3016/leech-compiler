#ifndef LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED

#include <vector>

#include "basic_block.hh"
#include "inst.hh"

namespace ljit
{

class Function final
{
  std::vector<BasicBlock> m_bbs;
  // std::vector<Param>
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED */
