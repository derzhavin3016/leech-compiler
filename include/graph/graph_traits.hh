#ifndef LEECH_JIT_INCLUDE_GRAPH_GRAPH_TRAITS_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_GRAPH_TRAITS_HH_INCLUDED

namespace ljit
{
template <class GraphTy>
struct GraphTraits final
{
  using node_pointer = typename GraphTy::pointer;
  using node_iterator = typename GraphTy::node_iterator;

  static node_pointer entryPoint(const GraphTy &graph);

  static node_iterator succBegin(node_pointer ptr);
  static node_iterator succEnd(node_pointer ptr);

  static node_iterator predBegin(node_pointer ptr);
  static node_iterator predEnd(node_pointer ptr);
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_GRAPH_GRAPH_TRAITS_HH_INCLUDED */
