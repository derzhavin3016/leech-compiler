#ifndef LEECH_JIT_INCLUDE_GRAPH_DSU_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DSU_HH_INCLUDED

#include "common/common.hh"

#include "graph/dom_tree_helpers.hh"
#include "graph_traits.hh"

namespace ljit::graph::detail
{
template <class GraphTy>
class [[nodiscard]] DSU final
{
public:
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

  explicit DSU(const DFSTimeToTimeMap &sdoms, const IdToDSFMap &rev)
    : m_sdoms(sdoms), m_rev(rev)
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
    LJIT_ASSERT(toFind != nullptr);
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
  [[nodiscard]] static auto getNodeId(NodePtrTy node)
  {
    return detail::getNodeId<GraphTy>(node);
  }

  [[nodiscard]] auto getSdomId(NodePtrTy node) const
  {
    return m_sdoms[m_rev.at(getNodeId(node))];
  }

  [[nodiscard]] auto &accessParent(NodePtrTy node) const
  {
    return m_parents.at(getNodeId(node));
  }
  [[nodiscard]] auto &accessParent(NodePtrTy node)
  {
    return m_parents[getNodeId(node)];
  }

  [[nodiscard]] auto &accessLabel(NodePtrTy node) const
  {
    return m_labels.at(getNodeId(node));
  }
  [[nodiscard]] auto &accessLabel(NodePtrTy node)
  {
    return m_labels[getNodeId(node)];
  }

  const DFSTimeToTimeMap &m_sdoms;
  const IdToDSFMap &m_rev;

  // parent of i’th node in the forest maintained during step 2 of the algorithm
  FromIdMap<NodePtrTy> m_parents;
  // At any point of time, label[i] stores the vertex v with minimum sdom, lying
  // on path from i to the root of the (dsu) tree in which node i lies
  FromIdMap<NodePtrTy> m_labels;
};
} // namespace ljit::graph::detail

#endif /* LEECH_JIT_INCLUDE_GRAPH_DSU_HH_INCLUDED */
