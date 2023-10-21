#ifndef LEECH_JIT_INCLUDE_GRAPH_DFS_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DFS_HH_INCLUDED

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stack>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "graph_traits.hh"


namespace ljit::graph
{
template <class GraphTy>
struct DefaultDFSVisitor final
{
  using NodePtr = typename GraphTraits<GraphTy>::node_pointer;

  void operator()([[maybe_unused]] NodePtr pNode) const noexcept
  {}
};

template <class GraphTy, class DFSVisitor>
void depthFirstSearch(const GraphTy &graph, DFSVisitor vis)
{
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;
  using NodeIt = typename Traits::node_iterator;

  std::unordered_set<NodePtrTy> visited;
  std::stack<std::pair<NodeIt, NodeIt>> toVisit;

  const auto entry = Traits::entryPoint(graph);
  if (entry == nullptr)
    return;

  auto &&visitNode = [&](NodePtrTy pNode) {
    visited.insert(pNode);
    vis(pNode);
  };

  visitNode(entry);

  toVisit.emplace(Traits::succBegin(entry), Traits::succEnd(entry));

  while (!toVisit.empty())
  {
    auto [first, last] = toVisit.top();
    toVisit.pop();

    auto unvisNode = std::find_if(first, last, [&visited](auto pNode) {
      return visited.find(pNode) == visited.end();
    });

    if (unvisNode == last)
      continue;

    toVisit.emplace(unvisNode, last);
    const auto pNode = *unvisNode;

    toVisit.emplace(Traits::succBegin(pNode), Traits::succEnd(pNode));

    visitNode(pNode);
  }
}
template <class GraphTy, class DFSVisitor, class RPONodeIter>
void depthFirstSearch(const GraphTy &graph, DFSVisitor vis, RPONodeIter it)
{
  depthFirstSearch(graph, [&](auto pNode) {
    *it++ = pNode;
    vis(pNode);
  });
}

template <class GraphTy>
[[nodiscard]] auto depthFirstSearch(const GraphTy &graph)
{
  std::vector<typename GraphTraits<GraphTy>::node_pointer> bbs;
  depthFirstSearch(graph, DefaultDFSVisitor<GraphTy>{},
                   std::back_inserter(bbs));
  return bbs;
}
} // namespace ljit::graph


#endif /* LEECH_JIT_INCLUDE_GRAPH_DFS_HH_INCLUDED */
