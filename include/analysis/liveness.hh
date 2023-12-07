#ifndef LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED
#define LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED

#include "graph/dfs.hh"
#include "ir/basic_block.hh"
#include "loop_analyzer.hh"
#include <unordered_set>
#include <vector>

namespace ljit
{

class LivenessAnalyzer final
{
public:
  using GraphTy = BasicBlockGraph;
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

private:
  using LinearOrderNodes = std::vector<NodePtrTy>;

public:
  LJIT_NO_COPY_SEMANTICS(LivenessAnalyzer);
  LJIT_NO_MOVE_SEMANTICS(LivenessAnalyzer);
  ~LivenessAnalyzer() = default;

  explicit LivenessAnalyzer(const GraphTy &graph) : m_loops(graph)
  {
    const auto linearOrder = buildLinearOrder();
  }

private:
  [[nodiscard]] LinearOrderNodes buildLinearOrder(const GraphTy &graph) const
  {
    // Get post order
    const auto rpoOrder = graph::depthFirstSearchReversePostOrder(graph);
    LinearOrderNodes res;
    std::unordered_set<NodePtrTy> visited;
    res.reserve(rpoOrder.size());
    // So we only need to rearrange false branches in optimizer way


    for (const auto *const node : rpoOrder)
    {
      if (!visited.insert(node).second)
        continue;


    }
  }

  LoopAnalyzer<GraphTy> m_loops;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED */
