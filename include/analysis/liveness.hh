#ifndef LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED
#define LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED

#include <algorithm>
#include <iterator>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "common/common.hh"
#include "graph/dfs.hh"
#include "ir/basic_block.hh"
#include "ir/inst.hh"
#include "loop_analyzer.hh"

namespace ljit
{

class LivenessAnalyzer final
{
public:
  using GraphTy = BasicBlockGraph;
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

private:
  using LinearOrderNodes = std::vector<NodePtrTy>;

public:
  LJIT_NO_COPY_SEMANTICS(LivenessAnalyzer);
  LJIT_NO_MOVE_SEMANTICS(LivenessAnalyzer);
  ~LivenessAnalyzer() = default;

  explicit LivenessAnalyzer(const GraphTy &graph)
    : m_loops(graph), m_linearOrder(buildLinearOrder(graph))
  {
    fillLiveNumbers();
    calcLiveRanges();
  }

  [[nodiscard]] const auto &getLinearOrder() const noexcept
  {
    return m_linearOrder;
  }

private:
  static constexpr std::size_t kLiveNumStep = 2;
  static constexpr std::size_t kLinNumStep = 1;

  using VisitedSet = std::unordered_set<NodePtrTy>;
  using LoopsTy = LoopAnalyzer<GraphTy>;
  using LoopInfo = typename LoopsTy::LoopInfo;

  using LiveSet = std::unordered_set<Value *>;

  void updateLiveInterval(Value *val, const LiveInterval &interval)
  {
    auto &&[it, wasNew] = m_liveIntervals.try_emplace(val, interval);
    if (wasNew)
      return;

    it->second.update(interval);
  }

  void fillLiveNumbers()
  {
    std::size_t curLin{};
    std::size_t curLive{};

    for (auto *bb : m_linearOrder)
    {
      const auto bbLiveNum = curLive;
      for (auto &inst : *bb)
      {
        const bool isPhi = inst.getInstType() == InstType::kPhi;
        inst.setLiveNum(isPhi ? bbLiveNum : curLive);

        if (!isPhi)
          curLive += kLiveNumStep;

        inst.setLinearNum(curLin);
        curLin += kLinNumStep;
      }

      bb->setLiveInterval({bbLiveNum, curLive});
    }
  }

  auto &calcInitLiveSet(NodePtrTy bb)
  {
    auto &&[it, wasNew] = m_liveSets.insert({bb, {}});
    LJIT_ASSERT(!wasNew);
    auto &liveSet = it->second;

    std::for_each(Traits::succBegin(bb), Traits::succEnd(bb),
                  [&](NodePtrTy succ) { processSucc(bb, succ, liveSet); });

    return liveSet;
  }

  void processSucc(NodePtrTy bb, NodePtrTy succ, LiveSet &set) const
  {
    auto foundIt = m_liveSets.find(succ);
    if (foundIt == m_liveSets.end())
      return;
    auto &&succLiveSet = foundIt->second;

    // Unite current set w/ successor's set
    set.insert(succLiveSet.begin(), succLiveSet.end());

    for (const auto &inst : *succ)
    {
      if (inst.getInstType() == InstType::kPhi)
      {
        const auto &phi = static_cast<const Phi &>(inst);
        for (const auto &entry : phi)
        {
          if (entry.bb == bb)
            set.insert(entry.m_val);
        }
      }
    }
  }

  void calcLiveRanges()
  {
    for (auto it = m_linearOrder.rbegin(); it != m_linearOrder.rend(); ++it)
    {
      auto &block = **it;

      auto &initLiveSet = calcInitLiveSet(&block);
      for (const auto &val : initLiveSet)
        updateLiveInterval(val, block.getLiveInterval());

      std::for_each(block.rbegin(), block.rend(), [&](Inst &inst) {
        const auto liveNum = inst.getLiveNum();
        auto &&[instIt, wasNew] =
          m_liveIntervals.try_emplace(&inst, liveNum, liveNum + kLiveNumStep);

        if (!wasNew)
          instIt->second.setStart(liveNum);
        initLiveSet.erase(&inst);

        processInputs(inst, initLiveSet, block.getLiveInterval().getStart());
      });

      for (auto &inst : block)
      {
        if (inst.getInstType() == InstType::kPhi)
          initLiveSet.erase(&inst);
      }

      if (const auto *const loopInfo = m_loops.getLoopInfo(&block);
          loopInfo->getHeader() == &block && loopInfo->reducible())
      {
        const auto start = block.getLiveInterval().getStart();
        const auto end = loopInfo->getLastBB()->getLiveInterval().getEnd();
        for (auto *v : initLiveSet)
          updateLiveInterval(v, {start, end});
      }
    }
  }

  void processInputs(const Inst &inst, LiveSet &liveSet, std::size_t bbLiveNum)
  {
    auto &&consumeInput = [&liveSet, this, bbLiveNum, &inst](Value *val) {
      LJIT_ASSERT(val != nullptr);
      liveSet.insert(val);
      updateLiveInterval(val, {bbLiveNum, inst.getLiveNum()});
    };

    switch (inst.getInstType())
    {
    case InstType::kBinOp: {
      const auto &binop = static_cast<const BinOp &>(inst);
      consumeInput(binop.getLeft());
      consumeInput(binop.getRight());
      break;
    }
    case InstType::kCast:
      consumeInput(static_cast<const Cast &>(inst).getSrc());
      break;
    case InstType::kUnknown:
      LJIT_UNREACHABLE("Unknown inst input");
    case InstType::kIf:
    case InstType::kConst:
    case InstType::kJump:
    case InstType::kRet:
    case InstType::kPhi:
    default:
      break;
    }
  }

  template <typename OutIt>
  void traverseLoop(const LoopInfo *lInfo, VisitedSet &visited, OutIt out) const
  {
    auto &&body = lInfo->getLinearOrder();

    for (auto *bb : body)
    {
      *out++ = bb;
      visited.insert(bb);
    }
  }

  [[nodiscard]] LinearOrderNodes buildLinearOrder(const GraphTy &graph) const
  {
    // Get post order
    const auto rpoOrder = graph::depthFirstSearchReversePostOrder(graph);

    using NodeIt = typename Traits::node_iterator;

    LinearOrderNodes res;
    VisitedSet visited;
    std::stack<std::pair<NodeIt, NodePtrTy>> toVisit;
    res.reserve(rpoOrder.size());

    for (auto *bb : rpoOrder)
    {
      if (visited.find(bb) != visited.end())
        continue;

      // Reducible loop
      if (const auto *loopInfo = m_loops.getLoopInfo(bb);
          loopInfo->getHeader() == bb && loopInfo->reducible())
      {
        traverseLoop(loopInfo, visited, std::back_inserter(res));
      }
      else
      {
        res.push_back(bb);
        visited.insert(bb);
      }
    }

    LJIT_ASSERT_MSG(res.size() == rpoOrder.size(),
                    "Missed some bb in Linear order");

    return res;
  }

  LoopsTy m_loops;
  LinearOrderNodes m_linearOrder;
  std::unordered_map<NodePtrTy, LiveSet> m_liveSets;
  std::unordered_map<Value *, LiveInterval> m_liveIntervals;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_ANALYSIS_LIVENESS_HH_INCLUDED */
