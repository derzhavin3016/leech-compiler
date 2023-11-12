#ifndef LEECH_JIT_INCLUDE_ANALYSIS_LOOP_ANALYZER_HH_INCLUDED
#define LEECH_JIT_INCLUDE_ANALYSIS_LOOP_ANALYZER_HH_INCLUDED

#include "graph/dfs.hh"
#include "graph/dom_tree.hh"
#include "graph/graph_traits.hh"
#include <unordered_map>
#include <unordered_set>

namespace ljit
{

template <class GraphTy>
class LoopAnalyzer final
{
private:
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

  class LoopInfo final
  {
    NodePtrTy m_header{};
    std::unordered_set<NodePtrTy> m_body{};

    bool m_reducible = false;

  public:
    LoopInfo(NodePtrTy header, bool reducible)
      : m_header(header), m_reducible(reducible)
    {}

    void addNode(NodePtrTy node)
    {
      m_body.insert(node);
    }

    [[nodiscard]] auto contains(NodePtrTy node) const
    {
      return node == m_header || m_body.find(node) != m_body.end();
    }
  };

public:
  explicit LoopAnalyzer(const GraphTy &graph)
    : m_domTree(graph::buildDomTree(graph))
  {
    auto &&loopsMap = collectBackEdges(graph);
  }

private:
  using LoopsMap = std::unordered_map<NodePtrTy, LoopInfo>;
  class Visitor final : public graph::DFSVisitor<GraphTy>
  {
    LoopsMap &m_info;
    const LoopAnalyzer &m_analyzer;

  public:
    Visitor(LoopsMap &info, const LoopAnalyzer &analyzer)
      : m_info(info), m_analyzer(analyzer)
    {}

    void backEdge(NodePtrTy src, NodePtrTy tar)
    {
      const bool isReducible = m_analyzer.m_domTree.isDominator(tar, src);
      const auto &itBool = m_info.try_emplace(tar, tar, isReducible);

      itBool.first->second.addNode(src);
    }
  };

  [[nodiscard]] LoopsMap collectBackEdges(const GraphTy &graph) const
  {
    LoopsMap loops;
    graph::depthFirstSearch(graph, Visitor{loops, *this});
    return loops;
  }

  graph::DominatorTree<GraphTy> m_domTree;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_ANALYSIS_LOOP_ANALYZER_HH_INCLUDED */
