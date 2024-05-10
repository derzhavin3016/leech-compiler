#include <gtest/gtest.h>
#include <type_traits>

#include "opt/peephole.hh"

#include "../graph/graph_test_builder.hh"
#include "ir/inst.hh"

class PeepHoleTest : public ljit::testing::GraphTestBuilder
{
protected:
  ljit::PeepHole pHole;
};

TEST_F(PeepHoleTest, add)
{
  // Assign
  genBBs(1);
  auto *const lval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(32);
  auto *const rval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(0);
  auto *const add =
    bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kAdd, lval, rval);
  bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, add, lval);
  const auto &graph = makeGraph();
  // Act
  pHole.run(graph);
  // Assert
  ASSERT_EQ(bbs[0]->size(), 3);
  const auto &inst = bbs[0]->getLast();
  ASSERT_EQ(inst.getInstType(), ljit::InstType::kBinOp);
  ASSERT_EQ(inst.getType(), ljit::Type::I64);
  const auto &op = static_cast<const ljit::BinOp &>(inst);
  EXPECT_EQ(op.getOper(), ljit::BinOp::Oper::kMul);
  EXPECT_EQ(op.getLeft(), lval);
}

TEST_F(PeepHoleTest, shr)
{
  // Assign
  genBBs(1);
  auto *const lval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(32);
  auto *const rval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(0);
  auto *const add =
    bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kShr, lval, rval);
  bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, add, lval);
  const auto &graph = makeGraph();
  // Act
  pHole.run(graph);
  // Assert
  ASSERT_EQ(bbs[0]->size(), 3);
  const auto &inst = bbs[0]->getLast();
  ASSERT_EQ(inst.getInstType(), ljit::InstType::kBinOp);
  ASSERT_EQ(inst.getType(), ljit::Type::I64);
  const auto &op = static_cast<const ljit::BinOp &>(inst);
  EXPECT_EQ(op.getOper(), ljit::BinOp::Oper::kMul);
  EXPECT_EQ(op.getLeft(), lval);
}

TEST_F(PeepHoleTest, shr2)
{
  // Assign
  genBBs(1);
  auto *const two = bbs[0]->pushInstBack<ljit::ConstVal_I64>(2);
  auto *const one = bbs[0]->pushInstBack<ljit::ConstVal_I64>(1);
  auto *const v0 =
    bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, two, one);

  auto *const fst =
    bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kShr, v0, two);
  auto *const sec =
    bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kShr, fst, one);

  [[maybe_unused]] auto *const user =
    bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, sec, one);

  const auto &graph = makeGraph();
  // Act
  pHole.run(graph);
  // Assert
  ASSERT_EQ(bbs[0]->size(), 6);
  const auto &inst = bbs[0]->getLast();
  ASSERT_EQ(inst.getInstType(), ljit::InstType::kBinOp);
  ASSERT_EQ(inst.getType(), ljit::Type::I64);
  const auto &op = static_cast<const ljit::BinOp &>(inst);
  EXPECT_EQ(op.getOper(), ljit::BinOp::Oper::kMul);
  EXPECT_EQ(op.getLeft(), sec);

  auto &prev = *std::prev(bbs[0]->end(), 2);
  auto &pprev = *std::prev(bbs[0]->end(), 3);
  ASSERT_EQ(prev.getInstType(), ljit::InstType::kBinOp);
  ASSERT_EQ(pprev.getInstType(), ljit::InstType::kBinOp);

  auto &add = static_cast<ljit::BinOp &>(pprev);
  auto &shr = static_cast<ljit::BinOp &>(prev);

  ASSERT_EQ(add.getOper(), ljit::BinOp::Oper::kAdd);
  ASSERT_EQ(shr.getOper(), ljit::BinOp::Oper::kShr);

  EXPECT_EQ(shr.getLeft(), v0);
  EXPECT_EQ(shr.getRight(), &add);
}

TEST_F(PeepHoleTest, or)
{
  // Assign
  genBBs(1);
  auto *const lval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(32);
  auto *const rval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(0);
  auto *const add =
    bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kOr, lval, rval);
  bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, add, lval);
  const auto &graph = makeGraph();
  // Act
  pHole.run(graph);
  // Assert
  ASSERT_EQ(bbs[0]->size(), 3);
  const auto &inst = bbs[0]->getLast();
  ASSERT_EQ(inst.getInstType(), ljit::InstType::kBinOp);
  ASSERT_EQ(inst.getType(), ljit::Type::I64);
  const auto &op = static_cast<const ljit::BinOp &>(inst);
  EXPECT_EQ(op.getOper(), ljit::BinOp::Oper::kMul);
  EXPECT_EQ(op.getLeft(), lval);
}

TEST_F(PeepHoleTest, or2)
{
  // Assign
  genBBs(1);
  auto *const lvalPrev = bbs[0]->pushInstBack<ljit::ConstVal_I64>(32);
  auto *const lval = bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul,
                                                       lvalPrev, lvalPrev);
  auto *const rval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(-1);
  auto *const add =
    bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kOr, rval, lval);
  bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, add, lval);
  const auto &graph = makeGraph();
  // Act
  pHole.run(graph);
  // Assert
  ASSERT_EQ(bbs[0]->size(), 4);
  const auto &inst = bbs[0]->getLast();
  ASSERT_EQ(inst.getInstType(), ljit::InstType::kBinOp);
  ASSERT_EQ(inst.getType(), ljit::Type::I64);
  const auto &op = static_cast<const ljit::BinOp &>(inst);
  EXPECT_EQ(op.getOper(), ljit::BinOp::Oper::kMul);
  EXPECT_EQ(op.getLeft(), rval);
}
