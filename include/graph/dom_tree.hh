#ifndef LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HH_INCLUDED

#include "common/common.hh"
#include "dfs.hh"

#include "dom_tree_helpers.hh"
#include "dom_tree_types.hh"
#include "graph/dsu.hh"
#include "graph/graph_traits.hh"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stack>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ljit::graph
{

template <class GraphTy>
class DominatorTree final
{
public:
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

  [[nodiscard]] bool isDominator(NodePtrTy dom, NodePtrTy node) const
  {
    if (node == dom)
      return true; // Every node dominates itself
    std::stack<detail::NodeIdTy> toVisit{};
    toVisit.push(detail::getNodeId<GraphTy>(dom));

    while (!toVisit.empty())
    {
      const auto id = toVisit.top();
      toVisit.pop();

      const auto found = m_tree.find(id);
      if (found == m_tree.end())
        continue;

      for (const auto &dommed : found->second.getIDommed())
      {
        if (node == dommed)
          return true;
        toVisit.push(detail::getNodeId<GraphTy>(dommed));
      }
    }

    return false;
  }

  [[nodiscard]] auto size() const noexcept
  {
    return m_tree.size();
  }

  void dump(std::ostream &ost) const
  {
    for (const auto &[domId, node] : m_tree)
    {
      ost << "Dominator: " << toUnderlying(domId) << '\n';
      ost << "Dominatees: ";
      for (const auto &dominatee : node.getIDommed())
        ost << Traits::id(dominatee) << ' ';
      ost << '\n';
    }
  }

  DominatorTree() = default;

private:
  template <class T>
  friend class DomTreeBuilder;

  std::unordered_map<detail::NodeIdTy, DomTreeNode<NodePtrTy>> m_tree;
};

template <class GraphTy>
class DomTreeBuilder final
{
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

  LJIT_NO_MOVE_SEMANTICS(DomTreeBuilder);
  LJIT_NO_COPY_SEMANTICS(DomTreeBuilder);

private:
  ~DomTreeBuilder() = default;

  explicit DomTreeBuilder(const GraphTy &graph)
    : m_sdoms(Traits::size(graph)), m_idoms(Traits::size(graph))
  {
    m_dfsTimes.reserve(Traits::size(graph));
    doDFS(graph);
    calcSdoms();
    calcIdoms();
  }

  [[nodiscard]] auto &&getDomTree() &&noexcept
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
  detail::FromDFSTimeMap<NodePtrTy> m_dfsTimes;
  // mapping of node id to DFS time
  detail::IdToDSFMap m_revIdMap;
  // parent of node i in dfs tree
  detail::FromIdMap<NodePtrTy> m_dfsParents;
  // label of semi-dominator of the i’th node
  detail::DFSTimeToTimeMap m_sdoms;
  // label of immediate-dominator of the i’th node
  detail::DFSTimeToTimeMap m_idoms;
  // For a vertex i, it stores a list of vertices for which i is the
  // semi-dominator
  detail::FromIdMap<std::vector<NodePtrTy>> m_sdommed;

  using DSUTy = detail::DSU<GraphTy>;

  DominatorTree<GraphTy> m_domTree;

  [[nodiscard]] static auto getNodeId(NodePtrTy node)
  {
    return detail::getNodeId<GraphTy>(node);
  }

  void dump(std::string_view func, std::ostream &stream = std::cerr) const
  {
    stream << func << '\n';

    // Dump dfsTimes
    stream << "DFSTimes:\n";
    m_dfsTimes.dump(stream);

    // Dump revIdmap
    stream << "revIdmap:\n";
    stream << "nodeId    dfsTime\n";
    stream << m_revIdMap;
    stream << '\n';

    stream << "Sdoms:\n";
    m_sdoms.dump(stream);

    stream << "Idoms:\n";
    m_idoms.dump(stream);

    stream << "Sdommed:\n";
    for (auto &&[nodeId, vec] : m_sdommed)
    {
      stream << nodeId << ":\n";
      std::copy(vec.begin(), vec.end(),
                std::ostream_iterator<NodePtrTy>{stream, "\n"});
    }
  }
  void doDFS(const GraphTy &graph)
  {
    depthFirstSearchPreOrder(
      graph, [this, prev = Traits::entryPoint(graph)](NodePtrTy node) mutable {
        const auto dfsTime = detail::toDFSTime(m_dfsTimes.size());

        m_dfsTimes.push_back(node);

        const auto nodeId = getNodeId(node);
        m_revIdMap[nodeId] = dfsTime;
        m_sdoms[dfsTime] = dfsTime;
        m_idoms[dfsTime] = dfsTime;

        m_dfsParents[nodeId] = prev;
        prev = node;
      });
  }

  [[nodiscard]] auto findMinSdom(NodePtrTy node, DSUTy &dsu)
  {
    const auto dfsTime = m_revIdMap[getNodeId(node)];
    auto &sdom = m_sdoms[dfsTime];
    std::for_each(Traits::predBegin(node), Traits::predEnd(node),
                  [&, this](auto pred) {
                    const auto tm = m_revIdMap[getNodeId(dsu.find(pred))];
                    sdom = std::min(sdom, m_sdoms[tm]);
                  });

    return sdom;
  }

  void fillIdoms(detail::NodeIdTy nodeId, DSUTy &dsu)
  {
    for (const auto &dominatee : m_sdommed[nodeId])
    {
      const auto minSdom = dsu.find(dominatee);

      const auto dominateeTime = m_revIdMap[getNodeId(dominatee)];

      const auto minSdomTime = m_revIdMap[getNodeId(minSdom)];
      const auto dominateeSdomTime = m_sdoms[dominateeTime];

      m_idoms[dominateeTime] = (dominateeSdomTime == m_sdoms[minSdomTime])
                                 ? dominateeSdomTime
                                 : minSdomTime;
    }
  }

  void calcSdoms()
  {
    DSUTy dsu{m_sdoms, m_revIdMap, m_dfsTimes};

    const auto rend = m_dfsTimes.rend();
    for (auto nodeIt = m_dfsTimes.rbegin(); nodeIt != rend; ++nodeIt)
    {
      const auto node = *nodeIt;

      const auto nodeId = getNodeId(node);
      const auto sdom = findMinSdom(node, dsu);

      const bool notFirst = node != m_dfsTimes.front();
      if (notFirst)
        m_sdommed[getNodeId(m_dfsTimes[sdom])].push_back(node);

      fillIdoms(nodeId, dsu);

      if (notFirst)
        dsu.unite(node, m_dfsParents[nodeId]);
    }
  }

  void calcIdoms()
  {
    const auto end = m_dfsTimes.end();
    for (auto nodeIt = std::next(m_dfsTimes.begin()); nodeIt != end; ++nodeIt)
    {
      const auto node = *nodeIt;

      const auto nodeTime = detail::toDFSTime(
        static_cast<std::size_t>(std::distance(m_dfsTimes.begin(), nodeIt)));
      auto &nodeIdomTime = m_idoms[nodeTime];

      if (nodeIdomTime != m_sdoms[nodeTime])
        nodeIdomTime = m_idoms[nodeIdomTime];

      const auto idomNode = m_dfsTimes[nodeIdomTime];

      const auto itBool =
        m_domTree.m_tree.try_emplace(getNodeId(idomNode), idomNode);

      itBool.first->second.addDommed(node);
    }
  }
};

template <class GraphTy>
[[nodiscard]] auto buildDomTree(const GraphTy &graph)
{
  return DomTreeBuilder<GraphTy>::build(graph);
}

} // namespace ljit::graph

#endif /* LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HH_INCLUDED */
