#ifndef LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED
#define LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>

#include "common/common.hh"

namespace ljit
{
struct IListError : public std::runtime_error
{
  explicit IListError(std::string_view msg) noexcept
    : std::runtime_error(msg.data())
  {}
};

template <class Derived>
class IntrusiveListNode
{
private:
  std::unique_ptr<IntrusiveListNode> m_next = nullptr;
  IntrusiveListNode *m_prev = nullptr;

public:
  IntrusiveListNode() = default;

  LJIT_NO_COPY_SEMANTICS(IntrusiveListNode);
  LJIT_NO_MOVE_SEMANTICS(IntrusiveListNode);

  virtual ~IntrusiveListNode() = default;

  [[nodiscard]] auto getNext() const noexcept
  {
    return static_cast<Derived *>(m_next.get());
  }

  [[nodiscard]] auto getPrev() const noexcept
  {
    return static_cast<Derived *>(m_prev);
  }

  void resetNext() noexcept
  {
    m_next.reset();
  }

  void insertBefore(IntrusiveListNode &pos) noexcept
  {
    setNext(&pos);

    auto *const newPrev = pos.getPrev();
    setPrev(newPrev);

    if (newPrev != nullptr)
      newPrev->setNext(this);

    pos.setPrev(this);
  }

  void insertBefore(IntrusiveListNode *pos)
  {
    if (pos == nullptr)
      throw IListError{"Trying to insert before NULL node"};
    insertBefore(*pos);
  }

  void insertAfter(IntrusiveListNode &pos) noexcept
  {
    setPrev(&pos);

    auto *const newNext = pos.getNext();
    setNext(newNext);

    if (newNext != nullptr)
      newNext->setPrev(this);

    pos.setNext(this);
  }

  void insertAfter(IntrusiveListNode *pos)
  {
    if (pos == nullptr)
      throw IListError{"Trying to insert after NULL node"};
    insertAfter(*pos);
  }

private:
  void setNext(IntrusiveListNode *next) noexcept
  {
    m_next.reset(next);
  }

  void setPrev(IntrusiveListNode *prev) noexcept
  {
    m_prev = prev;
  }
};

template <class BaseNode>
class IntrusiveList final
{
  static_assert(std::is_base_of_v<IntrusiveListNode<BaseNode>, BaseNode>);

  std::unique_ptr<BaseNode> m_head;
  BaseNode *m_tail{};
  std::size_t m_size{};

public:
  class iterator final
  {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = BaseNode;
    using pointer = value_type *;
    using reference = value_type &;

  private:
    pointer m_ptr{nullptr};

  public:
    iterator() = default;
    explicit iterator(pointer ptr) : m_ptr{ptr}
    {}

    [[nodiscard]] reference operator*() const noexcept
    {
      return *m_ptr;
    }

    [[nodiscard]] pointer operator->() const noexcept
    {
      return m_ptr;
    }

    reference operator++() noexcept
    {
      m_ptr = m_ptr->getNext();
      return *m_ptr;
    }

    reference operator--() noexcept
    {
      m_ptr = m_ptr->getPrev();
      return *m_ptr;
    }

    value_type operator++(int) noexcept
    {
      auto tmp = *this;
      operator++();
      return tmp;
    }
    value_type operator--(int) noexcept
    {
      auto tmp = *this;
      operator--();
      return tmp;
    }

    [[nodiscard]] bool operator==(const iterator &rhs) const noexcept
    {
      return m_ptr == rhs.m_ptr;
    }
    [[nodiscard]] bool operator!=(const iterator &rhs) const noexcept
    {
      return !(*this == rhs);
    }
  };

  IntrusiveList() = default;

  LJIT_NO_COPY_SEMANTICS(IntrusiveList);
  LJIT_DEFAULT_MOVE_SEMANTICS(IntrusiveList);

  ~IntrusiveList()
  {
    for (; m_tail != nullptr; m_tail = m_tail->getPrev())
      m_tail->resetNext();
  }

  [[nodiscard]] auto *getFirst() const noexcept
  {
    LJIT_ASSERT(!empty());
    return m_head.get();
  }

  [[nodiscard]] auto *getLast() const noexcept
  {
    LJIT_ASSERT(!empty());
    return m_tail;
  }

  template <class T = BaseNode, class... Args>
  auto emplaceBack(Args &&...args)
  {
    static_assert(std::is_base_of_v<BaseNode, T>);

    auto newTail = std::make_unique<T>(std::forward<Args>(args)...);

    if (empty())
    {
      m_tail = newTail.get();
      m_head.swap(newTail);
    }
    else
    {
      newTail.release()->insertAfter(*m_tail);
      m_tail = m_tail->getNext();
    }

    ++m_size;
    return m_tail;
  }

  [[nodiscard]] bool empty() const noexcept
  {
    return size() == 0;
  }

  [[nodiscard]] auto size() const noexcept
  {
    return m_size;
  }

  [[nodiscard]] auto begin() const noexcept
  {
    return iterator{m_head.get()};
  }

  [[nodiscard]] auto end() const noexcept
  {
    return iterator{};
  }
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED */
