#ifndef LEECH_JIT_INCLUDE_OPT_INLINING_HH_INCLUDED
#define LEECH_JIT_INCLUDE_OPT_INLINING_HH_INCLUDED

#include "ir/basic_block.hh"

namespace ljit
{
class Inlining final
{
  using GraphTy = BasicBlockGraph;
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

public:
  void run(GraphTy &graph)
  {}
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_OPT_INLINING_HH_INCLUDED */
