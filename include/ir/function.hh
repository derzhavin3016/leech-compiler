#ifndef LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED

#include <list>

#include "basic_block.hh"
#include "inst.hh"
#include "intrusive_list/intrusive_list.hh"

namespace ljit
{

struct Param final : public Value
{
  explicit Param(Type type) : Value(type)
  {}
};

class Function final
{
  IList<BasicBlock> m_bbs;
  std::list<Param> m_params;

public:
  auto *appendBB()
  {
    m_bbs.push_back(new BasicBlock{});
    return &m_bbs.back();
  }

  auto appendParam(Type type)
  {
    return &m_params.emplace_back(type);
  }
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED */
