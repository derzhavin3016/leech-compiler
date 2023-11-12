#ifndef LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HELPERS_HH_INCLUDED
#define LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HELPERS_HH_INCLUDED

#include "common/common.hh"
#include "graph/graph_traits.hh"
#include <algorithm>
#include <cstddef>
#include <ostream>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ljit::graph::detail
{

enum class NodeIdTy : std::size_t
{
};
enum class DFSTimeTy : std::size_t
{
};

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
std::ostream &operator<<(std::ostream &ost, E val)
{
  return ost << toUnderlying(val);
}

template <typename E, typename T>
[[nodiscard]] constexpr auto toEnum(T val)
{
  static_assert(std::is_enum_v<E>);
  static_assert(std::is_same_v<std::underlying_type_t<E>, std::decay_t<T>>);

  return static_cast<E>(val);
}

template <typename T>
[[nodiscard]] constexpr auto toDFSTime(T val)
{
  return toEnum<DFSTimeTy>(val);
}

template <typename T>
[[nodiscard]] constexpr auto toNodeId(T val)
{
  return toEnum<NodeIdTy>(val);
}

template <class GraphTy>
[[nodiscard]] detail::NodeIdTy getNodeId(
  typename GraphTraits<GraphTy>::node_pointer node)
{
  return detail::toNodeId(GraphTraits<GraphTy>::id(node));
}

template <class MappedTy>
using FromIdMap = std::unordered_map<NodeIdTy, MappedTy>;
using IdToDSFMap = FromIdMap<DFSTimeTy>;

template <class MappedTy>
std::ostream &operator<<(std::ostream &ost, const FromIdMap<MappedTy> &map)
{
  for (auto &&[key, val] : map)
    ost << '[' << key << "] = " << val << '\n';
  return ost;
}

template <class MappedTy>
class [[nodiscard]] FromDFSTimeMap final
{
  std::vector<MappedTy> m_map;

public:
  FromDFSTimeMap() = default;

  explicit FromDFSTimeMap(std::size_t size) : m_map(size)
  {}

  void reserve(std::size_t size)
  {
    m_map.reserve(size);
  }

  template <typename T>
  void push_back(T &&val)
  {
    static_assert(std::is_same_v<std::remove_reference_t<T>, MappedTy>,
                  "Cannot push back value of other type than value_type");
    m_map.push_back(std::forward<T>(val));
  }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LJIT_GEN_TRANSP_METHOD(name)                                           \
  [[nodiscard]] auto name()                                                    \
  {                                                                            \
    return m_map.name();                                                       \
  }                                                                            \
  [[nodiscard]] auto name() const                                              \
  {                                                                            \
    return m_map.name();                                                       \
  }

  LJIT_GEN_TRANSP_METHOD(begin)
  LJIT_GEN_TRANSP_METHOD(end)
  LJIT_GEN_TRANSP_METHOD(rbegin)
  LJIT_GEN_TRANSP_METHOD(rend)

#undef LJIT_GEN_TRANSP_METHOD

  [[nodiscard]] auto &operator[](DFSTimeTy idx) const
  {
    const auto id = toUnderlying(idx);
    LJIT_ASSERT(id < m_map.size());
    return m_map[id];
  }

  auto &operator[](DFSTimeTy idx)
  {
    const auto id = toUnderlying(idx);
    LJIT_ASSERT(id < m_map.size());
    return m_map[id];
  }

  auto &front()
  {
    LJIT_ASSERT(!m_map.empty());
    return m_map.front();
  }

  [[nodiscard]] auto &front() const
  {
    LJIT_ASSERT(!m_map.empty());
    return m_map.front();
  }

  [[nodiscard]] auto size() const noexcept
  {
    return m_map.size();
  }

  void dump(std::ostream &ost) const
  {
    std::for_each(m_map.cbegin(), m_map.cend(),
                  [&, cnt = std::size_t{}](const auto &val) mutable {
                    ost << '[' << cnt++ << "] = " << val << '\n';
                  });
  }
};

using DFSTimeToTimeMap = FromDFSTimeMap<DFSTimeTy>;
} // namespace ljit::graph::detail

#endif /* LEECH_JIT_INCLUDE_GRAPH_DOM_TREE_HELPERS_HH_INCLUDED */
