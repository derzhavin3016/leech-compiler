#include <gtest/gtest.h>

#include "ir/basic_block.hh"
#include "ir/function.hh"
#include "ir/inst.hh"

// Code:
// int64_t fact(int32_t n) {
//     int64_t res{1};
//     for(int32_t i{2}; i <= n; ++i) {
//         res *= i;
//     }
//     return res;
// }

// IR:
// def fact(v0 : i32) -> i64 {
//   bb0:
//     v1 = const i64 1 // res
//     v2 = const i32 2 // i
//     jmp bb1 // check_cond

//   bb1:
//     v3 = phi i32 [v2, bb0], [v7, bb2]
//     v4 = cmp le v3, v0
//     v5 = phi i64 [v1, bb0], [v9, bb2]
//     if v4, bb2, bb3

//   bb2:
//     v6 = const i32 1
//     v7 = add i32 v3, v6
//     v8 = cast v3 to i64
//     v9 = mul i64 v5, v8
//     jmp bb1
//

//   bb3:
//     ret v5
// }
TEST(Builder, Fibonacci)
{
  auto func = ljit::Function{};
  auto *v0 = func.appendParam(ljit::Type::I32);

  auto *bb0 = func.appendBB();
  auto *bb1 = func.appendBB();
  auto *bb2 = func.appendBB();
  auto *bb3 = func.appendBB();

  // build bb0
  auto *v1 = bb0->pushInstBack<ljit::ConstVal_I64>(1);
  auto *v2 = bb0->pushInstBack<ljit::ConstVal_I32>(2);
  bb0->pushInstBack<ljit::JumpInstr>(bb1);

  {
    ASSERT_EQ(v1->getType(), ljit::Type::I64);
    ASSERT_EQ(v2->getType(), ljit::Type::I32);

    ASSERT_EQ(v1->getVal(), 1);
    ASSERT_EQ(v2->getVal(), 2);

    ASSERT_EQ(v1->getNext(), v2);
    ASSERT_EQ(v2->getPrev(), v1);

    ASSERT_EQ(v1->getBB(), bb0);
    ASSERT_EQ(v2->getBB(), bb0);
  }

  // build bb1
  auto *v3 = bb1->pushInstBack<ljit::Phi>(ljit::Type::I32);
  auto *v4 = bb1->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kLE, v3, v0);
  auto *v5 = bb1->pushInstBack<ljit::Phi>(ljit::Type::I64);

  auto *iIf = bb1->pushInstBack<ljit::IfInstr>(v4, bb2, bb3);

  {
    ASSERT_EQ(v3->getNext(), v4);
    ASSERT_EQ(v4->getNext(), v5);
    ASSERT_EQ(v5->getNext(), iIf);

    ASSERT_EQ(v4->getPrev(), v3);
    ASSERT_EQ(v5->getPrev(), v4);
    ASSERT_EQ(iIf->getPrev(), v5);

    ASSERT_EQ(v3->getType(), ljit::Type::I32);
    ASSERT_EQ(v5->getType(), ljit::Type::I64);

    ASSERT_EQ(iIf->getTrueBB(), bb2);
    ASSERT_EQ(iIf->getFalseBB(), bb3);
  }
  // build bb2
  auto *v6 = bb2->pushInstBack<ljit::ConstVal_I32>(1);
  auto *v7 = bb2->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kAdd, v3, v6);
  auto *v8 = bb2->pushInstBack<ljit::Cast>(ljit::Type::I64, v3);
  auto *v9 = bb2->pushInstBack<ljit::BinOp>(ljit::BinOp::Oper::kMul, v5, v8);

  auto *jmp = bb2->pushInstBack<ljit::JumpInstr>(bb1);

  {
    ASSERT_EQ(v6->getType(), ljit::Type::I32);
    ASSERT_EQ(v6->getVal(), 1);

    ASSERT_EQ(v7->getType(), ljit::Type::I32);
    ASSERT_EQ(v8->getType(), ljit::Type::I64);
    ASSERT_EQ(v9->getType(), ljit::Type::I64);
    ASSERT_EQ(jmp->getTarget(), bb1);
  }

  // build bb3
  auto *ret = bb2->pushInstBack<ljit::Ret>(v5);
  {
    ASSERT_EQ(ret->getVal(), v5);
  }

  // fill PHI
  v3->addNode(v2, bb0);
  v3->addNode(v7, bb2);

  v5->addNode(v1, bb0);
  v5->addNode(v7, bb2);
}
