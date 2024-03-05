#ifndef LEECH_JIT_INCLUDE_COMMON_COMMON_HH_INCLUDED
#define LEECH_JIT_INCLUDE_COMMON_COMMON_HH_INCLUDED

#include <cstdio>
#include <cstdlib> // IWYU pragma: keep (for std::abort)
#include <type_traits>

#define LJIT_COPY_SEMANTICS(CLS_NAME, EQ)                                      \
  CLS_NAME(const CLS_NAME &) = EQ;                                             \
  CLS_NAME &operator=(const CLS_NAME &) = EQ

#define LJIT_MOVE_SEMANTICS(CLS_NAME, EQ)                                      \
  CLS_NAME(CLS_NAME &&) noexcept = EQ;                                         \
  CLS_NAME &operator=(CLS_NAME &&) noexcept = EQ

#define LJIT_NO_COPY_SEMANTICS(CLS_NAME) LJIT_COPY_SEMANTICS(CLS_NAME, delete)

#define LJIT_DEFAULT_COPY_SEMANTICS(CLS_NAME)                                  \
  LJIT_COPY_SEMANTICS(CLS_NAME, default)

#define LJIT_NO_MOVE_SEMANTICS(CLS_NAME) LJIT_MOVE_SEMANTICS(CLS_NAME, delete)

#define LJIT_DEFAULT_MOVE_SEMANTICS(CLS_NAME)                                  \
  LJIT_MOVE_SEMANTICS(CLS_NAME, default)

#define LJIT_PRINT_ERR(...) static_cast<void>(std::fprintf(stderr, __VA_ARGS__))

#define LJIT_ABORT() std::abort()

#define LJIT_LIKELY(cond) (__builtin_expect((cond), 1))
#define LJIT_UNLIKELY(cond) (__builtin_expect((cond), 0))

#define LJIT_ASSERT_MSG(cond, ...)                                             \
  do                                                                           \
  {                                                                            \
    if LJIT_UNLIKELY (!(cond))                                                 \
    {                                                                          \
      LJIT_PRINT_ERR("*** LJIT ASSERTION FAILED ***\n");                       \
      LJIT_PRINT_ERR("Reason:\n");                                             \
      LJIT_PRINT_ERR(__VA_ARGS__);                                             \
      LJIT_PRINT_ERR("\n");                                                    \
      LJIT_PRINT_ERR("Condition '%s' has evaluated to false\n", #cond);        \
      LJIT_PRINT_ERR("In file %s on line %i in function %s\n", __FILE__,       \
                     __LINE__,                                                 \
                     static_cast<const char *>(__PRETTY_FUNCTION__));          \
      LJIT_PRINT_ERR("**************************\n");                          \
      LJIT_ABORT();                                                            \
    }                                                                          \
  } while (false)

#define LJIT_ASSERT(cond) LJIT_ASSERT_MSG(cond, "%s", "")

namespace ljit
{
namespace utils
{
template <typename... F>
struct Overloaded : public F...
{
  using F::operator()...;
};

#if __cplusplus < 202002L
template <class... F>
Overloaded(F...) -> Overloaded<F...>;
#endif
} // namespace utils

namespace detail
{
[[noreturn]] inline void unreachable_impl(const char *msg, const char *file,
                                          unsigned line)
{
  if (msg != nullptr)
    LJIT_PRINT_ERR("%s\n", msg);
  LJIT_PRINT_ERR("UNREACHABLE executed ");

  if (file != nullptr)
    LJIT_PRINT_ERR("at %s:%u", file, line);

  LJIT_PRINT_ERR("!\n");
  LJIT_ABORT();
}
} // namespace detail

template <class EnumT>
constexpr auto toUnderlying(EnumT val) noexcept
{
  static_assert(std::is_enum_v<EnumT>,
                "Trying to cast non-enum to it's underlying type");
  return static_cast<std::underlying_type_t<EnumT>>(val);
}
} // namespace ljit

#define LJIT_UNREACHABLE(msg)                                                  \
  ::ljit::detail::unreachable_impl(msg, __FILE__, __LINE__)

#endif /* LEECH_JIT_INCLUDE_COMMON_COMMON_HH_INCLUDED */
