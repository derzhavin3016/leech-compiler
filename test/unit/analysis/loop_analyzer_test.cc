#include <algorithm>
#include <optional>
#include <unordered_set>

#include "gtest/gtest.h"
#include <vector>

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
  using LoopAnalyzer = ljit::LoopAnalyzer<ljit::BasicBlockGraph>;
  using LoopInfo = typename LoopAnalyzer::LoopInfo;

  [[nodiscard]] const auto *getLoopInfo(std::size_t bbId) const
  {
    return loopAnalyzer.getLoopInfo(bbs.at(bbId));
  }

  [[nodiscard]] auto checkHeader(
    const LoopInfo *loop, std::optional<std::size_t> bbId = std::nullopt) const
  {
    const auto *const exp = bbId ? bbs[*bbId] : nullptr;
    const auto *const got = loop->getHeader();
    if (got == exp)
      return testing::AssertionSuccess();

    return testing::AssertionFailure()
           << "Expected: " << exp << ", got: " << got;
  }

  [[nodiscard]] auto checkBackEdges(const LoopInfo *loop,
                                    std::vector<std::size_t> backs)
  {
    auto backEdges = loop->getBackEdgesSrc();
    decltype(backEdges) expectEdges(backs.size());

    std::transform(backs.begin(), backs.end(), expectEdges.begin(),
                   [&](auto id) { return bbs.at(id); });

    std::sort(expectEdges.begin(), expectEdges.end());
    std::sort(backEdges.begin(), backEdges.end());

    if (backEdges != expectEdges)
    {
      auto os = testing::AssertionFailure();
      os << "Expected back edges: [ ";
      for (const auto *p : expectEdges)
        os << p << " ";

      os << "], got: [ ";
      for (const auto *p : backEdges)
        os << p << " ";

      os << "]";
      return os;
    }

    return testing::AssertionSuccess();
  }

  [[nodiscard]] static auto checkInners(const LoopInfo *loop,
                                        std::vector<const LoopInfo *> inners)
  {
    decltype(inners) innersOrig{loop->getInners().begin(),
                                loop->getInners().end()};

    std::sort(innersOrig.begin(), innersOrig.end());
    std::sort(inners.begin(), inners.end());

    if (inners != innersOrig)
    {
      auto os = testing::AssertionFailure();
      os << "Expected inners: [ ";
      for (const auto *p : inners)
        os << p << " ";

      os << "], got: [ ";
      for (const auto *p : innersOrig)
        os << p << " ";

      os << "]";
      return os;
    }

    return testing::AssertionSuccess();
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
  EXPECT_TRUE(checkHeader(l1));
  EXPECT_TRUE(checkBackEdges(l1, {}));
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

  auto v1 = l1->getBodyAsVector();
  auto bbsC = toConstBBs();
  std::sort(v1.begin(), v1.end());
  std::sort(bbsC.begin(), bbsC.end());
  EXPECT_EQ(v1, bbsC);

  EXPECT_TRUE(checkHeader(l1, 0));
  EXPECT_TRUE(checkBackEdges(l1, {1}));
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
  EXPECT_TRUE(checkHeader(root));
  EXPECT_TRUE(checkBackEdges(root, {}));

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
  EXPECT_TRUE(checkHeader(root));
  EXPECT_TRUE(checkBackEdges(root, {}));
  EXPECT_EQ(getLoopInfo(8), root);
  EXPECT_EQ(getLoopInfo(10), root);
  EXPECT_EQ(root->getOuterLoop(), nullptr);
  EXPECT_TRUE(checkInners(root, {l1}));

  EXPECT_FALSE(l1->isRoot());
  EXPECT_TRUE(checkHeader(l1, 1));
  EXPECT_TRUE(checkBackEdges(l1, {7}));
  EXPECT_EQ(getLoopInfo(6), l1);
  EXPECT_EQ(getLoopInfo(7), l1);
  EXPECT_EQ(getLoopInfo(9), l1);
  EXPECT_EQ(l1->getOuterLoop(), root);
  EXPECT_TRUE(l1->reducible());
  EXPECT_TRUE(checkInners(l1, {l3, l2}));

  EXPECT_FALSE(l2->isRoot());
  EXPECT_TRUE(checkHeader(l2, 2));
  EXPECT_TRUE(checkBackEdges(l2, {3}));
  EXPECT_EQ(getLoopInfo(3), l2);
  EXPECT_EQ(l2->getOuterLoop(), l1);
  EXPECT_TRUE(l2->reducible());
  EXPECT_TRUE(checkInners(l2, {}));

  EXPECT_FALSE(l3->isRoot());
  EXPECT_TRUE(checkBackEdges(l3, {5}));
  EXPECT_TRUE(checkHeader(l3, 4));
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
  EXPECT_TRUE(checkHeader(root));
  EXPECT_TRUE(checkBackEdges(root, {}));
  EXPECT_EQ(getLoopInfo(3), root);
  EXPECT_EQ(getLoopInfo(7), root);
  EXPECT_EQ(getLoopInfo(8), root);
  EXPECT_EQ(root->getOuterLoop(), nullptr);
  EXPECT_TRUE(checkInners(root, {loop, irrLoop}));

  EXPECT_FALSE(irrLoop->isRoot());
  EXPECT_FALSE(irrLoop->reducible());
  EXPECT_TRUE(checkBackEdges(irrLoop, {6}));
  EXPECT_TRUE(checkHeader(irrLoop, 2));
  EXPECT_EQ(getLoopInfo(6), irrLoop);
  EXPECT_EQ(irrLoop->getOuterLoop(), root);
  EXPECT_TRUE(checkInners(irrLoop, {}));

  EXPECT_FALSE(loop->isRoot());
  EXPECT_TRUE(loop->reducible());
  EXPECT_TRUE(checkHeader(loop, 1));
  EXPECT_TRUE(checkBackEdges(loop, {5}));
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
  EXPECT_TRUE(checkHeader(root));
  EXPECT_TRUE(checkBackEdges(root, {}));
  EXPECT_EQ(getLoopInfo(2), root);
  EXPECT_EQ(root->getOuterLoop(), nullptr);
  EXPECT_TRUE(checkInners(root, {loop}));

  EXPECT_FALSE(loop->isRoot());
  EXPECT_TRUE(checkHeader(loop, 1));
  EXPECT_TRUE(checkBackEdges(loop, {4}));
  EXPECT_TRUE(loop->reducible());
  EXPECT_EQ(getLoopInfo(3), loop);
  EXPECT_EQ(getLoopInfo(4), loop);
  EXPECT_EQ(loop->getOuterLoop(), root);
  EXPECT_TRUE(checkInners(loop, {}));
}

TEST_F(LoopAnalyzerTest, example5)
{
  // Assign
  buildExample5();

  // Act
  buildLoopAnalyzer();
  const auto *root = getLoopInfo(0);
  const auto *loop = getLoopInfo(1);

  // Assert
  ASSERT_NE(root, loop);

  EXPECT_TRUE(root->isRoot());
  EXPECT_TRUE(checkBackEdges(root, {}));
  EXPECT_TRUE(checkHeader(root));
  EXPECT_EQ(getLoopInfo(3), root);
  EXPECT_EQ(root->getOuterLoop(), nullptr);
  EXPECT_TRUE(checkInners(root, {loop}));

  EXPECT_FALSE(loop->isRoot());
  EXPECT_TRUE(checkHeader(loop, 1));
  EXPECT_TRUE(checkBackEdges(loop, {5}));
  EXPECT_TRUE(loop->reducible());
  EXPECT_EQ(getLoopInfo(2), loop);
  EXPECT_EQ(getLoopInfo(4), loop);
  EXPECT_EQ(getLoopInfo(5), loop);
  EXPECT_EQ(loop->getOuterLoop(), root);
  EXPECT_TRUE(checkInners(loop, {}));
}

TEST_F(LoopAnalyzerTest, example6)
{
  // Assign
  buildExample6();

  // Act
  buildLoopAnalyzer();
  const auto *root = getLoopInfo(4);
  const auto *mainLoop = getLoopInfo(0);
  const auto *loop = getLoopInfo(1);

  // Assert
  ASSERT_EQ((std::unordered_set{root, mainLoop, loop}.size()), 3);

  EXPECT_TRUE(root->isRoot());
  EXPECT_TRUE(checkHeader(root));
  EXPECT_TRUE(checkBackEdges(root, {}));
  EXPECT_EQ(root->getOuterLoop(), nullptr);
  EXPECT_TRUE(checkInners(root, {mainLoop}));

  EXPECT_FALSE(mainLoop->isRoot());
  EXPECT_TRUE(checkBackEdges(mainLoop, {7}));
  EXPECT_TRUE(checkHeader(mainLoop, 0));
  EXPECT_TRUE(mainLoop->reducible());
  EXPECT_EQ(getLoopInfo(7), mainLoop);
  EXPECT_EQ(mainLoop->getOuterLoop(), root);
  EXPECT_TRUE(checkInners(mainLoop, {loop}));

  EXPECT_FALSE(loop->isRoot());
  EXPECT_TRUE(checkBackEdges(loop, {6}));
  EXPECT_TRUE(checkHeader(loop, 1));
  EXPECT_TRUE(loop->reducible());
  EXPECT_EQ(getLoopInfo(2), loop);
  EXPECT_EQ(getLoopInfo(3), loop);
  EXPECT_EQ(getLoopInfo(5), loop);
  EXPECT_EQ(getLoopInfo(6), loop);
  EXPECT_EQ(loop->getOuterLoop(), mainLoop);
  EXPECT_TRUE(checkInners(loop, {}));
}
