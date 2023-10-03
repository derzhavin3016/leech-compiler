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

class BasicBlock final
{
  IntrusiveList<Inst> m_instructions{};
  std::vector<BasicBlock *> m_pred{};
  std::size_t m_id{};

public:
  explicit BasicBlock(std::size_t idx) : m_id(idx)
  {}
  LJIT_NO_COPY_SEMANTICS(BasicBlock);
  LJIT_NO_MOVE_SEMANTICS(BasicBlock);
  ~BasicBlock() = default;

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
  void emplaceInst(Args &&...args)
  {
    m_instructions.emplaceBack<T>(std::forward<Args>(args)...);
  }

  void print(std::ostream &ost) const
  {
    ost << '%' << m_id << ":\n";
    std::for_each(m_instructions.begin(), m_instructions.end(),
                  [&ost](auto pInst) { pInst->print(ost); });
  }
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED */
