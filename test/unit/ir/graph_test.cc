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
//     v3 = phi i32 [v2, bb0], [v6, bb2]
//     v4 = cmp le v3, v0
//     v5 = phi i64 [v1, bb0], [v9, bb2]
//     if v4, bb2, bb3

//   bb2:
//     v6 = add i32 v3, 1
//     v7 = cast v6 to i64
//     v9 = mul i64 v5, v7
//     jmp bb1
//

//   bb3:
//     ret v5
// }
TEST(Builder, Fibonacci)
{
  auto func = ljit::Function{};
}
