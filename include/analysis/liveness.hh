#ifndef LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED
#define LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED

#include <algorithm>
#include <iterator>
#include <stack>
#include <unordered_set>
#include <utility>
#include <vector>

#include "common/common.hh"
#include "graph/dfs.hh"
#include "ir/basic_block.hh"
#include "loop_analyzer.hh"

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

  explicit LivenessAnalyzer(const GraphTy &graph)
    : m_loops(graph), m_linearOrder(buildLinearOrder(graph))
  {}

  [[nodiscard]] const auto &getLinearOrder() const noexcept
  {
    return m_linearOrder;
  }

private:
  using VisitedSet = std::unordered_set<NodePtrTy>;
  using LoopsTy = LoopAnalyzer<GraphTy>;
  using LoopInfo = typename LoopsTy::LoopInfo;

  template <typename OutIt>
  void traverseLoop(const LoopInfo *lInfo, VisitedSet &visited, OutIt out) const
  {
    auto &&body = lInfo->getLinearOrder();

    for (const auto *bb : body)
    {
      *out++ = bb;
      visited.insert(bb);
    }
  }

  [[nodiscard]] LinearOrderNodes buildLinearOrder(const GraphTy &graph) const
  {
    // Get post order
    const auto rpoOrder = graph::depthFirstSearchReversePostOrder(graph);

    using NodeIt = typename Traits::node_iterator;

    LinearOrderNodes res;
    VisitedSet visited;
    std::stack<std::pair<NodeIt, NodePtrTy>> toVisit;
    res.reserve(rpoOrder.size());

    for (const auto *bb : rpoOrder)
    {
      if (visited.find(bb) != visited.end())
        continue;

      // Reducible loop
      if (const auto *loopInfo = m_loops.getLoopInfo(bb);
          loopInfo->getHeader() == bb && loopInfo->reducible())
      {
        traverseLoop(loopInfo, visited, std::back_inserter(res));
      }
      else
      {
        res.push_back(bb);
        visited.insert(bb);
      }
    }

    LJIT_ASSERT_MSG(res.size() == rpoOrder.size(),
                    "Missed some bb in Linear order");

    return res;
  }

  LoopsTy m_loops;
  LinearOrderNodes m_linearOrder;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED */
