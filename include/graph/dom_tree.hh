#ifndef LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HH_INCLUDED

#include "common/common.hh"
#include "dfs.hh"

#include "dom_tree_types.hh"
#include "graph/dsu.hh"
#include "graph/graph_traits.hh"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <unordered_map>
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
    const auto domId = Traits::id(dom);
    const auto found = m_tree.find(domId);
    if (found == m_tree.end())
      return false;

    const auto &dommed = found->second.getIDommed();

    return std::find(dommed.begin(), dommed.end(), node) != dommed.end();
  }

  void dump(std::ostream &ost) const
  {
    for (const auto &[domid, node] : m_tree)
      node.dump(ost);
  }

  DominatorTree() = default;

private:
  template <class T>
  friend class DomTreeBuilder;

  explicit DominatorTree(std::size_t sz) : m_tree(sz)
  {}

  std::unordered_map<std::size_t, DomTreeNode<NodePtrTy>> m_tree;
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
    : m_revIdMap(Traits::size(graph)),
      m_dfsParents(Traits::size(graph)),
      m_sdoms(Traits::size(graph)),
      m_idoms(Traits::size(graph)),
      m_sdommed(Traits::size(graph)),
      m_dsu(m_sdoms, m_revIdMap),
      m_domTree(Traits::size(graph))
  {
    m_dfsTimes.reserve(Traits::size(graph));
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
  std::vector<NodePtrTy> m_dfsTimes;
  // mapping of node id to DFS time
  IdToIdMap m_revIdMap;
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
  DominatorTree<GraphTy> m_domTree;

  template <class Cont, class OutIt>
  static void cpy(Cont cont, OutIt it)
  {
    std::copy(std::begin(cont), std::end(cont), it);
  }
#define DUMP() dump(__PRETTY_FUNCTION__)

  void dump(const char *func) const
  {
    auto &stream = std::cerr;
    stream << func << std::endl;
    std::ostream_iterator<std::size_t> sz{stream, " "};
    std::ostream_iterator<NodePtrTy> nd{stream, " "};
#define PRINT_M(member, where)                                                 \
  stream << #member << std::endl;                                              \
  cpy(member, where);                                                          \
  stream << std::endl;

    PRINT_M(m_dfsTimes, nd);
    PRINT_M(m_dfsParents, nd);
    stream << "revIdmap\nnodeId: ";
    for (std::size_t i = 0; i < m_revIdMap.size(); ++i)
      stream << i << " ";
    stream << "\ndfsTme: ";
    for (auto el : m_revIdMap)
      stream << el << " ";
    stream << std::endl;

    PRINT_M(m_sdoms, sz);
    PRINT_M(m_idoms, sz);
    for (std::size_t i = 0; i < m_sdommed.size(); ++i)
    {
      PRINT_M(m_sdommed[i], nd);
    }
  }
  void doDFS(const GraphTy &graph)
  {
    depthFirstSearch(
      graph, [this, prev = Traits::entryPoint(graph)](NodePtrTy node) mutable {
        const auto dfsTime = m_dfsTimes.size();

        m_dfsTimes.push_back(node);

        const auto nodeId = Traits::id(node);
        m_revIdMap[nodeId] = dfsTime;
        m_sdoms[dfsTime] = dfsTime;
        m_idoms[dfsTime] = dfsTime;

        m_dsu.setParent(node, node);
        m_dsu.setLabel(node, node);

        m_dfsParents[nodeId] = prev;
        prev = node;
      });
    DUMP();
  }

#define P(val) std::cerr << #val << " is " << (val) << std::endl

  void calcSdoms()
  {
    std::for_each(
      m_dfsTimes.rbegin(), m_dfsTimes.rend(), [this](const auto node) {
        const auto nodeId = Traits::id(node);
        const auto dfsTime = m_revIdMap[nodeId];
        auto &sdom = m_sdoms[dfsTime];
        std::for_each(
          Traits::predBegin(node), Traits::predEnd(node), [&, this](auto pred) {
            const auto tm = m_revIdMap[Traits::id(m_dsu.find(pred))];
            sdom = std::min(sdom, m_sdoms[tm]);
          });

        const bool notFirst = node != m_dfsTimes.front();

        if (notFirst)
          m_sdommed[Traits::id(m_dfsTimes[sdom])].push_back(node);

        for (const auto &dominatee : m_sdommed[nodeId])
        {
          const auto minSdom = m_dsu.find(dominatee);
          const auto dominateeId = Traits::id(dominatee);
          const auto dominateeTime = m_revIdMap[dominateeId];

          const auto minSdomId = Traits::id(minSdom);
          const auto minSdomTime = m_revIdMap[minSdomId];
          const auto dominateeSdomTime = m_sdoms[dominateeTime];

          m_idoms[dominateeTime] = (dominateeSdomTime == m_sdoms[minSdomTime])
                                     ? dominateeSdomTime
                                     : minSdomTime;
        }

        if (notFirst)
          m_dsu.unite(node, m_dfsParents[nodeId]);
      });
  }

  void calcIdoms()
  {
    std::for_each(std::next(m_dfsTimes.begin()), m_dfsTimes.end(),
                  [this, tm = std::size_t{1}](const auto node) mutable {
                    const auto nodeId = Traits::id(node);
                    const auto nodeTime = tm++;
                    auto &nodeIdomTime = m_idoms[nodeTime];

                    if (nodeIdomTime != m_sdoms[nodeTime])
                      nodeIdomTime = m_idoms[nodeIdomTime];

                    const auto idomNode = m_dfsTimes[nodeIdomTime];

                    m_domTree.m_tree[nodeId].setIDom(idomNode);
                    m_domTree.m_tree[Traits::id(idomNode)].addDommed(node);
                  });
    DUMP();
  }
};

template <class GraphTy>
[[nodiscard]] auto buildDomTree(const GraphTy &graph)
{
  return DomTreeBuilder<GraphTy>::build(graph);
}

} // namespace ljit::graph

#endif /* LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HH_INCLUDED */
