#include <gtest/gtest.h>
#include <vector>

#include "opt/checks_elimination.hh"

#include "../graph/graph_test_builder.hh"
#include "ir/inst.hh"

class ChecksEliminationTest : public ljit::testing::GraphTestBuilder
{
protected:
  ChecksEliminationTest() = default;

  ljit::ChecksElimination elim;
};

TEST_F(ChecksEliminationTest, zero)
{
  // Assign
  genBBs(4, ljit::Type::I64, std::vector{ljit::Type::I64, ljit::Type::I64});

  auto *bb0 = bbs[0];
  auto *bb1 = bbs[1];
  auto *bb2 = bbs[2];
  auto *bb3 = bbs[3];

  auto *v0 = bb0->pushInstBack<ljit::Param>(0U, ljit::Type::I64);
  auto *v1 = bb0->pushInstBack<ljit::Param>(1U, ljit::Type::I64);
  auto *v2 = bb0->pushInstBack<ljit::ConstVal_I64>(1);
  bb0->pushInstBack<ljit::JumpInstr>(bb1);

  [[maybe_unused]] auto *check0 =
    bb1->pushInstBack<ljit::UnaryOp>(ljit::UnaryOp::Oper::kZeroCheck, v0);
  auto *v3 = bb1->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kDiv, v2, v0);
  [[maybe_unused]] auto *check1 =
    bb1->pushInstBack<ljit::UnaryOp>(ljit::UnaryOp::Oper::kZeroCheck, v0);
  auto *v4 = bb1->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kDiv, v1, v0);
  auto *v5 = bb1->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kEQ, v3, v4);
  bb1->pushInstBack<ljit::IfInstr>(v5, bb2, bb3);

  [[maybe_unused]] auto *check2 =
    bb2->pushInstBack<ljit::UnaryOp>(ljit::UnaryOp::Oper::kZeroCheck, v0);
  auto *v6 = bb2->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, v0, v2);
  bb2->pushInstBack<ljit::Ret>(v6);

  bb3->pushInstBack<ljit::Ret>(v1);

  // Act
  elim.run(makeGraph());
  // Assert

  EXPECT_EQ(v3->getPrev(), check0);
  EXPECT_EQ(v3->getNext(), v4);
  EXPECT_EQ(v6, &bb2->getFirst());
}

TEST_F(ChecksEliminationTest, bound)
{
  // Assign
  genBBs(4, ljit::Type::I64, std::vector{ljit::Type::I64, ljit::Type::I64});

  auto *bb0 = bbs[0];
  auto *bb1 = bbs[1];
  auto *bb2 = bbs[2];
  auto *bb3 = bbs[3];

  auto *v0 = bb0->pushInstBack<ljit::Param>(0U, ljit::Type::I64);
  auto *v1 = bb0->pushInstBack<ljit::Param>(1U, ljit::Type::I64);
  auto *v2 = bb0->pushInstBack<ljit::ConstVal_I64>(1);
  bb0->pushInstBack<ljit::JumpInstr>(bb1);

  [[maybe_unused]] auto *check0 =
    bb1->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kBoundsCheck, v0, v2);
  auto *v3 = bb1->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kDiv, v2, v0);
  auto *v5 = bb1->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kEQ, v3, v1);
  bb1->pushInstBack<ljit::IfInstr>(v5, bb2, bb3);

  [[maybe_unused]] auto *check1 =
    bb2->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kBoundsCheck, v0, v2);
  auto *v6 = bb2->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, v0, v2);
  bb2->pushInstBack<ljit::Ret>(v6);

  auto *v7 = bb3->pushInstBack<ljit::ConstVal_I64>(42);
  [[maybe_unused]] auto *check2 =
    bb3->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kBoundsCheck, v0, v7);
  bb3->pushInstBack<ljit::Ret>(v1);

  // Act
  elim.run(makeGraph());
  // Assert

  EXPECT_EQ(v3->getPrev(), check0);
  EXPECT_EQ(v6, &bb2->getFirst());
  EXPECT_EQ(v7->getNext(), check2);
}
