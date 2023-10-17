#ifndef LEECH_JIT_INCLUDE_GRAPH_GRAPH_TRAITS_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_GRAPH_TRAITS_HH_INCLUDED

namespace ljit
{
template <class GraphTy>
struct GraphTraits final
{
  using node_pointer = GraphTy::pointer;
  using node_iterator = GraphTy::node_iterator;

  static node_pointer entryPoint(const GraphTy &graph)
  {
    return graph.getEntry();
  }

  static node_iterator succBegin(node_pointer ptr)
  {
    return ptr->succBegin();
  }
  static node_iterator succEnd(node_pointer ptr)
  {
    return ptr->succEnd();
  }
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_GRAPH_GRAPH_TRAITS_HH_INCLUDED */
