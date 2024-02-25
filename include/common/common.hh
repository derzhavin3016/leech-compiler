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

#if __has_builtin(__builtin_unreachable) || defined(__GNUC__)
#define LJIT_BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#define LJIT_BUILTIN_UNREACHABLE __assume(false)
#endif

namespace ljit
{
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
#if defined(LJIT_BUILTIN_UNREACHABLE)
  LJIT_BUILTIN_UNREACHABLE;
#endif
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
