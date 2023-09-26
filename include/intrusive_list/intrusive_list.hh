#ifndef LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED
#define LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED

#include <algorithm>
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
  IntrusiveListNode *m_prev = nullptr;
  IntrusiveListNode *m_next = nullptr;

public:
  IntrusiveListNode() = default;

  LJIT_NO_COPY_SEMANTICS(IntrusiveListNode);
  LJIT_NO_MOVE_SEMANTICS(IntrusiveListNode);

  virtual ~IntrusiveListNode() = default;

  [[nodiscard]] auto getNext() const noexcept
  {
    return static_cast<Derived *>(m_next);
  }

  [[nodiscard]] auto getPrev() const noexcept
  {
    return static_cast<Derived *>(m_prev);
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
    m_next = next;
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
  std::vector<std::unique_ptr<BaseNode>> m_storage{};

public:
  [[nodiscard]] auto *getFirst() const noexcept
  {
    LJIT_ASSERT(!m_storage.empty());
    return m_storage.front().get();
  }

  [[nodiscard]] auto *getLast() const noexcept
  {
    LJIT_ASSERT(!m_storage.empty());
    return m_storage.back().get();
  }

  template <class T = BaseNode, class... Args>
  void emplaceBack(Args &&...args)
  {
    static_assert(std::is_base_of_v<BaseNode, T>);

    BaseNode *const newNode =
      m_storage.emplace_back(std::make_unique<T>(std::forward<Args>(args)...))
        .get();

    if (!m_storage.empty())
      newNode->insertAfter(m_storage.back().get());
  }

  [[nodiscard]] auto size() const noexcept
  {
    return m_storage.size();
  }

  template <class Walker>
  void walk(Walker &&walker)
  {
    for (const auto &elem : m_storage)
      std::forward<Walker>(walker)(elem.get());
  }
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED */
