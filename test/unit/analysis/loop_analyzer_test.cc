#include <gtest/gtest.h>

#include "../graph/graph_test_builder.hh"

#include "analysis/loop_analyzer.hh"
#include "ir/basic_block.hh"

class LoopAnalyzerTest : public ljit::testing::GraphTestBuilder
{
protected:
  LoopAnalyzerTest() = default;

  void buildLoopAnalyzer()
  {
    loopAnalyzer = decltype(loopAnalyzer){func->makeBBGraph()};
  }

  ljit::LoopAnalyzer<ljit::BasicBlockGraph> loopAnalyzer;
};

TEST_F(LoopAnalyzerTest, basic)
{
  // Assign
  constexpr std::size_t kSize = 2;
  genBBs(kSize);
  // Just 0 -> 1
  makeEdge(0, 1);

  // Act
  buildLoopAnalyzer();
  const auto *const l1 = loopAnalyzer.getLoopInfo(bbs[0]);
  const auto *const l2 = loopAnalyzer.getLoopInfo(bbs[1]);
  // Assert
  ASSERT_EQ(l1, l2);
  EXPECT_TRUE(l1->isRoot());
  EXPECT_EQ(l1->getOuterLoop(), nullptr);
  EXPECT_TRUE(l1->contains(bbs[0]));
  EXPECT_TRUE(l1->contains(bbs[1]));
}
