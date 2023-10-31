#ifndef LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_TYPES_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_TYPES_HH_INCLUDED

#include <vector>

namespace ljit::graph
{
template <class NodePtrTy>
class DomTreeNode
{
  NodePtrTy m_idom{};
  std::vector<NodePtrTy> m_idommed{};

public:
  DomTreeNode() = default;
  explicit DomTreeNode(NodePtrTy idom) : DomTreeNode(idom)
  {}
  DomTreeNode(NodePtrTy idom, const std::vector<NodePtrTy> &idommed)
    : m_idom(idom), m_idommed(idommed)
  {}

  void setIDom(NodePtrTy idom) noexcept
  {
    m_idom = idom;
  }

  [[nodiscard]] auto getIDom() const noexcept
  {
    return m_idom;
  }

  [[nodiscard]] auto &getIDommed() const noexcept
  {
    return m_idommed;
  }

  void addDommed(NodePtrTy node)
  {
    m_idommed.push_back(node);
  }
};
} // namespace ljit::graph

#endif /* LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_TYPES_HH_INCLUDED */
