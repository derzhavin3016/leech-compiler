#ifndef LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED
#define LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED

namespace ljit
{
class IntrusiveList
{
private:
  IntrusiveList *m_prev = nullptr;
  IntrusiveList *m_next = nullptr;

public:
  IntrusiveList() = default;

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  explicit IntrusiveList(IntrusiveList *prev, IntrusiveList *next) noexcept
    : m_prev(prev), m_next(next)
  {}

  IntrusiveList(const IntrusiveList &) = delete;
  IntrusiveList(IntrusiveList &&) = delete;
  IntrusiveList &operator=(const IntrusiveList &) = delete;
  IntrusiveList &operator=(IntrusiveList &&) = delete;
  virtual ~IntrusiveList() = default;

private:
  [[nodiscard]] auto getNext() const noexcept
  {
    return m_next;
  }

  [[nodiscard]] auto getPrev() const noexcept
  {
    return m_next;
  }

  void insertBefore(IntrusiveList *pos) noexcept;
  void insertAfter(IntrusiveList *pos) noexcept;
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_INTRUSIVE_LIST_INTRUSIVE_LIST_HH_INCLUDED */
