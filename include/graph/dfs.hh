#ifndef LEECH_JIT_INCLUDE_GRAPH_DFS_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DFS_HH_INCLUDED

#include <algorithm>
#include <stack>
#include <unordered_set>
#include <utility>
#include <vector>

#include "graph_traits.hh"

namespace ljit
{
template <class GraphTy>
struct DefaultDFSVisitor final
{
  using NodePtr = GraphTraits<GraphTy>::node_pointer;

  void operator()([[maybe_unused]] NodePtr pNode) const noexcept
  {}
};

template <class GraphTy, class DFSVisitor>
auto depthFirstSearch(const GraphTy &graph, DFSVisitor vis)
{
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = Traits::node_pointer;
  using NodeIt = Traits::node_iterator;

  std::vector<NodePtrTy> visTimes;
  std::unordered_set<NodePtrTy> visited;
  std::stack<std::pair<NodeIt, NodeIt>> toVisit;

  auto entry = Traits::entryPoint(graph);

  auto &&visitNode = [&](NodePtrTy pNode) {
    visited.insert(pNode);
    visTimes.push_back(pNode);
    vis(pNode);
  };

  visitNode(entry);

  toVisit.emplace(Traits::succBegin(entry), Traits::succEnd(entry));

  while (!toVisit.empty())
  {
    auto &&[first, last] = toVisit.top();
    toVisit.pop();

    auto unvisNode = std::find_if(first, last, [&visited](auto pNode) {
      return visited.find(pNode) == visited.end();
    });

    if (unvisNode == last)
      continue;

    toVisit.emplace(unvisNode, last);
    auto pNode = *unvisNode;

    visitNode(pNode);
  }

  return visTimes;
}

template <class GraphTy>
auto depthFirstSearch(const GraphTy &graph)
{
  return depthFirstSearch(graph, DefaultDFSVisitor<GraphTy>{});
}

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_GRAPH_DFS_HH_INCLUDED */
