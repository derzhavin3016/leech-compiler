#include <gtest/gtest.h>

#include "opt/constant_folding.hh"

#include "../graph/graph_test_builder.hh"
#include "ir/inst.hh"

class ConstFoldTest : public ljit::testing::GraphTestBuilder
{
protected:
  ConstFoldTest() = default;

  ljit::ConstantFolding cFold;
};

TEST_F(ConstFoldTest, addSimple)
{
  // Assign
  genBBs(1);
  auto *const lval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(32);
  auto *const rval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(10);
  bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kAdd, lval, rval);
  const auto &graph = makeGraph();
  // Act
  cFold.run(graph);
  // Assert
  ASSERT_EQ(bbs[0]->size(), 3);
  const auto &inst = bbs[0]->getLast();
  ASSERT_EQ(inst.getInstType(), ljit::InstType::kConst);
  ASSERT_EQ(inst.getType(), ljit::Type::I64);
  const auto &const_ = static_cast<const ljit::ConstVal_I64 &>(inst);
  EXPECT_EQ(const_.getVal(), 42);
}

TEST_F(ConstFoldTest, shrSimple)
{
  // Assign
  genBBs(1);
  auto *const lval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(32);
  auto *const rval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(2);
  bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kShr, lval, rval);
  const auto &graph = makeGraph();
  // Act
  cFold.run(graph);
  // Assert
  ASSERT_EQ(bbs[0]->size(), 3);
  const auto &inst = bbs[0]->getLast();
  ASSERT_EQ(inst.getInstType(), ljit::InstType::kConst);
  ASSERT_EQ(inst.getType(), ljit::Type::I64);
  const auto &const_ = static_cast<const ljit::ConstVal_I64 &>(inst);
  EXPECT_EQ(const_.getVal(), 8);
}

TEST_F(ConstFoldTest, orSimple)
{
  // Assign
  genBBs(1);
  auto *const lval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(32);
  auto *const rval = bbs[0]->pushInstBack<ljit::ConstVal_I64>(2);
  bbs[0]->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kOr, lval, rval);
  const auto &graph = makeGraph();
  // Act
  cFold.run(graph);
  // Assert
  ASSERT_EQ(bbs[0]->size(), 3);
  const auto &inst = bbs[0]->getLast();
  ASSERT_EQ(inst.getInstType(), ljit::InstType::kConst);
  ASSERT_EQ(inst.getType(), ljit::Type::I64);
  const auto &const_ = static_cast<const ljit::ConstVal_I64 &>(inst);
  EXPECT_EQ(const_.getVal(), 34);
}
