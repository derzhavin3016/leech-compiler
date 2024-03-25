#ifndef LEECH_JIT_INCLUDE_ANALYSIS_REGALLOC_HH_INCLUDED
#define LEECH_JIT_INCLUDE_ANALYSIS_REGALLOC_HH_INCLUDED

#include "common/common.hh"
#include "ir/basic_block.hh"
#include "liveness.hh"
#include <algorithm>
#include <bitset>
#include <cstddef>
#include <optional>
#include <set>
#include <vector>

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
  explicit RegAllocator(const GraphTy &graph)
    : m_liveIntervals(buildSortedLiveIntervals(graph))
  {
    linearScan();
  }

  ~RegAllocator() = default;
  LJIT_NO_COPY_SEMANTICS(RegAllocator);
  LJIT_NO_MOVE_SEMANTICS(RegAllocator);

private:
  static std::vector<LiveInterval> buildSortedLiveIntervals(
    const GraphTy &graph)
  {
    std::vector<LiveInterval> liveIns;
    {
      auto &&liveIntervals = LivenessAnalyzer{graph}.getLiveIntervals();
      liveIns.reserve(liveIntervals.size());

      for (auto &&[_, liveInt] : liveIntervals)
      {
        if (!liveInt.empty())
          liveIns.push_back(liveInt);
      }
    }

    std::sort(liveIns.begin(), liveIns.end(),
              [](const auto &lhs, const auto &rhs) {
                return lhs.getStart() < rhs.getStart();
              });

    return liveIns;
  }

  void linearScan()
  {
    for (auto &liveIn : m_liveIntervals)
    {
      expireOldIntervals(liveIn);
      if (m_active.size() == kNumRegs)
      {
        spillAtInterval(liveIn);
      }
      else
      {
        const auto newRegId = [this] {
          const auto allocated = m_regPool.allocateReg();
          LJIT_ASSERT(allocated.has_value());
          return allocated.value();
        }();

        liveIn.setLocId(newRegId);
        m_active.insert(&liveIn);
      }
    }
  }

  void expireOldIntervals(const LiveInterval &liveIn)
  {
    for (auto it = m_active.begin(); it != m_active.end();)
    {
      const auto &activeIn = **it;
      if (activeIn.getEnd() > liveIn.getStart())
        return;

      LJIT_ASSERT(!activeIn.isOnStack());
      m_regPool.deallocateReg(activeIn.getLocId());
      it = m_active.erase(it);
    }
  }

  void moveToStack(LiveInterval &liveIn)
  {
    liveIn.setLocId(m_stackPos++);
    liveIn.moveToStack();
  }

  void spillAtInterval(LiveInterval &liveIn)
  {
    const auto spillIt = std::prev(m_active.end());
    if (auto &spill = **spillIt; spill.getEnd() > liveIn.getEnd())
    {
      LJIT_ASSERT(!spill.isOnStack());
      liveIn.setLocId(spill.getLocId());

      moveToStack(spill);

      m_active.erase(spillIt);
      m_active.insert(&liveIn);
      return;
    }

    moveToStack(liveIn);
  }

  struct CompareLessEndP
  {
    bool operator()(const LiveInterval *lhs, const LiveInterval *rhs) const
    {
      return lhs->getEnd() < rhs->getEnd();
    }
  };

  RegisterPool<kNumRegs> m_regPool;
  std::vector<LiveInterval> m_liveIntervals;
  std::set<LiveInterval *, CompareLessEndP> m_active;
  std::size_t m_stackPos{};
};

} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_ANALYSIS_REGALLOC_HH_INCLUDED */
