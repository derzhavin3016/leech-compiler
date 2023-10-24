#ifndef LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HH_INCLUDED

#include "common/common.hh"
#include "dfs.hh"

#include "dom_tree_node.hh"
#include "graph/dsu.hh"
#include "graph/graph_traits.hh"
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace ljit::graph
{

template <class NodePtrTy>
class DominatorTree final
{
public:
  [[nodiscard]] bool isDominator(NodePtrTy dom, NodePtrTy node) const
  {
    const auto domIt = m_domMap.find(dom);

    if (domIt == m_domMap.end())
      return false;
    const auto &dommed = domIt->second;

    return std::find(dommed.begin(), dommed.end(), node) != dommed.end();
  }

private:
  template <class T>
  friend class DomTreeBuilder;

  DominatorTree() = default;

  std::unordered_map<NodePtrTy, std::vector<NodePtrTy>> m_domMap;
};

template <class GraphTy>
class DomTreeBuilder final
{
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = Traits::node_pointer;

  LJIT_NO_MOVE_SEMANTICS(DomTreeBuilder);
  LJIT_NO_COPY_SEMANTICS(DomTreeBuilder);

private:
  ~DomTreeBuilder() = default;

  explicit DomTreeBuilder(const GraphTy &graph)
    : m_dfsTimes(Traits::size(graph)),
      m_dfsParents(m_dfsTimes.size()),
      m_sdoms(m_dfsTimes.size()),
      m_idoms(m_dfsTimes.size()),
      m_sdommed(m_dfsTimes.size()),
      m_dsu(m_sdoms)
  {
    doDFS(graph);
    calcSdoms();
    calcIdoms();
  }

  [[nodiscard]] auto getDomTree() && noexcept
  {
    return std::move(m_domTree);
  }

public:
  [[nodiscard]] static auto build(const GraphTy &graph)
  {
    return DomTreeBuilder(graph).getDomTree();
  }

private:
  // mapping of i’th node to its new index, equal to the arrival time of node in
  // dfs tree
  IdToNodeMap<NodePtrTy> m_dfsTimes;
  // parent of node i in dfs tree
  IdToNodeMap<NodePtrTy> m_dfsParents;
  // label of semi-dominator of the i’th node
  IdToIdMap m_sdoms;
  // label of immediate-dominator of the i’th node
  IdToIdMap m_idoms;
  // For a vertex i, it stores a list of vertices for which i is the
  // semi-dominator
  std::vector<IdToNodeMap<NodePtrTy>> m_sdommed;

  DSU<GraphTy> m_dsu;
  DominatorTree<NodePtrTy> m_domTree;

  void doDFS(const GraphTy &graph)
  {
    depthFirstSearch(graph,
                     [this, prev = NodePtrTy{nullptr}](NodePtrTy node) mutable {
                       m_dfsTimes.push_back(node);
                       m_sdoms.push_back(m_sdoms.size());
                       m_idoms.push_back(m_idoms.size());

                       m_dsu.setParent(node, node);
                       m_dsu.setLabel(node, node);

                       m_dfsParents.push_back(prev);
                       prev = node;
                     });
  }

  void calcSdoms()
  {
    std::for_each(
      m_dfsTimes.rbegin(), m_dfsTimes.rend(), [this](NodePtrTy node) {
        auto &sdom = m_sdoms[Traits::id(node)];
        std::for_each(Traits::predBegin(node), Traits::predEnd(node),
                      [&, this](auto pred) {
                        sdom = std::min(sdom, m_sdoms[m_dsu.find(pred)]);
                      });
        const bool notFirst = node != m_dfsTimes.front();
        if (notFirst)
          m_sdommed[sdom].push_back(node);

        const auto &nodeDommed = m_sdommed[Traits::id(node)];
        std::for_each(
          nodeDommed.begin(), nodeDommed.end(), [this](const auto dominatee) {
            const auto dominateeId = Traits::id(dominatee);
            const auto minSdom = m_dsu.find(dominatee);
            const auto minSdomId = Traits::id(minSdom);
            const auto dominateeSdomId = m_sdoms[dominateeId];

            m_idoms[dominateeId] = dominateeSdomId == m_sdoms[minSdomId]
                                     ? dominateeSdomId
                                     : minSdomId;
          });

        if (notFirst)
          m_dsu.unite(node, m_dfsParents[Traits::id(node)]);
      });
  }

  void calcIdoms()
  {
    std::for_each(std::next(m_dfsTimes.begin()), m_dfsTimes.end(),
                  [this](const auto node) {
                    const auto nodeId = Traits::id(node);
                    auto &nodeIdomId = m_idoms[nodeId];
                    if (nodeIdomId != m_sdoms[nodeId])
                      nodeIdomId = m_idoms[nodeIdomId];
                  });
  }
};

} // namespace ljit::graph

#endif /* LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HH_INCLUDED */
