#include <gtest/gtest.h>

#include "graph_test_builder.hh"

#include "graph/dom_tree.hh"

class DomTreeTest : public ljit::testing::GraphTestBuilder
{
protected:
  DomTreeTest() = default;
};

TEST_F(DomTreeTest, simplest)
{
  // Assign
  constexpr std::size_t kSize = 2;
  genBBs(kSize);
  // Just 0 -> 1
  makeEdge(0, 1);

  // Act
  auto res = ljit::graph::buildDomTree(func->makeBBGraph());
  // Assert
  EXPECT_TRUE(res.isDominator(bbs[0], bbs[1]));
  EXPECT_TRUE(res.isDominator(bbs[1], bbs[1]));
  EXPECT_TRUE(res.isDominator(bbs[0], bbs[0]));
  EXPECT_FALSE(res.isDominator(bbs[1], bbs[0]));
}

TEST_F(DomTreeTest, example1)
{
  // Assign
  constexpr std::size_t kSize = 7;
  genBBs(kSize);

  /* Example 1
   *                     0
   *                     |
   *                     1---
   *                    /    \
   *                   2  4--5
   *                   | /   |
   *                   3---- 6
   * Directions:
   * 0 -> 1
   *
   * 1 -> 2
   * 1 -> 5
   *
   * 2 -> 3
   *
   * 5 -> 4
   * 5 -> 6
   *
   * 4 -> 3
   *
   * 6 -> 3
   */
  makeEdge(0, 1);

  makeEdge(1, 2);
  makeEdge(1, 5);

  makeEdge(2, 3);

  makeEdge(5, 4);
  makeEdge(5, 6);

  makeEdge(4, 3);

  makeEdge(6, 3);

  // Act
  auto domTree = ljit::graph::buildDomTree(func->makeBBGraph());
  domTree.dump(std::clog);
  // Assert
  auto &&isDom = [&](std::size_t dom, std::size_t node) {
    return domTree.isDominator(bbs[dom], bbs[node]);
  };
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
  constexpr std::size_t kSize = 11;
  genBBs(kSize);
  auto &&toId = [](char letter) -> std::size_t {
    return static_cast<std::size_t>(letter) - static_cast<std::size_t>('A');
  };
  auto &&makeLEdge = [&](char pred, char succ) {
    makeEdge(toId(pred), toId(succ));
  };

  makeLEdge('A', 'B');

  makeLEdge('B', 'J');
  makeLEdge('B', 'C');

  makeLEdge('C', 'D');

  makeLEdge('D', 'C');
  makeLEdge('D', 'E');

  makeLEdge('E', 'F');

  makeLEdge('F', 'E');
  makeLEdge('F', 'G');

  makeLEdge('G', 'H');
  makeLEdge('G', 'I');

  makeLEdge('H', 'B');

  makeLEdge('I', 'K');

  makeLEdge('J', 'C');

  // Act
  auto domTree = ljit::graph::buildDomTree(func->makeBBGraph());
  // Assert
  auto &&isDom = [&](char dom, char node) {
    return domTree.isDominator(bbs[toId(dom)], bbs[toId(node)]);
  };
  EXPECT_TRUE(isDom('A', 'B'));

  EXPECT_TRUE(isDom('B', 'J'));
  EXPECT_TRUE(isDom('B', 'C'));

  EXPECT_TRUE(isDom('C', 'D'));
  EXPECT_TRUE(isDom('D', 'E'));
  EXPECT_TRUE(isDom('E', 'F'));
  EXPECT_TRUE(isDom('F', 'G'));
}
