#include <algorithm>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <memory>

#include "../graph/graph_test_builder.hh"
#include "graph/dfs.hh"
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
  auto &&graph = ljit::graph::depthFirstSearchReversePostOrder(makeGraph());

  // Assert
  ASSERT_EQ(graph.size(), 6);
#define VAR_BB(idx) auto *bb##idx = graph.at(idx)
  VAR_BB(0);
  VAR_BB(1);
  VAR_BB(2);
  VAR_BB(3);
  VAR_BB(4);
  VAR_BB(5);
#undef VAR_BB

  ASSERT_EQ(bb0->size(), 3);

  // bb0
  auto &last0 = bb0->getLast();
  ASSERT_EQ(last0.getInstType(), ljit::InstType::kJump);
  EXPECT_EQ(dynamic_cast<ljit::JumpInstr &>(last0).getTarget(), bb1);

  // bb1
  {
    ASSERT_TRUE(bb1->collectInsts(ljit::InstType::kCall).empty());
    std::vector<ljit::Inst *> insns1;
    std::transform(bb1->begin(), bb1->end(), std::back_inserter(insns1),
                   [](auto &insn) { return &insn; });
    ASSERT_EQ(insns1.size(), 3);
    EXPECT_EQ(insns1[0]->getInstType(), ljit::InstType::kBinOp);
    EXPECT_EQ(insns1[1]->getInstType(), ljit::InstType::kConst);
    ASSERT_EQ(insns1[2]->getInstType(), ljit::InstType::kJump);
    EXPECT_EQ(dynamic_cast<ljit::JumpInstr &>(*insns1.back()).getTarget(), bb2);
  }

  // bb2
  {
    std::vector<ljit::Inst *> insns;
    std::transform(bb2->begin(), bb2->end(), std::back_inserter(insns),
                   [](auto &insn) { return &insn; });
    ASSERT_EQ(insns.size(), 2);
    ASSERT_EQ(insns[0]->getInstType(), ljit::InstType::kBinOp);
    EXPECT_EQ(dynamic_cast<ljit::BinOp &>(*insns.front()).getOper(),
              ljit::BinOp::Oper::kEQ);
    // Check params
    {
      ASSERT_TRUE(insns.front()->inputAt(0)->isInst());
      ASSERT_TRUE(insns.front()->inputAt(1)->isInst());

      auto *param0 = static_cast<ljit::Inst *>(insns.front()->inputAt(0));
      auto *param1 = static_cast<ljit::Inst *>(insns.front()->inputAt(1));

      EXPECT_EQ(param0->getInstType(), ljit::InstType::kBinOp);
      EXPECT_EQ(param1->getInstType(), ljit::InstType::kConst);
    }

    ASSERT_EQ(insns[1]->getInstType(), ljit::InstType::kIf);
    EXPECT_EQ(dynamic_cast<ljit::IfInstr &>(*insns.back()).getTrueBB(), bb4);
    EXPECT_EQ(dynamic_cast<ljit::IfInstr &>(*insns.back()).getFalseBB(), bb3);
  }

  // bb3
  {
    std::vector<ljit::Inst *> insns;
    std::transform(bb3->begin(), bb3->end(), std::back_inserter(insns),
                   [](auto &insn) { return &insn; });
    ASSERT_EQ(insns.size(), 2);
    ASSERT_EQ(insns[0]->getInstType(), ljit::InstType::kBinOp);
    EXPECT_EQ(dynamic_cast<ljit::BinOp &>(*insns.front()).getOper(),
              ljit::BinOp::Oper::kSub);
    // Check params
    {
      ASSERT_TRUE(insns.front()->inputAt(0)->isInst());
      ASSERT_TRUE(insns.front()->inputAt(1)->isInst());

      auto *param0 = static_cast<ljit::Inst *>(insns.front()->inputAt(0));
      auto *param1 = static_cast<ljit::Inst *>(insns.front()->inputAt(1));

      EXPECT_EQ(param0->getInstType(), ljit::InstType::kConst);
      EXPECT_EQ(param1->getInstType(), ljit::InstType::kConst);
    }

    ASSERT_EQ(insns[1]->getInstType(), ljit::InstType::kJump);
    EXPECT_EQ(dynamic_cast<ljit::JumpInstr &>(*insns.back()).getTarget(), bb5);
  }

  // bb4
  {
    std::vector<ljit::Inst *> insns;
    std::transform(bb4->begin(), bb4->end(), std::back_inserter(insns),
                   [](auto &insn) { return &insn; });
    ASSERT_EQ(insns.size(), 2);
    ASSERT_EQ(insns[0]->getInstType(), ljit::InstType::kBinOp);
    EXPECT_EQ(dynamic_cast<ljit::BinOp &>(*insns.front()).getOper(),
              ljit::BinOp::Oper::kMul);
    // Check params
    {
      ASSERT_TRUE(insns.front()->inputAt(0)->isInst());
      ASSERT_TRUE(insns.front()->inputAt(1)->isInst());

      auto *param0 = static_cast<ljit::Inst *>(insns.front()->inputAt(0));
      auto *param1 = static_cast<ljit::Inst *>(insns.front()->inputAt(1));

      EXPECT_EQ(param0->getInstType(), ljit::InstType::kBinOp);
      EXPECT_EQ(param1->getInstType(), ljit::InstType::kConst);
    }

    ASSERT_EQ(insns[1]->getInstType(), ljit::InstType::kJump);
    EXPECT_EQ(dynamic_cast<ljit::JumpInstr &>(*insns.back()).getTarget(), bb5);
  }

  // bb5
  {
    std::vector<ljit::Inst *> insns;
    std::transform(bb5->begin(), bb5->end(), std::back_inserter(insns),
                   [](auto &insn) { return &insn; });
    ASSERT_EQ(insns.size(), 3);

    ASSERT_EQ(insns.front()->getInstType(), ljit::InstType::kPhi);
    {
      auto &phi = dynamic_cast<ljit::Phi &>(*insns.front());
      std::vector<ljit::Phi::Entry> entries{};
      std::copy(phi.begin(), phi.end(), std::back_inserter(entries));
      ASSERT_EQ(entries.size(), 2);
      EXPECT_EQ(entries[0].bb, bb4);
      EXPECT_EQ(entries[1].bb, bb3);
    }

    ASSERT_EQ(insns[1]->getInstType(), ljit::InstType::kBinOp);
    EXPECT_EQ(dynamic_cast<ljit::BinOp &>(*insns[1]).getOper(),
              ljit::BinOp::Oper::kMul);
    // Check params
    {
      ASSERT_TRUE(insns[1]->inputAt(0)->isInst());
      ASSERT_TRUE(insns[1]->inputAt(1)->isInst());

      auto *param0 = static_cast<ljit::Inst *>(insns[1]->inputAt(0));
      auto *param1 = static_cast<ljit::Inst *>(insns[1]->inputAt(1));

      EXPECT_EQ(param0, insns[0]);
      EXPECT_EQ(param1->getInstType(), ljit::InstType::kConst);
    }

    ASSERT_EQ(insns.back()->getInstType(), ljit::InstType::kRet);
  }
}
