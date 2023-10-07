#ifndef LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "common/common.hh"

#include "inst.hh"
#include "intrusive_list/intrusive_list.hh"

namespace ljit
{

class BasicBlock final : public IntrusiveListNode<BasicBlock>
{
  IntrusiveList<Inst> m_instructions{};
  std::vector<BasicBlock *> m_pred{};
  std::size_t m_id{};

public:
  BasicBlock() = default;
  explicit BasicBlock(std::size_t idx) : m_id(idx)
  {}

  [[nodiscard]] auto size() const noexcept
  {
    return m_instructions.size();
  }

  [[nodiscard]] auto numPred() const noexcept
  {
    return m_pred.size();
  }

  void addPredecessor(BasicBlock *bb)
  {
    m_pred.push_back(bb);
  }

  [[nodiscard]] auto getFirst() const noexcept
  {
    return m_instructions.getFirst();
  }
  [[nodiscard]] auto getLast() const noexcept
  {
    return m_instructions.getLast();
  }

  template <class T, class... Args>
  auto emplaceInstBack(Args &&...args)
  {
    auto *const inserted = static_cast<T *>(
      m_instructions.emplaceBack(makeInst<T>(std::forward<Args>(args)...)));
    inserted->setBB(this);

    auto &&setThisAsPred = [this](BasicBlock *next) {
      next->addPredecessor(this);
    };

    if constexpr (std::is_same_v<T, IfInstr>)
    {
      setThisAsPred(inserted->getTrueBB());
      setThisAsPred(inserted->getFalseBB());
    }
    else if constexpr (std::is_same_v<T, JumpInstr>)
      setThisAsPred(inserted->getTarget());

    return inserted;
  }

  void print(std::ostream &ost) const
  {
    ost << '%' << m_id << ":\n";
    std::for_each(m_instructions.begin(), m_instructions.end(),
                  [&ost](const auto &inst) { inst.print(ost); });
  }
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED */
