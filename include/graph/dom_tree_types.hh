#ifndef LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_TYPES_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_TYPES_HH_INCLUDED

#include <ostream>
#include <vector>

namespace ljit::graph
{
template <class NodePtr>
using IdToNodeMap = std::vector<NodePtr>;
using IdToIdMap = std::vector<std::size_t>;

template <class NodePtrTy>
class DomTreeNode
{
  NodePtrTy m_idom{};
  std::vector<NodePtrTy> m_idommed{};

public:
  DomTreeNode() = default;
  explicit DomTreeNode(NodePtrTy idom) : m_idom(idom)
  {}
  DomTreeNode(NodePtrTy idom, const std::vector<NodePtrTy> &idommed)
    : DomTreeNode(idom), m_idommed(idommed)
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

  void dump(std::ostream &ost) const
  {
    ost << "Dominator: " << m_idom << std::endl;
    for (const auto &dom : m_idommed)
      ost << "Dominatee: " << dom << std::endl;
  }
};
} // namespace ljit::graph

#endif /* LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_TYPES_HH_INCLUDED */
