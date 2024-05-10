#ifndef LEECH_JIT_INCLUDE_COMMON_ERROR_HH_INCLUDED
#define LEECH_JIT_INCLUDE_COMMON_ERROR_HH_INCLUDED

#include <stdexcept>

namespace ljit
{
class ArithmeticError : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_COMMON_ERROR_HH_INCLUDED */
