#ifndef LEECH_JIT_INCLUDE_ANALYSIS_LOOP_ANALYZER_HH_INCLUDED
#define LEECH_JIT_INCLUDE_ANALYSIS_LOOP_ANALYZER_HH_INCLUDED

#include <algorithm>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "intrusive_list/intrusive_list.hh"

#include "common/common.hh"
#include "graph/dfs.hh"
#include "graph/dom_tree.hh"
#include "graph/graph_traits.hh"

namespace ljit
{

template <class GraphTy>
class LoopAnalyzer final
{
private:
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

public:
  class LoopInfo;
  using NodesToLoop = std::unordered_map<NodePtrTy, LoopInfo *>;
  class LoopInfo final : public IListNode
  {
    NodePtrTy m_header{};
    std::unordered_set<NodePtrTy> m_body{};
    LoopInfo *m_outer = nullptr;
    std::vector<LoopInfo *> m_inners{};

    bool m_reducible = false;
    bool m_root = false;

    void setOuter(LoopInfo *outer) noexcept
    {
      m_outer = outer;
    }

    friend class LoopAnalyzer;

  public:
    LoopInfo(NodePtrTy header, bool reducible, bool root = false)
      : m_header(header), m_reducible(reducible), m_root(root)
    {}

    [[nodiscard]] bool reducible() const noexcept
    {
      return m_reducible;
    }

    [[nodiscard]] bool isRoot() const noexcept
    {
      return m_root;
    }

    void addNode(NodePtrTy node)
    {
      m_body.insert(node);
    }

    [[nodiscard]] auto getOuterLoop() const noexcept
    {
      return m_outer;
    }

    [[nodiscard]] const auto &getInners() const noexcept
    {
      return m_inners;
    }

    [[nodiscard]] auto contains(NodePtrTy node) const
    {
      return node == m_header || m_body.find(node) != m_body.end();
    }

  private:
    void addInnerLoop(LoopInfo *inner)
    {
      m_inners.push_back(inner);
      inner->setOuter(this);
    }

    void populate(NodesToLoop &nodesToLoop)
    {
      // Associate all sources of back edges w/ this loop
      for (const auto &backSrc : m_body)
        nodesToLoop.emplace(backSrc, this);

      // If it is irreducible, that's all
      if (!m_reducible)
        return;

      using NodeIt = typename Traits::node_iterator;
      // Fill loop's body
      std::stack<std::pair<NodeIt, NodePtrTy>> toVisit;
      std::unordered_set<NodePtrTy> visited;

      auto &&vis = [&](const NodePtrTy node) {
        toVisit.emplace(Traits::predBegin(node), node);
        visited.insert(node);
      };

      std::for_each(m_body.cbegin(), m_body.cend(), vis);

      // DFS loop
      while (!toVisit.empty())
      {
        auto [first, child] = toVisit.top();
        toVisit.pop();

        const auto predEnd = Traits::predEnd(child);
        const auto unvisNode =
          std::find_if(first, predEnd, [&, this](auto node) {
            if (node == m_header || visited.find(node) != visited.end())
              return false;

            const auto [it, wasNew] = nodesToLoop.emplace(node, this);

            if (wasNew)
            {
              // Add to body
              addNode(it->first);
            }
            else
            {
              // Link loops
              addInnerLoop(it->second);
            }

            return true;
          });

        if (unvisNode == predEnd)
          continue;

        toVisit.emplace(std::next(unvisNode), child);
        vis(*unvisNode);
      }
    }
  };

  using Nodes = std::vector<NodePtrTy>;

  LoopAnalyzer() = default;

  explicit LoopAnalyzer(const GraphTy &graph)
  {
    Nodes nonHeadNodes;
    Nodes loopHeadPostOrder;

    m_nodesToLoop = collectBackEdges(graph, loopHeadPostOrder, nonHeadNodes);
    std::for_each(loopHeadPostOrder.crbegin(), loopHeadPostOrder.crend(),
                  [&](const auto node) {
                    const auto found = m_nodesToLoop.find(node);
                    LJIT_ASSERT(found != m_nodesToLoop.end());

                    found->second->populate(m_nodesToLoop);
                  });

    if (nonHeadNodes.size() + loopHeadPostOrder.size() == m_nodesToLoop.size())
      return;

    loopHeadPostOrder.clear();

    auto *const rootLoop =
      &emplaceBackToList<LoopInfo>(m_loops, nullptr, false, true);

    // Put all free nodes to the root loop

    for (const auto &node : nonHeadNodes)
    {
      const auto [it, wasNew] = m_nodesToLoop.emplace(node, rootLoop);
      if (wasNew)
        rootLoop->addNode(it->first);
    }

    std::for_each(m_loops.begin(), m_loops.end(), [&](auto &loop) {
      if (&loop != rootLoop && loop.getOuterLoop() == nullptr)
        rootLoop->addInnerLoop(&loop);
    });
  }

  [[nodiscard]] const auto *getLoopInfo(NodePtrTy node) const
  {
    const auto found = m_nodesToLoop.find(node);
    LJIT_ASSERT(found != m_nodesToLoop.end());
    return found->second;
  }

private:
  class Visitor final : public graph::DFSVisitor<GraphTy>
  {
    NodesToLoop &m_toLoop;
    LoopAnalyzer &m_analyzer;
    Nodes &m_postOrder;
    Nodes &m_allNodes;
    const graph::DominatorTree<GraphTy> &m_domTree;

  public:
    Visitor(NodesToLoop &toLoops, LoopAnalyzer &analyzer, Nodes &postOrder,
            Nodes &allNodes, const graph::DominatorTree<GraphTy> &domTree)
      : m_toLoop(toLoops),
        m_analyzer(analyzer),
        m_postOrder(postOrder),
        m_allNodes(allNodes),
        m_domTree(domTree)
    {}

    void finishNode(NodePtrTy node)
    {
      if (m_toLoop.find(node) != m_toLoop.end())
      {
        m_postOrder.push_back(node);
      }
      else
      {
        m_allNodes.push_back(node);
      }
    }

    void backEdge(NodePtrTy src, NodePtrTy tar)
    {
      const bool isReducible = m_domTree.isDominator(tar, src);
      auto *const pLoopInfo =
        &emplaceBackToList<LoopInfo>(m_analyzer.m_loops, tar, isReducible);

      m_toLoop.emplace(tar, pLoopInfo).first->second->addNode(src);
    }
  };

  [[nodiscard]] NodesToLoop collectBackEdges(const GraphTy &graph,
                                             Nodes &postOrder, Nodes &allNodes)
  {
    const auto domTree = graph::buildDomTree(graph);
    NodesToLoop toLoops;
    graph::depthFirstSearch(
      graph, Visitor{toLoops, *this, postOrder, allNodes, domTree});
    return toLoops;
  }

  NodesToLoop m_nodesToLoop;
  IList<LoopInfo> m_loops;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_ANALYSIS_LOOP_ANALYZER_HH_INCLUDED */
