#include <algorithm>
#include <functional>
#include <gtest/gtest.h>

#include "../graph/graph_test_builder.hh"
#include <initializer_list>
#include <unordered_set>

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
  using LoopAnalyzer = ljit::LoopAnalyzer<ljit::BasicBlockGraph>;
  using LoopInfo = typename LoopAnalyzer::LoopInfo;

  [[nodiscard]] const auto *getLoopInfo(std::size_t bbId) const
  {
    return loopAnalyzer.getLoopInfo(bbs.at(bbId));
  }

  [[nodiscard]] static bool checkInners(const LoopInfo *loop,
                                        std::vector<const LoopInfo *> inners)
  {
    auto innersOrig = loop->getInners();
    std::sort(innersOrig.begin(), innersOrig.end());
    std::sort(inners.begin(), inners.end());

    return std::equal(inners.begin(), inners.end(), innersOrig.begin(),
                      innersOrig.end());
  }

  LoopAnalyzer loopAnalyzer;
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
  const auto *const l1 = getLoopInfo(0);
  const auto *const l2 = getLoopInfo(1);
  // Assert
  ASSERT_EQ(l1, l2);
  EXPECT_TRUE(l1->isRoot());
  EXPECT_EQ(l1->getOuterLoop(), nullptr);
  EXPECT_TRUE(l1->contains(bbs[0]));
  EXPECT_TRUE(l1->contains(bbs[1]));
}

TEST_F(LoopAnalyzerTest, simpleLoop)
{
  // Assign
  genBBs(2);
  // Just 0 <-> 1
  makeEdge(0, 1);
  makeEdge(1, 0);

  // Act
  buildLoopAnalyzer();
  const auto *l1 = getLoopInfo(0);
  const auto *l2 = getLoopInfo(1);

  // Assert
  ASSERT_EQ(l1, l2);
  EXPECT_FALSE(l1->isRoot());
  EXPECT_EQ(l1->getOuterLoop(), nullptr);
  EXPECT_TRUE(l1->reducible());
  EXPECT_TRUE(l1->contains(bbs[0]));
  EXPECT_TRUE(l1->contains(bbs[1]));
  EXPECT_TRUE(l1->getInners().empty());
}

TEST_F(LoopAnalyzerTest, example1)
{
  // Assign
  buildExample1();

  // Act
  buildLoopAnalyzer();
  const auto *const root = getLoopInfo(0);

  // Assert
  for (std::size_t i = 1; i < bbs.size(); ++i)
    ASSERT_EQ(root, getLoopInfo(i));

  EXPECT_TRUE(root->isRoot());

  for (const auto &bb : bbs)
    EXPECT_TRUE(root->contains(bb));

  EXPECT_EQ(root->getOuterLoop(), nullptr);
  EXPECT_TRUE(root->getInners().empty());
}

TEST_F(LoopAnalyzerTest, example2)
{
  // Assign
  buildExample2();

  // Act
  buildLoopAnalyzer();
  const auto *root = getLoopInfo(0);
  const auto *l1 = getLoopInfo(1);
  const auto *l2 = getLoopInfo(2);
  const auto *l3 = getLoopInfo(4);

  // Assert
  ASSERT_EQ((std::unordered_set{root, l1, l2, l3}.size()), 4);

  EXPECT_TRUE(root->isRoot());
  EXPECT_EQ(getLoopInfo(8), root);
  EXPECT_EQ(getLoopInfo(10), root);
  EXPECT_EQ(root->getOuterLoop(), nullptr);
  EXPECT_TRUE(checkInners(root, {l1}));

  EXPECT_FALSE(l1->isRoot());
  EXPECT_EQ(getLoopInfo(6), l1);
  EXPECT_EQ(getLoopInfo(7), l1);
  EXPECT_EQ(getLoopInfo(9), l1);
  EXPECT_EQ(l1->getOuterLoop(), root);
  EXPECT_TRUE(l1->reducible());
  EXPECT_TRUE(checkInners(l1, {l3, l2}));

  EXPECT_FALSE(l2->isRoot());
  EXPECT_EQ(getLoopInfo(3), l2);
  EXPECT_EQ(l2->getOuterLoop(), l1);
  EXPECT_TRUE(l2->reducible());
  EXPECT_TRUE(checkInners(l2, {}));

  EXPECT_FALSE(l3->isRoot());
  EXPECT_EQ(getLoopInfo(5), l3);
  EXPECT_EQ(l3->getOuterLoop(), l1);
  EXPECT_TRUE(l3->reducible());
  EXPECT_TRUE(checkInners(l3, {}));
}

TEST_F(LoopAnalyzerTest, example3)
{
  // Assign
  buildExample3();

  // Act
  buildLoopAnalyzer();
  const auto *root = getLoopInfo(0);
  const auto *irrLoop = getLoopInfo(2);
  const auto *loop = getLoopInfo(1);

  // Assert
  ASSERT_EQ((std::unordered_set{root, irrLoop, loop}.size()), 3);

  EXPECT_TRUE(root->isRoot());
  EXPECT_EQ(getLoopInfo(3), root);
  EXPECT_EQ(getLoopInfo(7), root);
  EXPECT_EQ(getLoopInfo(8), root);
  EXPECT_EQ(root->getOuterLoop(), nullptr);
  EXPECT_TRUE(checkInners(root, {loop, irrLoop}));

  EXPECT_FALSE(irrLoop->isRoot());
  EXPECT_FALSE(irrLoop->reducible());
  EXPECT_EQ(getLoopInfo(6), irrLoop);
  EXPECT_EQ(irrLoop->getOuterLoop(), root);
  EXPECT_TRUE(checkInners(irrLoop, {}));

  EXPECT_FALSE(loop->isRoot());
  EXPECT_TRUE(loop->reducible());
  EXPECT_EQ(getLoopInfo(4), loop);
  EXPECT_EQ(getLoopInfo(5), loop);
  EXPECT_EQ(loop->getOuterLoop(), root);
  EXPECT_TRUE(checkInners(loop, {}));
}

TEST_F(LoopAnalyzerTest, example4)
{
  // Assign
  buildExample4();

  // Act
  buildLoopAnalyzer();
  const auto *root = getLoopInfo(0);
  const auto *loop = getLoopInfo(1);

  // Assert
  ASSERT_NE(root, loop);

  EXPECT_TRUE(root->isRoot());
  EXPECT_EQ(getLoopInfo(2), root);
  EXPECT_EQ(root->getOuterLoop(), nullptr);
  EXPECT_TRUE(checkInners(root, {loop}));

  EXPECT_FALSE(loop->isRoot());
  EXPECT_TRUE(loop->reducible());
  EXPECT_EQ(getLoopInfo(3), loop);
  EXPECT_EQ(getLoopInfo(4), loop);
  EXPECT_EQ(loop->getOuterLoop(), root);
  EXPECT_TRUE(checkInners(loop, {}));
}
