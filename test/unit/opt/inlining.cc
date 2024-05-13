#include <gtest/gtest.h>
#include <memory>

#include "../graph/graph_test_builder.hh"
#include "ir/inst.hh"

#include "opt/inlining.hh"

class InliningTest : public ljit::testing::GraphTestBuilder
{
protected:
  InliningTest()
  {
    fillCallee();
    fillCaller();
    inlining = std::make_unique<ljit::Inlining>(func.get());
  }

  std::unique_ptr<ljit::Function> callee;
  std::unique_ptr<ljit::Inlining> inlining;

private:
  void fillCallee()
  {
    genBBs(4, ljit::Type::I64, std::vector{ljit::Type::I64, ljit::Type::I64});
    auto *bb0 = bbs[0];
    auto *bb1 = bbs[1];
    auto *bb2 = bbs[2];
    auto *bb3 = bbs[3];

    auto *v0 = bb0->pushInstBack<ljit::Param>(0U, ljit::Type::I64);
    auto *v1 = bb0->pushInstBack<ljit::Param>(1U, ljit::Type::I64);
    auto *v2 = bb0->pushInstBack<ljit::ConstVal_I64>(1);
    bb0->pushInstBack<ljit::JumpInstr>(bb1);

    auto *v3 = bb1->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kEQ, v0, v1);
    bb1->pushInstBack<ljit::IfInstr>(v3, bb2, bb3);

    auto *v4 = bb2->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, v0, v2);
    bb2->pushInstBack<ljit::Ret>(v4);

    auto *v5 = bb3->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kSub, v1, v2);
    bb3->pushInstBack<ljit::Ret>(v5);

    callee.swap(func);
  }
  void fillCaller()
  {
    genBBs(2);
    auto *bb0 = bbs[0];
    auto *bb1 = bbs[1];

    auto *v0 = bb0->pushInstBack<ljit::ConstVal_I64>(1);
    auto *v1 = bb0->pushInstBack<ljit::ConstVal_I64>(5);
    bb0->pushInstBack<ljit::JumpInstr>(bb1);

    auto *v2 = bb1->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kAdd, v0, v1);
    auto *v3 = bb1->pushInstBack<ljit::Call>(callee.get());
    v3->appendArg(v2);
    v3->appendArg(v0);

    auto *v4 = bb1->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, v3, v1);
    bb1->pushInstBack<ljit::Ret>(v4);
  }
};

TEST_F(InliningTest, lecture)
{
  // Act
  inlining->run();

  // Assert
}
