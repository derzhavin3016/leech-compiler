#include "gtest/gtest.h"
#include <algorithm>
#include <cstddef>
#include <gtest/gtest.h>
#include <memory>

#include "../graph/graph_test_builder.hh"
#include "analysis/liveness.hh"
#include "ir/basic_block.hh"

class LinearOrderTest : public ljit::testing::GraphTestBuilder
{
protected:
  LinearOrderTest()
  {}
  auto checkLinearOrder(std::vector<std::size_t> expected)
  {
    ljit::LivenessAnalyzer liveness(func->makeBBGraph());
    auto &&linOrder = liveness.getLinearOrder();
    std::vector<std::size_t> got(linOrder.size());
    std::transform(linOrder.begin(), linOrder.end(), got.begin(),
                   [](const auto *bb) { return bb->getId(); });

    if (expected == got)
      return testing::AssertionSuccess();

    auto &&ost = testing::AssertionFailure();
    auto &&printIds = [&](const auto &vec) {
      ost << "{";
      for (auto id : vec)
        ost << id << ", ";
      ost << "}\n";
    };

    ost << "Expected: ";
    printIds(expected);
    ost << "Got: ";
    printIds(got);

    return ost;
  }
};

TEST_F(LinearOrderTest, example1)
{
  buildExample1();

  ASSERT_TRUE(checkLinearOrder({0, 1, 5, 6, 4, 2, 3}));
}

TEST_F(LinearOrderTest, example2)
{
  buildExample2();

  ASSERT_TRUE(checkLinearOrder({0, 1, 9, 2, 3, 4, 5, 6, 7, 8, 10}));
}

TEST_F(LinearOrderTest, example3)
{
  buildExample3();

  ASSERT_TRUE(checkLinearOrder({0, 1, 4, 5, 7, 2, 3, 6, 8}));
}

TEST_F(LinearOrderTest, example4)
{
  buildExample4();

  ASSERT_TRUE(checkLinearOrder({0, 1, 3, 4, 2}));
}

TEST_F(LinearOrderTest, example5)
{
  buildExample5();

  ASSERT_TRUE(checkLinearOrder({0, 1, 2, 4, 5, 3}));
}

TEST_F(LinearOrderTest, example6)
{
  buildExample6();

  ASSERT_TRUE(checkLinearOrder({0, 1, 3, 2, 5, 6, 7, 4}));
}
