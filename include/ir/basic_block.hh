#ifndef LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "common/common.hh"

#include "inst.hh"

namespace ljit
{

constexpr std::size_t kNumBBSuccessors = 2;

class BasicBlock final
{
  std::vector<std::unique_ptr<Inst>> m_instructions{};
  Inst *m_firstInst = nullptr;
  Inst *m_lastInst = nullptr;

  std::vector<BasicBlock *> m_pred{};
  std::array<BasicBlock *, kNumBBSuccessors> m_succ{};
  std::size_t m_numSucc{};

public:
  LJIT_NO_COPY_SEMANTICS(BasicBlock);
  LJIT_DEFAULT_MOVE_SEMANTICS(BasicBlock);
  ~BasicBlock() = default;

  [[nodiscard]] auto size() const noexcept
  {
    return m_instructions.size();
  }

  [[nodiscard]] auto numPred() const noexcept
  {
    return m_pred.size();
  }

  [[nodiscard]] auto numSucc() const noexcept
  {
    return m_numSucc;
  }

  void addPredecessor(BasicBlock *bb)
  {
    m_pred.push_back(bb);
  }

  void addSuccessor(BasicBlock *bb) noexcept
  {
    LJIT_ASSERT_MSG(m_numSucc < kNumBBSuccessors,
                    "Trying to add more than %lu successors of BasicBlock",
                    kNumBBSuccessors);
    m_succ[m_numSucc++] = bb;
  }

  [[nodiscard]] auto getFirst() const noexcept
  {
    return m_firstInst;
  }
  [[nodiscard]] auto getLast() const noexcept
  {
    return m_lastInst;
  }

  template <class T, class... Args>
  void appendInst(Args &&...args)
  {
    static_assert(std::is_base_of_v<Inst, T>);

    Inst *const newInst =
      m_instructions
        .emplace_back(std::make_unique<T>(std::forward<Args>(args)...))
        .get();

    if (m_firstInst == nullptr)
    {
      m_firstInst = m_lastInst = newInst;
      return;
    }

    newInst->insertAfter(m_lastInst);
    m_lastInst = newInst;
  }
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED */
