#ifndef LEECH_JIT_INCLUDE_OPT_PEEPHOLE_HH_INCLUDED
#define LEECH_JIT_INCLUDE_OPT_PEEPHOLE_HH_INCLUDED

#include "common/common.hh"
#include "ir/basic_block.hh"
#include "ir/inst.hh"
#include <cstdio>
#include <memory>

namespace ljit
{
class PeepHole final
{
  using GraphTy = BasicBlockGraph;
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

  // Add + Shr + Xor
public:
  void run(const GraphTy &graph)
  {
    findCandidates(graph);

    for (auto inst : m_candidates)
    {
      auto &rInst = inst.get();
      fold(rInst);
    }
  }

private:
  void findCandidates(const GraphTy &graph)
  {
    auto &&bbs = graph::depthFirstSearchReversePostOrder(graph);

    std::vector<std::reference_wrapper<Inst>> toFold{};
    for (auto *const bb : bbs)
    {
      std::copy_if(bb->begin(), bb->end(), std::back_inserter(toFold),
                   foldable);
    }

    m_candidates.swap(toFold);
  }

  [[nodiscard]] static bool foldable(Inst &inst)
  {
    switch (inst.getInstType())
    {
    case InstType::kBinOp:
      return true;

    case InstType::kCast:
    case InstType::kConst:
    case InstType::kIf:
    case InstType::kJump:
    case InstType::kPhi:
    case InstType::kRet:
    case InstType::kUnknown:
    default:
      return false;
    }
  }

  static bool doBinFold(BinOp &binop)
  {
    if (binop.users().empty())
    {
      return false; // No users => no need to fold
    }

    if (binop.getOper() == BinOp::Oper::kAdd ||
        binop.getOper() == BinOp::Oper::kOr)
    {
      auto *lhs = binop.inputAt(0);
      auto *rhs = binop.inputAt(1);
      if (lhs->isInst() &&
          static_cast<Inst *>(lhs)->getInstType() == InstType::kConst &&
          (!rhs->isInst() ||
           static_cast<Inst *>(rhs)->getInstType() != InstType::kConst))
      {
        binop.swapInputs(0, 1);
      }
    }

    switch (binop.getOper())
    {
    case BinOp::Oper::kAdd:
      return doAdd(binop);
    case BinOp::Oper::kShr:
      return doShr(binop);
    case BinOp::Oper::kOr:
      return doOr(binop);
    case BinOp::Oper::kSub:
    case BinOp::Oper::kMul:
    case BinOp::Oper::kLE:
    case BinOp::Oper::kEQ:
      break;
    }

    return false;
  }

  static bool doAdd(BinOp &add)
  {
    auto *const lval = add.inputAt(0);
    auto *const rval = add.inputAt(1);

    // Rule 0:
    // add v0, 0

    if (rval->isInst())
    {
      if (auto *const_ = static_cast<Inst *>(rval);
          const_->getInstType() == InstType::kConst && checkForVal<0>(const_))
      {
        lval->setUsersFrom(add);
        removeInst(&add);
        return true;
      }
    }

    return false;
  }

  static bool doShr(BinOp &binop)
  {
    auto *const lval = binop.inputAt(0);
    auto *const rval = binop.inputAt(1);

    // Rule 0:
    // shr v0, 0
    if (!rval->isInst())
    {
      return false;
    }
    auto *const const_ = static_cast<Inst *>(rval);

    if (const_->getInstType() != InstType::kConst)
    {
      return false;
    }

    if (checkForVal<0>(const_))
    {
      lval->setUsersFrom(binop);
      removeInst(&binop);
      return true;
    }

    // Rule 1:
    // v1 = shr v0, const_1
    // v2 = shr v1, const_2
    // --> v = add const_1, const_2
    // --> v2 = shr v0, v

    if (lval->isInst() &&
        static_cast<Inst *>(lval)->getInstType() == InstType::kBinOp &&
        static_cast<BinOp *>(lval)->getOper() == BinOp::Oper::kShr)
    {
      auto *fstShr = static_cast<BinOp *>(lval);
      auto *fstShamt = fstShr->inputAt(1);
      if (fstShr->users().size() > 1)
      {
        return false;
      }

      if (!fstShamt->isInst() ||
          static_cast<Inst *>(fstShamt)->getInstType() != InstType::kConst)
        return false;

      std::unique_ptr<Inst> newAdd{
        new BinOp{BinOp::Oper::kAdd, fstShamt, rval}};

      binop.setInput(0, fstShr->getLeft());
      binop.setInput(1, newAdd.get());

      fstShr->clearInputs();
      fstShr->getBB()->replaceInst(fstShr, newAdd.release());
    }

    return false;
  }

  static bool doOr(BinOp &binop)
  {
    auto *const lval = binop.inputAt(0);
    auto *const rval = binop.inputAt(1);

    // Rule 0:
    // or v0, 0
    if (!rval->isInst())
    {
      return false;
    }
    auto *const const_ = static_cast<Inst *>(rval);

    if (const_->getInstType() != InstType::kConst)
    {
      return false;
    }

    if (checkForVal<0>(const_))
    {
      lval->setUsersFrom(binop);
      removeInst(&binop);
      return true;
    }

    // Rule 1:
    // or v0, -1 (0xFFFF) -> -1

    if (checkForVal<-1>(const_))
    {
      rval->setUsersFrom(binop);
      removeInst(&binop);
      return true;
    }

    return false;
  }

  static bool fold(Inst &inst)
  {
    switch (inst.getInstType())
    {
    case InstType::kBinOp: {
      return doBinFold(static_cast<BinOp &>(inst));
    }
    case InstType::kUnknown:
    case InstType::kIf:
    case InstType::kConst:
    case InstType::kJump:
    case InstType::kRet:
    case InstType::kCast:
    case InstType::kPhi:
    default:
      break;
    }
    LJIT_UNREACHABLE("Bad inst type");
  }

  template <typename T, T val>
  [[nodiscard]] static bool checkVal(const Inst *inst)
  {
    return static_cast<const ConstVal<T> *>(inst)->getVal() == val;
  }

  template <int val>
  [[nodiscard]] static bool checkForVal(const Inst *inst)
  {
    switch (inst->getType())
    {
    case Type::I1:
      return checkVal<bool, static_cast<bool>(val)>(inst);

#define DO_CASE(w)                                                             \
  case Type::I##w:                                                             \
    return checkVal<std::int##w##_t, std::int##w##_t{val}>(inst);

      DO_CASE(8)
      DO_CASE(16)
      DO_CASE(32)
      DO_CASE(64)

#undef DO_CASE

    case Type::None:
    default:
      LJIT_UNREACHABLE("Bad const");
    }
  }

  std::vector<std::reference_wrapper<Inst>> m_candidates{};
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_OPT_PEEPHOLE_HH_INCLUDED */
