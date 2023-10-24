#ifndef LEECH_JIT_INCLUDE_GRAPH_DSU_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DSU_HH_INCLUDED

#include "common/common.hh"

#include "dom_tree_types.hh"
#include "graph_traits.hh"

namespace ljit::graph
{
template <class GraphTy>
class DSU final
{
public:
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = Traits::node_pointer;

  explicit DSU(const IdToIdMap &sdoms)
    : m_sdoms(sdoms), m_parents(sdoms.size()), m_labels(sdoms.size())
  {}

  LJIT_NO_COPY_SEMANTICS(DSU);
  LJIT_NO_MOVE_SEMANTICS(DSU);
  ~DSU() = default;

  [[nodiscard]] auto getParent(NodePtrTy node) const
  {
    return accessParent(node);
  }

  void setParent(NodePtrTy node, NodePtrTy par)
  {
    accessParent(node) = par;
  }

  [[nodiscard]] auto getLabel(NodePtrTy node) const
  {
    return accessLabel(node);
  }

  void setLabel(NodePtrTy node, NodePtrTy label)
  {
    accessLabel(node) = label;
  }

  [[nodiscard]] NodePtrTy find(NodePtrTy toFind)
  {
    auto &parent = accessParent(toFind);
    if (toFind == parent)
      return toFind;

    const auto found = find(parent);
    const auto parLabel = getLabel(parent);
    auto &toFindLabel = accessLabel(toFind);

    if (getSdomId(parLabel) < getSdomId(toFindLabel))
      toFindLabel = parLabel;

    parent = found;

    return toFindLabel;
  }

  void unite(NodePtrTy node, NodePtrTy parent)
  {
    setParent(node, parent);
  }

private:
  [[nodiscard]] IdToIdMap::value_type getSdomId(NodePtrTy node) const
  {
    return m_sdoms[Traits::id(node)];
  }

  [[nodiscard]] auto &accessParent(NodePtrTy node) const
  {
    return m_parents[Traits::id(node)];
  }
  [[nodiscard]] auto &accessParent(NodePtrTy node)
  {
    return m_parents[Traits::id(node)];
  }

  [[nodiscard]] auto &accessLabel(NodePtrTy node) const
  {
    return m_labels[Traits::id(node)];
  }
  [[nodiscard]] auto &accessLabel(NodePtrTy node)
  {
    return m_labels[Traits::id(node)];
  }

  const IdToIdMap &m_sdoms;

  // parent of i’th node in the forest maintained during step 2 of the algorithm
  IdToNodeMap<NodePtrTy> m_parents;
  // At any point of time, label[i] stores the vertex v with minimum sdom, lying
  // on path from i to the root of the (dsu) tree in which node i lies
  IdToNodeMap<NodePtrTy> m_labels;
};
} // namespace ljit::graph

#endif /* LEECH_JIT_INCLUDE_GRAPH_DSU_HH_INCLUDED */
