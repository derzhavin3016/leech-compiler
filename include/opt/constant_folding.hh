#ifndef LEECH_JIT_INCLUDE_OPT_CONSTANT_FOLDING_HH_INCLUDED
#define LEECH_JIT_INCLUDE_OPT_CONSTANT_FOLDING_HH_INCLUDED

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>

#include "common/common.hh"
#include "common/error.hh"
#include "graph/dfs.hh"
#include "ir/basic_block.hh"
#include "ir/inst.hh"

namespace ljit
{
class ConstantFolding
{
  using GraphTy = BasicBlockGraph;
  using Traits = GraphTraits<GraphTy>;
  using NodePtrTy = typename Traits::node_pointer;

  // Add + Shr + Or
public:
  void run(const GraphTy &graph)
  {
    findFoldable(graph);

    for (auto inst : m_toFold)
    {
      auto &rInst = inst.get();
      auto newInst = fold(rInst);
      auto *const bb = rInst.getBB();
      // Remove inputs
      rInst.clearInputs();

      bb->replaceInst(&rInst, newInst.release());
    }
  }

private:
  void findFoldable(const GraphTy &graph)
  {
    auto &&bbs = graph::depthFirstSearchReversePostOrder(graph);

    std::vector<std::reference_wrapper<Inst>> toFold{};
    for (auto *const bb : bbs)
    {
      std::copy_if(bb->begin(), bb->end(), std::back_inserter(toFold),
                   foldable);
    }

    m_toFold.swap(toFold);
  }

  [[nodiscard]] static bool foldable(Inst &inst)
  {
    switch (inst.getInstType())
    {
    case InstType::kBinOp: {
      const auto &binOp = static_cast<BinOp &>(inst);
      return (tryRetrieveConst(binOp.getLeft()) != nullptr) &&
             (tryRetrieveConst(binOp.getRight()) != nullptr);
    }

    case InstType::kCast: {
      const auto &cast = static_cast<Cast &>(inst);
      return tryRetrieveConst(cast.getSrc()) != nullptr;
    }
    case InstType::kConst:
    case InstType::kIf:
    case InstType::kUnaryOp:
    case InstType::kJump:
    case InstType::kParam:
    case InstType::kCall:
    case InstType::kPhi:
    case InstType::kRet:
    case InstType::kUnknown:
    default:
      return false;
    }
  }

  std::unique_ptr<Inst> fold(Inst &inst) const
  {
    // Check if instruction supports folding
    switch (inst.getInstType())
    {
#define DO_FOLD(Ty)                                                            \
  case InstType::k##Ty:                                                        \
    return do##Ty##Fold(static_cast<Ty &>(inst));

      DO_FOLD(BinOp)
      DO_FOLD(Cast)

#undef DO_FOLD
    case InstType::kConst:
    case InstType::kUnaryOp:
    case InstType::kIf:
    case InstType::kJump:
    case InstType::kPhi:
    case InstType::kParam:
    case InstType::kCall:
    case InstType::kRet:
    case InstType::kUnknown:
    default:
      break;
    }
    LJIT_UNREACHABLE("Cannot fold");
  }

  template <typename T>
  static std::unique_ptr<Inst> binEval(BinOp::Oper oper, const Inst *lv,
                                       const Inst *rv)
  {
    const auto *const lhs = static_cast<const ConstVal<T> *>(lv);
    const auto *const rhs = static_cast<const ConstVal<T> *>(rv);

    const auto lval = lhs->getVal();
    const auto rval = rhs->getVal();
    T res{};

    switch (oper)
    {
    case BinOp::Oper::kAdd:
      res = static_cast<T>(lval + rval);
      break;
    case BinOp::Oper::kSub:
      res = static_cast<T>(lval - rval);
      break;
    case BinOp::Oper::kMul:
      if constexpr (std::is_same_v<T, bool>)
      {
        res = static_cast<T>(lval && rval);
      }
      else
      {
        res = static_cast<T>(lval * rval);
      }
      break;
    case BinOp::Oper::kLE:
      res = static_cast<T>(lval < rval);
      break;
    case BinOp::Oper::kEQ:
      res = static_cast<T>(lval == rval);
      break;
    case BinOp::Oper::kShr: {
      constexpr auto kWidth = std::numeric_limits<T>::digits;
      if (rval >= kWidth)
      {
        std::ostringstream ss;
        ss << "Shift amount (which is " << rval
           << ") exceeds the width of type (" << kWidth << ")";
        throw ArithmeticError{ss.str()};
      }
      if constexpr (!std::is_same_v<T, bool>)
      {
        if (rval < 0)
        {
          throw ArithmeticError{"Shamt is negative"};
        }
      }
      res = static_cast<T>(lval >> rval);
      break;
    }
    case BinOp::Oper::kOr: {
      res = static_cast<T>(lval | rval);
      break;
    }
    case BinOp::Oper::kBoundsCheck:
    case BinOp::Oper::kDiv:
    default:
      LJIT_UNREACHABLE("Bad Oper");
    }

    return std::make_unique<ConstVal<T>>(res);
  }

  std::unique_ptr<Inst> doBinOpFold(BinOp &inst) const
  {
    const auto *const lval = static_cast<const Inst *>(inst.getLeft());
    const auto *const rval = static_cast<const Inst *>(inst.getRight());

    LJIT_ASSERT(lval->getInstType() == InstType::kConst &&
                rval->getInstType() == InstType::kConst);

    const auto valTy = lval->getType();

    LJIT_ASSERT(lval->getType() == rval->getType());

    switch (valTy)
    {
    case Type::I1:
      return binEval<bool>(inst.getOper(), lval, rval);

#define DO_FOLD(width)                                                         \
  case Type::I##width:                                                         \
    return binEval<std::int##width##_t>(inst.getOper(), lval, rval);

      DO_FOLD(8)
      DO_FOLD(16)
      DO_FOLD(32)
      DO_FOLD(64)

#undef DO_FOLD

    case Type::None:
    default:
      break;
    }
    LJIT_UNREACHABLE("Unknown const type");
  }

  template <typename T>
  std::unique_ptr<Inst> castEval(const Inst *val) const
  {
    const auto *const cval = static_cast<const ConstVal<T> *>(val);
    const auto res = static_cast<T>(cval->getVal());
    return std::make_unique<ConstVal<T>>(res);
  }
  std::unique_ptr<Inst> doCastFold(Cast &inst) const
  {
    const auto *srcV = static_cast<const Inst *>(inst.getSrc());
    switch (inst.getType())
    {
    case Type::I1:
      return castEval<bool>(srcV);

#define DO_FOLD(width)                                                         \
  case Type::I##width:                                                         \
    return castEval<std::int##width##_t>(srcV);

      DO_FOLD(8)
      DO_FOLD(16)
      DO_FOLD(32)
      DO_FOLD(64)

#undef DO_FOLD
    case Type::None:
    default:
      LJIT_UNREACHABLE("Bad type");
    }
  }

  std::vector<std::reference_wrapper<Inst>> m_toFold{};
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_OPT_CONSTANT_FOLDING_HH_INCLUDED */
