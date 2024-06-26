#include "ir/inst.hh"
#include "ir/function.hh"
#include <algorithm>

namespace ljit
{
Call::Call(Function *callee)
  : Inst(callee->getResType(), InstType::kCall), m_callee(callee)
{}

[[nodiscard]] bool Call::verify() const
{
  if (m_callee->getResType() != getType())
    return false;

  const auto &calleeArgs = m_callee->getArgs();
  return std::equal(
    inputBegin(), inputEnd(), calleeArgs.begin(), calleeArgs.end(),
    [](auto *const val, auto tpy) { return val->getType() == tpy; });
}
} // namespace ljit
