#ifndef LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include "common/common.hh"
#include "inst.hh"
#include "intrusive_list/intrusive_list.hh"

#include "graph/graph_traits.hh"

namespace ljit
{

class BasicBlock final : public IListNode
{
  IList<Inst> m_instructions{};
  std::vector<BasicBlock *> m_pred{};
  std::vector<BasicBlock *> m_succ{};
  std::size_t m_id{};

public:
  BasicBlock() = default;
  explicit BasicBlock(std::size_t idx) : m_id(idx)
  {}

  [[nodiscard]] auto getId() const noexcept
  {
    return m_id;
  }

  [[nodiscard]] auto size() const noexcept
  {
    return m_instructions.size();
  }

  [[nodiscard]] auto numPred() const noexcept
  {
    return m_pred.size();
  }

  [[nodiscard]] auto &getPred() const noexcept
  {
    return m_pred;
  }

  [[nodiscard]] auto &getSucc() const noexcept
  {
    return m_succ;
  }

  [[nodiscard]] auto numSucc() const noexcept
  {
    return m_succ.size();
  }

  [[nodiscard]] auto &getFirst() const noexcept
  {
    return m_instructions.front();
  }
  [[nodiscard]] auto &getLast() const noexcept
  {
    return m_instructions.back();
  }

  template <class T, class... Args>
  auto pushInstBack(Args &&...args)
  {
    auto *const toIns = static_cast<T *>(
      &emplaceBackToList<T>(m_instructions, std::forward<Args>(args)...));

    toIns->setBB(this);

    if constexpr (std::is_same_v<T, IfInstr>)
    {
      linkSucc(toIns->getTrueBB());
      linkSucc(toIns->getFalseBB());
    }
    else if constexpr (std::is_same_v<T, JumpInstr>)
    {
      linkSucc(toIns->getTarget());
    }

    return toIns;
  }

  void print(std::ostream &ost) const
  {
    ost << '%' << m_id << ":\n";
    std::for_each(m_instructions.begin(), m_instructions.end(),
                  [&ost](const auto &inst) { inst.print(ost); });
  }

  void linkSucc(BasicBlock *succ)
  {
    linkBBs(this, succ);
  }

  void linkPred(BasicBlock *pred)
  {
    linkBBs(pred, this);
  }

  static void linkBBs(BasicBlock *pred, BasicBlock *succ) noexcept
  {
    LJIT_ASSERT(pred != nullptr);
    LJIT_ASSERT(succ != nullptr);
    pred->addSuccessor(succ);
    succ->addPredecessor(pred);
  }

private:
  void addPredecessor(BasicBlock *bb)
  {
    m_pred.push_back(bb);
  }

  void addSuccessor(BasicBlock *bb)
  {
    m_succ.push_back(bb);
  }
};

class BasicBlockGraph final
{
public:
  using value_type = const BasicBlock;
  using pointer = value_type *;

  explicit BasicBlockGraph(pointer root) noexcept : m_root(root)
  {}

  [[nodiscard]] pointer getRoot() const noexcept
  {
    return m_root;
  }

private:
  pointer m_root{};
};

template <>
struct GraphTraits<BasicBlockGraph> final
{
  using node_pointer = BasicBlockGraph::pointer;
  using node_iterator = decltype(BasicBlock{}.getSucc().begin());

  static node_pointer entryPoint(const BasicBlockGraph &graph)
  {
    return graph.getRoot();
  }

  static node_iterator succBegin(node_pointer node)
  {
    return node->getSucc().begin();
  }
  static node_iterator succEnd(node_pointer node)
  {
    return node->getSucc().end();
  }
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED */
