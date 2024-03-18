#ifndef LEECH_JIT_INCLUDE_ANALYSIS_REGALLOC_HH_INCLUDED
#define LEECH_JIT_INCLUDE_ANALYSIS_REGALLOC_HH_INCLUDED

#include "common/common.hh"
#include "liveness.hh"
#include <bitset>
#include <cstddef>
#include <optional>

namespace ljit
{
template <std::size_t NumRegs>
class RegisterPool final
{
  std::size_t m_useCount{};
  std::bitset<NumRegs> m_regs{};

public:
  [[nodiscard]] constexpr auto getUseCount() const
  {
    return m_useCount;
  }

  [[nodiscard]] constexpr std::optional<std::size_t> allocateReg()
  {
    if (m_useCount == m_regs.size())
      return std::nullopt;

    for (std::size_t i = 0; i < m_regs.size(); ++i)
    {
      if (!m_regs.test(i))
      {
        m_regs.set(i);
        ++m_useCount;
        return i;
      }
    }

    LJIT_UNREACHABLE("No registers found, but useCount is OK");
  }

  constexpr void deallocateReg(std::size_t idx)
  {
    LJIT_ASSERT_MSG(idx < m_regs.size(),
                    "Trying to deallocate register id which is out of bound");
    m_regs.reset(idx);
    --m_useCount;
  }
};

class RegAllocator final
{
  constexpr static std::size_t kNumRegs = 3;

  using GraphTy = BasicBlockGraph;
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

public:
  explicit RegAllocator(const GraphTy &graph) : m_liveAnalyzer(graph)
  {}
  ~RegAllocator() = default;
  LJIT_NO_COPY_SEMANTICS(RegAllocator);
  LJIT_NO_MOVE_SEMANTICS(RegAllocator);

private:
  RegisterPool<kNumRegs> m_regPool;
  LivenessAnalyzer m_liveAnalyzer;
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_ANALYSIS_REGALLOC_HH_INCLUDED */
