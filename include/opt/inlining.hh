#ifndef LEECH_JIT_INCLUDE_OPT_INLINING_HH_INCLUDED
#define LEECH_JIT_INCLUDE_OPT_INLINING_HH_INCLUDED

#include "common/common.hh"
#include "graph/dfs.hh"
#include "ir/basic_block.hh"
#include "ir/function.hh"
#include "ir/inst.hh"
#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

namespace ljit
{
class Inlining final
{
  Function *m_func{};

public:
  explicit Inlining(Function *func) : m_func(func)
  {}

  void run()
  {
    fillCandidates();
    for (auto &inst : m_candidates)
      doInline(static_cast<Call &>(inst.get()));
  }

private:
  void fillCandidates()
  {
    auto &&bbs = graph::depthFirstSearchReversePostOrder(m_func->makeBBGraph());
    m_candidates.clear();
    for (auto *const bb : bbs)
    {
      std::copy_if(
        bb->begin(), bb->end(), std::back_inserter(m_candidates),
        [](Inst &inst) { return inst.getInstType() == InstType::kCall; });
    }
  }

  auto *splitBBAfter(Inst *inst)
  {
    auto *const newBB = m_func->appendBB();
    const auto pivot = std::next(BasicBlock::iterator{inst});

    newBB->splice(newBB->end(), pivot, inst->getBB()->end());

    return newBB;
  }

  static void adjustInputs(Call &inst, Function *callee)
  {
    BasicBlock *const firstBB = callee->makeBBGraph().getRoot();

    auto &&params = firstBB->collectInsts(InstType::kParam);
    auto paramIt = params.begin();

    for (auto inpIt = inst.inputBegin();
         inpIt != inst.inputEnd() && paramIt != params.end();
         ++inpIt, ++paramIt)
    {
      auto *input = *inpIt;
      Inst &param = *paramIt;
      // Instructions in callee function should use origins of parameters
      input->setUsersFrom(param);
      firstBB->eraseInst(&param);
    }
  }

  static void adjustOutputs(Call &inst, Function *callee,
                            BasicBlock *afterCallBB)
  {
    if (callee->getResType() == Type::None)
    {
      return;
    }

    std::vector<Ret *> rets;
    graph::depthFirstSearchPreOrder(
      callee->makeBBGraph(), [&rets](BasicBlock *bb) {
        LJIT_ASSERT(!bb->empty());
        auto *lastInsn = &bb->getLast();
        if (lastInsn->getInstType() == InstType::kRet)
        {
          rets.push_back(static_cast<Ret *>(lastInsn));
        }
      });

    if (rets.size() == 1)
    {
      auto *retVal = rets.front()->getVal();

      retVal->setUsersFrom(inst);
    }
    else
    {
      auto *const phi = afterCallBB->pushInstFront<Phi>(callee->getResType());

      for (auto *ret : rets)
        phi->addNode(ret->getVal(), ret->getBB());

      phi->setUsersFrom(inst);
    }

    // Replace return w/ jmp

    for (auto *ret : rets)
    {
      auto *bb = ret->getBB();
      bb->eraseInst(ret);
      bb->pushInstBack<JumpInstr>(afterCallBB);
    }
  }

  void doInline(Call &inst)
  {
    LJIT_ASSERT(inst.verify());

    auto *bb = inst.getBB();
    auto *callee = inst.getCallee();

    auto *afterCallBB = splitBBAfter(&inst);

    adjustInputs(inst, callee);
    adjustOutputs(inst, callee, afterCallBB);
    {
      auto *const calleeFstBB = callee->makeBBGraph().getRoot();
      bb->splice(bb->end(), *calleeFstBB);
      callee->eraseBB(calleeFstBB);
    }

    // Inline function into caller graph
    m_func->splice(Function::BBIterator{afterCallBB}, *callee);

    // Remove call instruction from caller BB
    bb->eraseInst(&inst);
  }

  std::vector<std::reference_wrapper<Inst>> m_candidates;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_OPT_INLINING_HH_INCLUDED */
