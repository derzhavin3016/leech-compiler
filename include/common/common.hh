#ifndef LEECH_JIT_INCLUDE_COMMON_COMMON_HH_INCLUDED
#define LEECH_JIT_INCLUDE_COMMON_COMMON_HH_INCLUDED

#include <cstdio>
#include <string_view>
#include <tuple>

#define LJIT_NO_COPY_SEMANTICS(CLS_NAME)                                       \
  CLS_NAME(const CLS_NAME &) = delete;                                         \
  CLS_NAME &operator=(const CLS_NAME &) = delete;

#define LJIT_NO_MOVE_SEMANTICS(CLS_NAME)                                       \
  CLS_NAME(CLS_NAME &&) = delete;                                              \
  CLS_NAME &operator=(CLS_NAME &&) = delete;

#define LJIT_PRINT_ERR(msg, ...)                                               \
  std::ignore = std::fprintf(stderr, msg, __VA_ARGS__)

#define LJIT_ABORT() std::abort()

#define LJIT_LIKELY(cond) (__builtin_expect((cond), 1))
#define LJIT_UNLIKELY(cond) (__builtin_expect((cond), 0))

#define LJIT_ASSERT_MSG(cond, msg, ...)                                        \
  [func_name = static_cast<const char *>(__PRETTY_FUNCTION__), &] {            \
    if LJIT_LIKELY (cond)                                                      \
      return;                                                                  \
    LJIT_PRINT_ERR("*** LJIT ASSERTION FAILED ***\n");                         \
    LJIT_PRINT_ERR("Condition '%s' has evaluated to false\n", #cond);          \
    LJIT_PRINT_ERR("In file %s on line %u in function %s\n", __FILE__,         \
                   __LINE__, func_name);                                       \
    LJIT_PRINT_ERR("**************************\n");                            \
    LJIT_ABORT();                                                              \
  }()

namespace ljit
{
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_COMMON_COMMON_HH_INCLUDED */
