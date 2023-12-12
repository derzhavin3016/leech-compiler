#ifndef LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED
#define LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED

#include "graph/dfs.hh"
#include "ir/basic_block.hh"
#include "loop_analyzer.hh"
#include <algorithm>
#include <iterator>
#include <stack>
#include <unordered_set>
#include <utility>
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

  explicit LivenessAnalyzer(const GraphTy &graph)
    : m_loops(graph), m_linearOrder(buildLinearOrder(graph))
  {}

  [[nodiscard]] const auto &getLinearOrder() const noexcept
  {
    return m_linearOrder;
  }

private:
  using VisitedSet = std::unordered_set<NodePtrTy>;

  [[nodiscard]] LinearOrderNodes buildLinearOrder(const GraphTy &graph) const
  {
    // Get post order
    const auto rpoOrder = graph::depthFirstSearchReversePostOrder(graph);
    using NodeIt = typename Traits::node_iterator;
    LinearOrderNodes res;
    VisitedSet visited;
    std::stack<std::pair<NodeIt, NodePtrTy>> toVisit;
    res.reserve(rpoOrder.size());

    auto &&visitNode = [&](NodePtrTy node) {
      toVisit.emplace(Traits::succBegin(node), node);
      visited.insert(node);
      res.push_back(node);
    };

    visitNode(Traits::entryPoint(graph));

    while (!toVisit.empty())
    {
      auto &&[first, parent] = toVisit.top();
      toVisit.pop();

      const auto endSucc = Traits::succEnd(parent);
      const auto unvisNode =
        std::find_if(first, endSucc, [&](const auto pNode) {
          if (visited.find(pNode) != visited.end())
            return false;

          // check for loop header
          if (const auto *const lInfo = m_loops.getLoopInfo(pNode);
              lInfo->getHeader() == pNode)
            return checkLoopHeader(lInfo, visited);

          return checkBB(pNode, visited);
        });

      if (unvisNode == endSucc)
        continue;

      toVisit.emplace(std::next(unvisNode), parent);

      visitNode(*unvisNode);
    }

    return res;
  }

  using LoopsTy = LoopAnalyzer<GraphTy>;
  using LoopInfo = typename LoopsTy::LoopInfo;

  template <typename Pred>
  static bool checkPreds(NodePtrTy node, Pred &&pred)
  {
    return std::all_of(Traits::predBegin(node), Traits::predEnd(node),
                       std::forward<Pred>(pred));
  }

  static bool checkLoopHeader(const LoopInfo *lInfo, const VisitedSet &visited)
  {
    // Irreducible loop - no check
    if (!lInfo->reducible())
      return true;

    // Reducible loop - check for external nodes visit state
    return checkPreds(lInfo->getHeader(), [&](auto pNode) {
      return visited.find(pNode) != visited.end() || lInfo->contains(pNode);
    });
  }

  static bool checkBB(NodePtrTy node, const VisitedSet &visited)
  {
    return checkPreds(
      node, [&](auto nd) { return visited.find(nd) != visited.end(); });
  }

  LoopsTy m_loops;
  LinearOrderNodes m_linearOrder;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED */
