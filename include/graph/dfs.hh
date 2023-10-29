#ifndef LEECH_JIT_INCLUDE_GRAPH_DFS_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DFS_HH_INCLUDED

#include <algorithm>
#include <iterator>
#include <stack>
#include <unordered_set>
#include <utility>
#include <vector>

#include "common/common.hh"
#include "graph_traits.hh"

namespace ljit::graph
{
template <class GraphTy>
struct DFSVisitor
{
  DFSVisitor() = default;
  using NodePtr = typename GraphTraits<GraphTy>::node_pointer;

  void discoverNode([[maybe_unused]] NodePtr ptr)
  {}
  void finishNode([[maybe_unused]] NodePtr ptr)
  {}

  LJIT_DEFAULT_MOVE_SEMANTICS(DFSVisitor);
  LJIT_DEFAULT_COPY_SEMANTICS(DFSVisitor);

protected:
  ~DFSVisitor() = default;
};

template <class GraphTy, class DFSVisitor>
void depthFirstSearch(const GraphTy &graph, DFSVisitor vis)
{
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;
  using NodeIt = typename Traits::node_iterator;

  std::unordered_set<NodePtrTy> visited;
  std::stack<std::pair<NodeIt, NodePtrTy>> toVisit;

  const auto entry = Traits::entryPoint(graph);
  if (entry == nullptr)
    return;

  auto &&visitNode = [&](NodePtrTy pNode) {
    visited.insert(pNode);
    vis.discoverNode(pNode);
    toVisit.emplace(Traits::succBegin(pNode), pNode);
  };

  visitNode(entry);

  while (!toVisit.empty())
  {
    auto [first, parent] = toVisit.top();
    toVisit.pop();
    const auto succEnd = Traits::succEnd(parent);

    auto unvisNode = std::find_if(first, succEnd, [&visited](auto pNode) {
      return visited.find(pNode) == visited.end();
    });

    if (unvisNode == succEnd)
    {
      vis.finishNode(parent);
      continue;
    }

    toVisit.emplace(std::next(unvisNode), parent);

    const auto pNode = *unvisNode;
    visitNode(pNode);
  }
}

template <class GraphTy, class PreOrderVis>
void depthFirstSearchPreOrder(const GraphTy &graph, PreOrderVis vis)
{
  struct Vis : public DFSVisitor<GraphTy>
  {
    explicit Vis(PreOrderVis &vis) : m_vis{vis}
    {}

    void discoverNode(typename GraphTraits<GraphTy>::node_pointer nodePtr)
    {
      m_vis(nodePtr);
    }

  private:
    PreOrderVis &m_vis;
  };

  depthFirstSearch(graph, Vis{vis});
}

template <class GraphTy, class PostOrderVis>
void depthFirstSearchPostOrder(const GraphTy &graph, PostOrderVis vis)
{
  struct Vis : public DFSVisitor<GraphTy>
  {
    explicit Vis(PostOrderVis &vis) : m_vis{vis}
    {}

    void finishNode(typename GraphTraits<GraphTy>::node_pointer nodePtr)
    {
      m_vis(nodePtr);
    }

  private:
    PostOrderVis &m_vis;
  };

  depthFirstSearch(graph, Vis{vis});
}

template <class GraphTy>
[[nodiscard]] auto depthFirstSearchPreOrder(const GraphTy &graph)
{
  std::vector<typename GraphTraits<GraphTy>::node_pointer> bbs;
  depthFirstSearchPreOrder(graph, [&](auto node) { bbs.push_back(node); });
  return bbs;
}

template <class GraphTy>
[[nodiscard]] auto depthFirstSearchPostOrder(const GraphTy &graph)
{
  std::vector<typename GraphTraits<GraphTy>::node_pointer> bbs;
  depthFirstSearchPostOrder(graph, [&](auto node) { bbs.push_back(node); });
  return bbs;
}

template <class GraphTy>
[[nodiscard]] auto depthFirstSearchReversePostOrder(const GraphTy &graph)
{
  auto bbs = depthFirstSearchPostOrder(graph);
  std::reverse(bbs.begin(), bbs.end());
  return bbs;
}
} // namespace ljit::graph

#endif /* LEECH_JIT_INCLUDE_GRAPH_DFS_HH_INCLUDED */
