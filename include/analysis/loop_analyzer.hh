#ifndef LEECH_JIT_INCLUDE_ANALYSIS_LOOP_ANALYZER_HH_INCLUDED
#define LEECH_JIT_INCLUDE_ANALYSIS_LOOP_ANALYZER_HH_INCLUDED

#include "graph/graph_traits.hh"

namespace ljit
{

template <class GraphTy>
class LoopAnalyzer final
{
public:
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

  LoopAnalyzer() = default;

private:
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_ANALYSIS_LOOP_ANALYZER_HH_INCLUDED */
