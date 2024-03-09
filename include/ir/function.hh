#ifndef LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED

#include "basic_block.hh"
#include "common/common.hh"
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
    const auto idx = m_bbs.empty() ? 0ULL : m_bbs.back().getId() + 1;
    return &emplaceBackToList<BasicBlock>(m_bbs, idx);
  }

  auto appendParam(Type type)
  {
    return &emplaceBackToList<Param>(m_params, type);
  }

  [[nodiscard]] auto makeBBGraph() const noexcept
  {
    LJIT_ASSERT(!m_bbs.empty());
    return BasicBlockGraph{const_cast<BasicBlock *>(&m_bbs.front()),
                           m_bbs.size()};
  }
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED */
