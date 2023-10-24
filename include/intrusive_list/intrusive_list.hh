#ifndef LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED
#define LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "common/common.hh"

namespace ljit
{
struct IListError : public std::runtime_error
{
  explicit IListError(std::string_view msg) noexcept
    : std::runtime_error(msg.data())
  {}
};

class IListNode
{
  IListNode *m_prev = nullptr;
  IListNode *m_next = nullptr;

public:
  IListNode() = default;
  LJIT_NO_COPY_SEMANTICS(IListNode);
  LJIT_NO_MOVE_SEMANTICS(IListNode);
  ~IListNode() = default;

  void setPrev(IListNode *prev) noexcept
  {
    m_prev = prev;
  }
  void setNext(IListNode *next) noexcept
  {
    m_next = next;
  }

  [[nodiscard]] auto *getPrev() const noexcept
  {
    return m_prev;
  }

  [[nodiscard]] auto *getNext() const noexcept
  {
    return m_next;
  }
};

struct IListSentinel : public IListNode
{
  IListSentinel()
  {
    setPrev(this);
    setNext(this);
  }

  [[nodiscard]] bool empty() const noexcept
  {
    return this == getPrev();
  }
};

class IListBase
{
protected:
  static void insertBefore(IListNode *point, IListNode *toInsert) noexcept
  {
    LJIT_ASSERT(point != nullptr);
    LJIT_ASSERT(toInsert != nullptr);

    auto *const prev = point->getPrev();
    if (prev != nullptr)
    {
      prev->setNext(toInsert);
    }
    toInsert->setPrev(prev);
    toInsert->setNext(point);

    point->setPrev(toInsert);
  }

  static void insertAfter(IListNode *point, IListNode *toInsert) noexcept
  {
    LJIT_ASSERT(point != nullptr);
    LJIT_ASSERT(toInsert != nullptr);

    auto *const next = point->getNext();
    if (next != nullptr)
    {
      next->setPrev(toInsert);
    }

    toInsert->setPrev(point);
    toInsert->setNext(next);

    point->setNext(toInsert);
  }

  static void remove(IListNode *toRemove)
  {
    LJIT_ASSERT(toRemove != nullptr);

    auto *const next = toRemove->getNext();
    auto *const prev = toRemove->getPrev();

    next->setPrev(prev);
    prev->setNext(next);

    // Just to be sure
    toRemove->setNext(nullptr);
    toRemove->setPrev(nullptr);
  }

  static void moveBefore(IListNode *point, IListNode *first,
                         IListNode *last) noexcept
  {
    LJIT_ASSERT(point != nullptr);
    LJIT_ASSERT(first != nullptr);
    LJIT_ASSERT(last != nullptr);

    LJIT_ASSERT_MSG(point != first,
                    "Trying to insert last before first (list cycling)");
    if (last == point || first == last)
    {
      // No need to do anything in this case
      return;
    }

    // Link last with the pre-first part
    auto *const lastToIns = last->getPrev();
    auto *const preFirst = first->getPrev();
    last->setPrev(preFirst);
    preFirst->setNext(last);

    auto *const prev = point->getPrev();
    point->setPrev(lastToIns);
    prev->setNext(first);

    lastToIns->setNext(point);
    first->setPrev(prev);
  }
};

namespace detail
{

template <bool Cond, class T>
using AddConstIf = std::conditional_t<Cond, std::add_const_t<T>, T>;

template <class NodeTy, bool IsConst>
class IListIterator final
{
  static_assert(std::is_base_of_v<IListNode, NodeTy>);

public:
  using value_type = AddConstIf<IsConst, NodeTy>;
  using pointer = value_type *;
  using reference = value_type &;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;

  using node_pointer = AddConstIf<IsConst, IListNode> *;

private:
  node_pointer m_ptr = nullptr;

public:
  IListIterator() = default;

  explicit IListIterator(node_pointer node) : m_ptr(node)
  {}

  explicit IListIterator(pointer ptr) : m_ptr(ptr)
  {}
  explicit IListIterator(reference ref) : m_ptr(&ref)
  {}

  [[nodiscard]] reference operator*() const noexcept
  {
    return *static_cast<pointer>(m_ptr);
  }

  [[nodiscard]] pointer operator->() const noexcept
  {
    return &operator*();
  }

  [[nodiscard]] bool equal(const IListIterator &other) const noexcept
  {
    return m_ptr == other.m_ptr;
  }

  IListIterator &operator--() noexcept
  {
    m_ptr = m_ptr->getPrev();
    return *this;
  }

  IListIterator &operator++() noexcept
  {
    m_ptr = m_ptr->getNext();
    return *this;
  }

  IListIterator operator--(int) noexcept
  {
    auto tmp = *this;
    --*this;
    return tmp;
  }

  IListIterator operator++(int) noexcept
  {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  [[nodiscard]] node_pointer getNodePtr() const noexcept
  {
    return m_ptr;
  }
};

template <class NodeTy, bool IsConst>
[[nodiscard]] bool operator==(
  const IListIterator<NodeTy, IsConst> &lhs,
  const IListIterator<NodeTy, IsConst> &rhs) noexcept
{
  return lhs.equal(rhs);
}

template <class NodeTy, bool IsConst>
[[nodiscard]] bool operator!=(
  const IListIterator<NodeTy, IsConst> &lhs,
  const IListIterator<NodeTy, IsConst> &rhs) noexcept
{
  return !(lhs == rhs);
}
} // namespace detail

template <class NodeTy>
class IListAllocTraits
{
protected:
  static void deallocate(NodeTy *node)
  {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete node;
  }
};

template <class NodeTy>
struct IListNoAllocTraits
{
protected:
  static void deallocate([[maybe_unused]] NodeTy *node)
  {}
};

template <class NodeTy, class AllocTraits>
class IListImpl final : public IListBase, public AllocTraits
{
public:
  using iterator = detail::IListIterator<NodeTy, false>;
  using const_iterator = detail::IListIterator<NodeTy, true>;
  using value_type = typename iterator::value_type;
  using pointer = typename iterator::pointer;
  using reference = typename iterator::reference;
  using const_pointer = typename const_iterator::pointer;
  using const_reference = typename const_iterator::reference;

  IListImpl() = default;
  LJIT_NO_COPY_SEMANTICS(IListImpl);

  IListImpl(IListImpl &&that) noexcept
  {
    splice(end(), that);
  }

  IListImpl &operator=(IListImpl &&that) noexcept
  {
    clear();
    splice(end(), that);
    return *this;
  }

  ~IListImpl()
  {
    clear();
  }

  [[nodiscard]] iterator begin() noexcept
  {
    return std::next(end());
  }

  [[nodiscard]] const_iterator begin() const noexcept
  {
    return std::next(end());
  }

  [[nodiscard]] iterator end() noexcept
  {
    return iterator{&m_sentinel};
  }

  [[nodiscard]] const_iterator end() const noexcept
  {
    return const_iterator{&m_sentinel};
  }

  [[nodiscard]] bool empty() const noexcept
  {
    return m_sentinel.empty();
  }

  [[nodiscard]] auto size() const noexcept
  {
    const auto sz = std::distance(begin(), end());
    LJIT_ASSERT(sz > 0);
    return static_cast<std::size_t>(sz);
  }

  [[nodiscard]] const_reference front() const noexcept
  {
    return *begin();
  }

  [[nodiscard]] reference front() noexcept
  {
    return *begin();
  }

  [[nodiscard]] const_reference back() const noexcept
  {
    return *std::prev(end());
  }

  [[nodiscard]] reference back() noexcept
  {
    return *std::prev(end());
  }

  iterator insert(iterator pos, pointer node)
  {
    IListBase::insertBefore(pos.getNodePtr(), node);
    return iterator{node};
  }

  void push_back(pointer node) noexcept
  {
    insert(end(), node);
  }

  pointer remove(iterator &pos) noexcept
  {
    pointer pNode = &*pos++;
    IListBase::remove(pNode);
    return pNode;
  }

  iterator erase(iterator pos) noexcept
  {
    LJIT_ASSERT_MSG(pos != end(), "Trying to erase end()");
    AllocTraits::deallocate(remove(pos));
    return pos;
  }

  iterator erase(iterator first, iterator last) noexcept
  {
    while (first != last)
    {
      first = erase(first);
    }

    return last;
  }

  void splice(iterator pos, IListImpl &other) noexcept
  {
    splice(pos, other.begin(), other.end());
  }
  void splice(iterator pos, iterator it) noexcept
  {
    splice(pos, it, std::next(it));
  }
  void splice(iterator pos, iterator first, iterator last) noexcept
  {
    IListBase::moveBefore(pos.getNodePtr(), first.getNodePtr(),
                          last.getNodePtr());
  }

  void clear()
  {
    erase(begin(), end());
  }

private:
  IListSentinel m_sentinel{};
};

template <class T>
using IList = IListImpl<T, IListAllocTraits<T>>;

template <class T, class... Args, class NodeTy>
[[nodiscard]] auto &emplaceBackToList(IList<NodeTy> &ilist, Args &&...args)
{
  auto *const toEmplace = new T{std::forward<Args>(args)...};
  ilist.push_back(toEmplace);
  return *toEmplace;
}

template <class T>
using IListView = IListImpl<T, IListNoAllocTraits<T>>;

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED */
