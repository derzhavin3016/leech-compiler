#ifndef LEECH_JIT_INCLUDE_COMMON_COMMON_HH_INCLUDED
#define LEECH_JIT_INCLUDE_COMMON_COMMON_HH_INCLUDED

#include <cstdio>
#include <string_view>
#include <tuple>
#include <type_traits>

#define LJIT_COPY_SEMANTICS(CLS_NAME, EQ)                                      \
  CLS_NAME(const CLS_NAME &) = EQ;                                             \
  CLS_NAME &operator=(const CLS_NAME &) = EQ

#define LJIT_MOVE_SEMANTICS(CLS_NAME, EQ)                                      \
  CLS_NAME(CLS_NAME &&) = EQ;                                                  \
  CLS_NAME &operator=(CLS_NAME &&) = EQ

#define LJIT_NO_COPY_SEMANTICS(CLS_NAME) LJIT_COPY_SEMANTICS(CLS_NAME, delete)

#define LJIT_DEFAULT_COPY_SEMANTICS(CLS_NAME)                                  \
  LJIT_COPY_SEMANTICS(CLS_NAME, default)

#define LJIT_NO_MOVE_SEMANTICS(CLS_NAME) LJIT_MOVE_SEMANTICS(CLS_NAME, delete)

#define LJIT_DEFAULT_MOVE_SEMANTICS(CLS_NAME)                                  \
  LJIT_MOVE_SEMANTICS(CLS_NAME, default)

#define LJIT_PRINT_ERR(...) std::ignore = std::fprintf(stderr, __VA_ARGS__)

#define LJIT_ABORT() std::abort()

#define LJIT_LIKELY(cond) (__builtin_expect((cond), 1))
#define LJIT_UNLIKELY(cond) (__builtin_expect((cond), 0))

#define LJIT_ASSERT_MSG(cond, ...)                                             \
  [&, func_name = static_cast<const char *>(__PRETTY_FUNCTION__)] {            \
    if LJIT_UNLIKELY (cond)                                                    \
    {                                                                          \
      LJIT_PRINT_ERR("*** LJIT ASSERTION FAILED ***\n");                       \
      LJIT_PRINT_ERR("Reason:\n");                                             \
      LJIT_PRINT_ERR(__VA_ARGS__);                                             \
      LJIT_PRINT_ERR("\n");                                                    \
      LJIT_PRINT_ERR("Condition '%s' has evaluated to false\n", #cond);        \
      LJIT_PRINT_ERR("In file %s on line %u in function %s\n", __FILE__,       \
                     __LINE__, func_name);                                     \
      LJIT_PRINT_ERR("**************************\n");                          \
      LJIT_ABORT();                                                            \
    }                                                                          \
  }()

#define LJIT_ASSERT(cond) LJIT_ASSERT_MSG(cond, "")

namespace ljit
{
template <class EnumT>
constexpr auto toUnderlying(EnumT val) noexcept
{
  static_assert(std::is_enum_v<EnumT>,
                "Trying to cast non-enum to it's underlying type");
  return static_cast<std::underlying_type_t<EnumT>>(val);
}
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_COMMON_COMMON_HH_INCLUDED */
