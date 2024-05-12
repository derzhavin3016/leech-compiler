#ifndef LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED

#include "basic_block.hh"
#include "common/common.hh"
#include "inst.hh"
#include "intrusive_list/intrusive_list.hh"

namespace ljit
{

class Function final
{
  IList<BasicBlock> m_bbs;
  Type m_resType{Type::None};
  std::vector<Type> m_args{};

public:
  Function() = default;
  explicit Function(Type resTy) : m_resType(resTy)
  {}

  explicit Function(Type resTy, std::vector<Type> args)
    : m_resType(resTy), m_args(std::move(args))
  {}

  [[nodiscard]] auto getResType() const noexcept
  {
    return m_resType;
  }

  [[nodiscard]] const auto &getArgs() const noexcept
  {
    return m_args;
  }

  auto *appendBB()
  {
    const auto idx = m_bbs.empty() ? 0ULL : m_bbs.back().getId() + 1;
    return &emplaceBackToList<BasicBlock>(m_bbs, idx);
  }

  [[nodiscard]] auto makeBBGraph() const noexcept
  {
    LJIT_ASSERT(!m_bbs.empty());
    return BasicBlockGraph{const_cast<BasicBlock *>(&m_bbs.front()),
                           m_bbs.size()};
  }
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_FUNCTION_HH_INCLUDED */
