#ifndef LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED
#define LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED

#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace ljit
{
struct IListError : public std::runtime_error
{
  explicit IListError(std::string_view msg) noexcept
    : std::runtime_error(msg.data())
  {}
};

class IntrusiveListNode
{
private:
  IntrusiveListNode *m_prev = nullptr;
  IntrusiveListNode *m_next = nullptr;

  template <class T>
  using DeriverdPtrType = std::remove_reference_t<T> *;

  template <class T>
  using DerivedPtr = std::enable_if_t<
    std::is_base_of_v<IntrusiveListNode, std::remove_reference_t<T>>,
    DeriverdPtrType<T>>;

public:
  IntrusiveListNode() = default;

  IntrusiveListNode(const IntrusiveListNode &) = delete;
  IntrusiveListNode(IntrusiveListNode &&) = default;
  IntrusiveListNode &operator=(const IntrusiveListNode &) = delete;
  IntrusiveListNode &operator=(IntrusiveListNode &&) = default;
  virtual ~IntrusiveListNode() = default;

  template <class T = IntrusiveListNode>
  [[nodiscard]] DerivedPtr<T> getNext() const noexcept
  {
    return static_cast<DeriverdPtrType<T>>(m_next);
  }

  template <class T = IntrusiveListNode>
  [[nodiscard]] DerivedPtr<T> getPrev() const noexcept
  {
    return static_cast<DeriverdPtrType<T>>(m_prev);
  }

  void insertBefore(IntrusiveListNode &pos) noexcept;
  void insertBefore(IntrusiveListNode *pos)
  {
    if (pos == nullptr)
      throw IListError{"Trying to insert before NULL node"};
    insertBefore(*pos);
  }

  void insertAfter(IntrusiveListNode &pos) noexcept;
  void insertAfter(IntrusiveListNode *pos)
  {
    if (pos == nullptr)
      throw IListError{"Trying to insert after NULL node"};
    insertAfter(*pos);
  }

private:
  void setNext(IntrusiveListNode *next) noexcept
  {
    m_next = next;
  }

  void setPrev(IntrusiveListNode *prev) noexcept
  {
    m_prev = prev;
  }
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED */
