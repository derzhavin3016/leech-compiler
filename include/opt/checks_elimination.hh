#ifndef LEECH_JIT_INCLUDE_OPT_CHECKS_ELIMINATION_HH_INCLUDED
#define LEECH_JIT_INCLUDE_OPT_CHECKS_ELIMINATION_HH_INCLUDED

#include "common/common.hh"
#include "graph/dom_tree.hh"
#include "ir/basic_block.hh"
#include "ir/inst.hh"

namespace ljit
{
class ChecksElimination
{
  using GraphTy = BasicBlockGraph;
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

public:
  void run(GraphTy &graph)
  {
    fillCandidates(graph);
    m_domTree = graph::buildDomTree(graph);

    for (auto &cand : m_candidates)
    {
      Inst &inst = cand.get();
      switch (inst.getInstType())
      {
      case InstType::kUnknown:
      case InstType::kIf:
      case InstType::kConst:
      case InstType::kJump:
      case InstType::kBinOp: {
        auto &binop = static_cast<BinOp &>(inst);
        if (binop.getOper() == BinOp::Oper::kBoundsCheck)
        {
          boundsCheckElim(binop);
        }
        break;
      }
      case InstType::kUnaryOp: {
        auto &op = static_cast<UnaryOp &>(inst);
        if (op.getOper() == UnaryOp::Oper::kZeroCheck)
        {
          zeroCheckElim(op);
        }
        break;
      }
      case InstType::kRet:
      case InstType::kCast:
      case InstType::kPhi:
      case InstType::kCall:
      case InstType::kParam:
        break;
      }
    }
  }

private:
  void zeroCheckElim(UnaryOp &op)
  {
    auto *input = op.getVal();
    for (auto it = input->usersBegin(); it != input->usersEnd(); ++it)
    {
      auto *user = *it;
      if (&op == user)
      {
        continue;
      }
      if (user->getInstType() != InstType::kUnaryOp)
      {
        continue;
      }
      auto *unop = static_cast<UnaryOp *>(user);
      if (unop->getOper() != UnaryOp::Oper::kZeroCheck)
      {
        continue;
      }

      if (m_domTree.isDominator(user, &op))
      {
        input->users().erase(&op);
        op.getBB()->eraseInst(&op);
      }
    }
  }

  void boundsCheckElim(BinOp &op)
  {
    auto *input = op.inputAt(0);
    auto *bound = op.inputAt(1);

    for (auto it = input->usersBegin(); it != input->usersEnd(); ++it)
    {
      auto *user = *it;
      if (&op == user)
      {
        continue;
      }
      if (user->getInstType() != InstType::kBinOp)
      {
        continue;
      }
      auto *oper = static_cast<BinOp *>(user);
      if (oper->getOper() != BinOp::Oper::kBoundsCheck ||
          bound != oper->inputAt(1))
      {
        continue;
      }

      if (m_domTree.isDominator(user, &op))
      {
        input->users().erase(&op);
        op.getBB()->eraseInst(&op);
      }
    }
  }

  void fillCandidates(GraphTy &graph)
  {
    auto &&bbs = graph::depthFirstSearchReversePostOrder(graph);
    m_candidates.clear();
    for (auto *const bb : bbs)
    {
      std::copy_if(bb->begin(), bb->end(), std::back_inserter(m_candidates),
                   [](Inst &inst) {
                     switch (inst.getInstType())
                     {
                     case InstType::kUnaryOp:
                       return static_cast<UnaryOp &>(inst).getOper() ==
                              UnaryOp::Oper::kZeroCheck;
                     case InstType::kBinOp:
                       return static_cast<BinOp &>(inst).getOper() ==
                              BinOp::Oper::kBoundsCheck;
                     case InstType::kUnknown:
                     case InstType::kIf:
                     case InstType::kConst:
                     case InstType::kJump:
                     case InstType::kRet:
                     case InstType::kCast:
                     case InstType::kPhi:
                     case InstType::kCall:
                     case InstType::kParam:
                       break;
                     }
                     return false;
                   });
    }
  }

  graph::DominatorTree<GraphTy> m_domTree;
  std::vector<std::reference_wrapper<Inst>> m_candidates;
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_OPT_CHECKS_ELIMINATION_HH_INCLUDED */
