#include <gtest/gtest.h>

#include "graph_test_builder.hh"

#include "graph/dom_tree.hh"
#include "ir/basic_block.hh"

class DomTreeTest : public ljit::testing::GraphTestBuilder
{
protected:
  DomTreeTest() = default;

  void buildDomTree()
  {
    domTree = ljit::graph::buildDomTree(func->makeBBGraph());
  }

  [[nodiscard]] bool isDom(std::size_t dom, std::size_t node) const
  {
    return domTree.isDominator(bbs[dom], bbs[node]);
  }

  ljit::graph::DominatorTree<ljit::BasicBlockGraph> domTree;
};

TEST_F(DomTreeTest, simplest)
{
  // Assign
  constexpr std::size_t kSize = 2;
  genBBs(kSize);
  // Just 0 -> 1
  makeEdge(0, 1);

  // Act
  buildDomTree();
  // Assert
  EXPECT_TRUE(isDom(0, 1));
  EXPECT_TRUE(isDom(1, 1));
  EXPECT_TRUE(isDom(0, 0));
  EXPECT_FALSE(isDom(1, 0));
}

TEST_F(DomTreeTest, example1)
{
  // Assign
  buildExample1();

  // Act
  buildDomTree();
  domTree.dump(std::clog);

  // Assert
  for (const auto &bb : bbs)
    ASSERT_TRUE(domTree.isDominator(bb, bb));

  EXPECT_TRUE(isDom(0, 1));
  EXPECT_TRUE(isDom(1, 2));
  EXPECT_TRUE(isDom(1, 5));
  EXPECT_TRUE(isDom(1, 3));
  EXPECT_TRUE(isDom(5, 4));
  EXPECT_TRUE(isDom(5, 6));
}

TEST_F(DomTreeTest, example2)
{
  // Assign
  buildExample2();

  // Act
  buildDomTree();

  // Assert
  EXPECT_TRUE(isDom(0, 1));

  EXPECT_TRUE(isDom(1, 9));
  EXPECT_TRUE(isDom(1, 2));

  EXPECT_TRUE(isDom(2, 3));
  EXPECT_TRUE(isDom(3, 4));
  EXPECT_TRUE(isDom(4, 5));
  EXPECT_TRUE(isDom(5, 6));

  EXPECT_TRUE(isDom(6, 7));
  EXPECT_TRUE(isDom(6, 8));

  EXPECT_TRUE(isDom(8, 10));
}

TEST_F(DomTreeTest, example3)
{
  // Assign
  buildExample3();

  // Act
  buildDomTree();

  // Assert
  EXPECT_TRUE(isDom(0, 1));

  EXPECT_TRUE(isDom(1, 2));
  EXPECT_TRUE(isDom(1, 4));
  EXPECT_TRUE(isDom(1, 3));
  EXPECT_TRUE(isDom(1, 6));
  EXPECT_TRUE(isDom(1, 8));

  EXPECT_TRUE(isDom(4, 5));

  EXPECT_TRUE(isDom(5, 7));
}