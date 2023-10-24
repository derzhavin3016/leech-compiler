#ifndef LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_NODE_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_NODE_HH_INCLUDED

#include <vector>

namespace ljit::graph
{
template <class NodePtr>
using IdToNodeMap = std::vector<NodePtr>;
using IdToIdMap = std::vector<std::size_t>;

template <class NodeTy>
class DomTreeNode
{
  using NodePtrTy = NodeTy *;

  NodePtrTy m_idom{};
  std::vector<NodePtrTy> m_idommed{};

public:
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

#endif /* LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_NODE_HH_INCLUDED */
