#ifndef LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED
#define LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iterator>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "common/common.hh"
#include "graph/dfs.hh"
#include "inst.hh"
#include "intrusive_list/intrusive_list.hh"

#include "graph/graph_traits.hh"

namespace ljit
{
constexpr std::size_t kIvalidRegId = std::numeric_limits<std::size_t>::max();

class LiveInterval final
{
  std::size_t m_start{};
  std::size_t m_end{};
  std::size_t m_locationId{kIvalidRegId};
  bool m_onStack{false};

  constexpr void validate() const
  {
    if (m_start > m_end)
    {
      throw std::runtime_error{
        "Trying to create incorrect interval (start > end)"};
    }
  }

public:
  constexpr LiveInterval() = default;
  constexpr LiveInterval(std::size_t start, std::size_t end)
    : m_start(start), m_end(end)
  {
    validate();
  }

  constexpr void setStart(std::size_t start)
  {
    m_start = start;
    validate();
  }

  constexpr void setEnd(std::size_t end)
  {
    m_end = end;
    validate();
  }

  constexpr void moveToStack()
  {
    m_onStack = true;
  }

  [[nodiscard]] constexpr bool isOnStack() const
  {
    return m_onStack;
  }

  [[nodiscard]] constexpr auto getLocId() const
  {
    return m_locationId;
  }

  constexpr void setLocId(std::size_t locId)
  {
    m_locationId = locId;
  }

  [[nodiscard]] constexpr auto getStart() const
  {
    return m_start;
  }

  [[nodiscard]] constexpr auto getEnd() const
  {
    return m_end;
  }

  [[nodiscard]] constexpr bool empty() const
  {
    return m_start == m_end;
  }

  constexpr void update(const LiveInterval &other)
  {
    m_start = std::min(m_start, other.m_start);
    m_end = std::max(m_end, other.m_end);
  }

  [[nodiscard]] constexpr bool equal(const LiveInterval &rhs) const
  {
    return m_start == rhs.m_start && m_end == rhs.m_end;
  }

  void print(std::ostream &ost) const
  {
    ost << '[' << m_start << ", " << m_end << ']';
  }
};

inline std::ostream &operator<<(std::ostream &ost, const LiveInterval &interval)
{
  interval.print(ost);
  return ost;
}

[[nodiscard]] constexpr bool operator==(const LiveInterval &lhs,
                                        const LiveInterval &rhs)
{
  return lhs.equal(rhs);
}

[[nodiscard]] constexpr bool operator!=(const LiveInterval &lhs,
                                        const LiveInterval &rhs)
{
  return !(lhs == rhs);
}

class BasicBlock final : public IListNode
{
  using InstList = IList<Inst>;
  using InstIter = InstList::iterator;
  InstList m_instructions{};
  std::vector<BasicBlock *> m_pred{};
  std::vector<BasicBlock *> m_succ{};
  std::size_t m_id{};
  LiveInterval m_interval{};

public:
  using iterator = InstIter;

  BasicBlock() = default;
  explicit BasicBlock(std::size_t idx) : m_id(idx)
  {}

  [[nodiscard]] auto getId() const noexcept
  {
    return m_id;
  }

  [[nodiscard]] bool empty() const noexcept
  {
    return m_instructions.empty();
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

  [[nodiscard]] auto begin() const
  {
    return m_instructions.begin();
  }

  [[nodiscard]] auto begin()
  {
    return m_instructions.begin();
  }

  [[nodiscard]] auto end() const
  {
    return m_instructions.end();
  }

  [[nodiscard]] auto end()
  {
    return m_instructions.end();
  }

  [[nodiscard]] auto rbegin() const
  {
    return std::reverse_iterator{end()};
  }

  [[nodiscard]] auto rbegin()
  {
    return std::reverse_iterator{end()};
  }

  [[nodiscard]] auto rend() const
  {
    return std::reverse_iterator{begin()};
  }

  [[nodiscard]] auto rend()
  {
    return std::reverse_iterator{begin()};
  }

  [[nodiscard]] auto collectInsts(InstType type)
  {
    std::vector<std::reference_wrapper<Inst>> res;

    std::copy_if(begin(), end(), std::back_inserter(res),
                 [type](Inst &inst) { return inst.getInstType() == type; });

    return res;
  }

  [[nodiscard]] const auto &getFirst() const noexcept
  {
    return m_instructions.front();
  }
  [[nodiscard]] const auto &getLast() const noexcept
  {
    return m_instructions.back();
  }

  [[nodiscard]] auto &getFirst() noexcept
  {
    return m_instructions.front();
  }
  [[nodiscard]] auto &getLast() noexcept
  {
    return m_instructions.back();
  }

  void setLiveInterval(LiveInterval interval)
  {
    m_interval = interval;
  }

  [[nodiscard]] const auto &getLiveInterval() const
  {
    return m_interval;
  }
  template <class T, class... Args>
  auto pushInstFront(Args &&...args)
  {
    auto *const toIns = static_cast<T *>(&emplaceToList<T>(
      m_instructions, m_instructions.begin(), std::forward<Args>(args)...));
    toIns->setBB(this);

    return toIns;
  }

  template <class T, class... Args>
  auto pushInstBack(Args &&...args)
  {
    auto *const toIns = static_cast<T *>(
      &emplaceBackToList<T>(m_instructions, std::forward<Args>(args)...));

    toIns->setBB(this);

    updateLinks();

    return toIns;
  }

  void eraseInst(Inst *toErase)
  {
    m_instructions.erase(InstIter{toErase});
  }

  void replaceInst(Inst *old, Inst *newInst)
  {
    newInst->setUsersFrom(*old);
    m_instructions.insert(m_instructions.erase(InstIter{old}), newInst);
  }

  template <typename T, typename... Args>
  void replaceInstEmplace(Inst *old, Args &&...args)
  {
    const auto pos = m_instructions.erase(InstIter{old});
    emplaceToList<T>(m_instructions, pos, std::forward<Args>(args)...);
  }

  void print(std::ostream &ost) const
  {
    ost << '%' << m_id << ":\n";
    for (const auto &inst : m_instructions)
      inst.print(ost);
  }

  void linkSucc(BasicBlock *succ)
  {
    linkBBs(this, succ);
  }

  void linkPred(BasicBlock *pred)
  {
    linkBBs(pred, this);
  }

  void splice(InstIter pos, BasicBlock &other)
  {
    splice(pos, other.begin(), other.end());
  }

  void splice(InstIter pos, InstIter first, InstIter last)
  {
    std::for_each(first, last, [this](Inst &inst) { inst.setBB(this); });
    const bool updateRequired = pos == m_instructions.end();
    m_instructions.splice(pos, first, last);

    if (updateRequired)
      updateLinks();
  }

  void updateLinks()
  {
    // cleanup succs
    for (auto *succ : m_succ)
    {
      unlinkBBs(this, succ);
    }

    LJIT_ASSERT(m_succ.empty());

    if (m_instructions.empty())
    {
      return;
    }
    const auto &lastInsn = m_instructions.back();
    switch (lastInsn.getInstType())
    {
    case InstType::kIf: {
      linkSucc(static_cast<const IfInstr &>(lastInsn).getTrueBB());
      linkSucc(static_cast<const IfInstr &>(lastInsn).getFalseBB());
      break;
    }
    case InstType::kJump: {
      linkSucc(static_cast<const JumpInstr &>(lastInsn).getTarget());
      break;
    }
    case InstType::kUnknown:
    case InstType::kConst:
    case InstType::kBinOp:
    case InstType::kUnaryOp:
    case InstType::kRet:
    case InstType::kCast:
    case InstType::kPhi:
    case InstType::kCall:
    case InstType::kParam:
    default:
      break;
    }
  }

  static void unlinkBBs(BasicBlock *pred, BasicBlock *succ) noexcept
  {
    {
      auto &preds = succ->m_pred;
      const auto foundPred = std::find(preds.begin(), preds.end(), pred);
      LJIT_ASSERT(foundPred != preds.end());
      preds.erase(foundPred);
    }
    {
      auto &succs = pred->m_succ;
      const auto foundSucc = std::find(succs.begin(), succs.end(), succ);
      LJIT_ASSERT(foundSucc != succs.end());
      succs.erase(foundSucc);
    }
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

inline void removeInst(Inst *inst)
{
  inst->clearInputs();
  inst->getBB()->eraseInst(inst);
}

class BasicBlockGraph final
{
public:
  using value_type = BasicBlock;
  using pointer = value_type *;

  BasicBlockGraph(pointer root, std::size_t size) noexcept
    : m_root(root), m_size(size)
  {}

  explicit BasicBlockGraph(pointer root) noexcept : BasicBlockGraph{root, {}}
  {}

  [[nodiscard]] pointer getRoot() const noexcept
  {
    return m_root;
  }

  [[nodiscard]] auto size() const noexcept
  {
    return m_size;
  }

  void dumpDot(std::ostream &ost,
               const std::string &graphName = "BBGraph") const
  {
    ost << "digraph " << graphName << "{\n";
    graph::depthFirstSearchPreOrder(*this, [&](auto *pNode) {
      auto &&getName = [](auto *node) {
        std::ostringstream ss;
        ss << "bb" << node->getId();
        return ss.str();
      };
      auto &&name = getName(pNode);
      ost << name << " [label=" << '"' << pNode->getId() << "\"];\n";

      for (const auto &pred : pNode->getPred())
        ost << getName(pred) << " -> " << name << ";\n";
    });
    ost << "}";
  }

  void dumpDot(const std::string &filename,
               const std::string &name = "BBGraph") const
  {
    std::ofstream oft{filename};
    LJIT_ASSERT(oft.is_open());
    dumpDot(oft, name);
  }

private:
  pointer m_root{};
  std::size_t m_size{};
};

template <>
struct GraphTraits<BasicBlockGraph> final
{
  using node_pointer = BasicBlockGraph::pointer;
  using node_iterator = decltype(BasicBlock{}.getSucc().begin());

  static std::size_t id(node_pointer ptr)
  {
    LJIT_ASSERT(ptr != nullptr);
    return ptr->getId();
  }

  static node_pointer entryPoint(const BasicBlockGraph &graph)
  {
    return graph.getRoot();
  }
  static std::size_t size(const BasicBlockGraph &graph)
  {
    return graph.size();
  }

  static node_iterator succBegin(node_pointer node)
  {
    return node->getSucc().begin();
  }
  static node_iterator succEnd(node_pointer node)
  {
    return node->getSucc().end();
  }

  static node_iterator predBegin(node_pointer node)
  {
    return node->getPred().begin();
  }
  static node_iterator predEnd(node_pointer node)
  {
    return node->getPred().end();
  }
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_IR_BASIC_BLOCK_HH_INCLUDED */
