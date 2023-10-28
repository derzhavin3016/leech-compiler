#include "graph_test_builder.hh"

#include <cstddef>
#include <gtest/gtest.h>
#include <initializer_list>
#include <sstream>
#include <string_view>
#include <vector>

#include "graph/dfs.hh"
#include "ir/basic_block.hh"

TEST(DFS, empty)
{
  // Assign
  auto func = ljit::Function{};

  // Act & Assert
  EXPECT_DEATH(
    static_cast<void>(ljit::graph::depthFirstSearch(func.makeBBGraph())), "");
}

TEST(DFS, emptyGraph)
{
  // Assign
  const auto graph = ljit::BasicBlockGraph{nullptr};

  // Act
  const auto res = ljit::graph::depthFirstSearch(graph);

  // Assert
  EXPECT_TRUE(res.empty());
}

class DFSTest : public ljit::testing::GraphTestBuilder
{
protected:
  DFSTest() = default;

  [[nodiscard]] static auto makePreOrderAnswer(
    std::initializer_list<std::size_t> answ)
  {
    return std::vector<std::size_t>{answ};
  }

  [[nodiscard]] auto getPreOrderIdx() const
  {
    std::vector<std::size_t> res;
    res.reserve(bbs.size());
    ljit::graph::depthFirstSearch(
      func->makeBBGraph(), [&](auto pNode) { res.push_back(pNode->getId()); });
    return res;
  }
};

TEST_F(DFSTest, simple)
{
  // Assign
  constexpr std::size_t kSize = 5;
  genBBs(kSize);
  // Just a linear bb structure:
  // bb0 -> bb1 -> bb2 -> bb3 -> bb4
  for (std::size_t i = 0; i < bbs.size() - 1; ++i)
    makeEdge(i, i + 1);

  auto answer = toConstBBs();

  // Act
  auto vis = ljit::graph::depthFirstSearch(func->makeBBGraph());

  // Assert
  EXPECT_EQ(answer, vis);
}

TEST_F(DFSTest, tree)
{
  constexpr std::size_t kSize = 6;
  genBBs(kSize);

  // Here we just build a simple tree
  /*            bb0
              /     \
            bb1    bb2
           /  \     |
         bb3  bb4  bb5
  So the PreOrder will be 0 1 3 4 2 5
  */
  const auto answer = makePreOrderAnswer({0, 1, 3, 4, 2, 5});

  makeEdge(0, 1);
  makeEdge(0, 2);

  makeEdge(1, 3);
  makeEdge(1, 4);

  makeEdge(2, 5);

  // Act
  const auto res = getPreOrderIdx();

  // Assert
  EXPECT_EQ(answer, res);
}

TEST_F(DFSTest, biggerTree)
{
  constexpr std::size_t kSize = 10;
  genBBs(kSize);

  // Here we just build a tree
  /*            bb0
              /     \
            bb1    bb2
           /  \     |  \
         bb3  bb4  bb5  bb6
        /  \    |
      bb7  bb8  bb9
  So the PreOrder will be 0 1 3 7 8 4 9 2 5 6
  */
  const auto answer = makePreOrderAnswer({0, 1, 3, 7, 8, 4, 9, 2, 5, 6});

  makeEdge(0, 1);
  makeEdge(0, 2);

  makeEdge(1, 3);
  makeEdge(1, 4);

  makeEdge(3, 7);
  makeEdge(3, 8);

  makeEdge(4, 9);

  makeEdge(2, 5);
  makeEdge(2, 6);

  // Act
  const auto res = getPreOrderIdx();

  // Assert
  EXPECT_EQ(answer, res);
}

TEST_F(DFSTest, biggerNonTree)
{
  constexpr std::size_t kSize = 10;
  genBBs(kSize);

  // Here we just build a tree
  /*            bb0
              /  |   \
            bb1  |  bb2
           /  \ /   |  \
      --bb3  bb4  bb5  bb6
      |/  \    |  |
      bb7  bb8  bb9
  So the PreOrder will be 0 1 3 7 8 4 9 2 5 6
  */
  const auto answer = makePreOrderAnswer({0, 1, 3, 7, 8, 4, 9, 2, 5, 6});

  makeEdge(0, 1);
  makeEdge(0, 2);
  makeEdge(0, 4);

  makeEdge(1, 3);
  makeEdge(1, 4);

  makeEdge(3, 7);
  makeEdge(3, 7);
  makeEdge(3, 8);

  makeEdge(4, 9);
  makeEdge(5, 9);

  makeEdge(2, 5);
  makeEdge(2, 6);

  // Act
  const auto res = getPreOrderIdx();

  // Assert
  EXPECT_EQ(answer, res);
}

TEST_F(DFSTest, biggerNonTreeDot)
{
  constexpr std::size_t kSize = 10;
  genBBs(kSize);

  // Here we just build a tree
  /*            bb0
              /  |   \
            bb1  |  bb2
           /  \ /   |  \
      --bb3  bb4  bb5  bb6
      |/  \    |  |
      bb7  bb8  bb9
  So the PreOrder will be 0 1 3 7 8 4 9 2 5 6
  */
  makeEdge(0, 1);
  makeEdge(0, 2);
  makeEdge(0, 4);

  makeEdge(1, 3);
  makeEdge(1, 4);

  makeEdge(3, 7);
  makeEdge(3, 7);
  makeEdge(3, 8);

  makeEdge(4, 9);
  makeEdge(5, 9);

  makeEdge(2, 5);
  makeEdge(2, 6);

  constexpr std::string_view kAnswer =
    R"(digraph BBGraph{
bb0 [label="0"];
bb1 [label="1"];
bb0 -> bb1;
bb3 [label="3"];
bb1 -> bb3;
bb7 [label="7"];
bb3 -> bb7;
bb3 -> bb7;
bb8 [label="8"];
bb3 -> bb8;
bb4 [label="4"];
bb0 -> bb4;
bb1 -> bb4;
bb9 [label="9"];
bb4 -> bb9;
bb5 -> bb9;
bb2 [label="2"];
bb0 -> bb2;
bb5 [label="5"];
bb2 -> bb5;
bb6 [label="6"];
bb2 -> bb6;
})";

  // Act
  std::ostringstream ss;
  func->makeBBGraph().dumpDot(ss);

  // Assert
  EXPECT_EQ(kAnswer, ss.str());
}

TEST_F(DFSTest, cycle)
{
  genBBs(6);

  // Here we just build a simple tree
  // BUT WITH A LOOP EDGE!!!
  /*            bb0
              /     \
            bb1  --> bb2
           /  \ /    |
         bb3  bb4  <- bb5
  But the PreOrder will be still 0 1 3 4 2 5
  */
  const auto answer = makePreOrderAnswer({0, 1, 3, 4, 2, 5});

  makeEdge(0, 1);
  makeEdge(0, 2);

  makeEdge(1, 3);
  makeEdge(1, 4);

  makeEdge(2, 5);

  // Loop edges
  makeEdge(4, 2);
  makeEdge(5, 4);

  const auto res = getPreOrderIdx();

  // Assert
  EXPECT_EQ(answer, res);
}

TEST_F(DFSTest, example1)
{
  // Assign
  buildExample1();
  const auto answer = makePreOrderAnswer({0, 1, 2, 3, 5, 4, 6});
  // Act
  const auto res = getPreOrderIdx();

  // Assert
  EXPECT_EQ(answer, res);
}

TEST_F(DFSTest, example2)
{
  // Assign
  buildExample2();
  const auto answer = makePreOrderAnswer({0, 1, 9, 2, 3, 4, 5, 6, 7, 8, 10});
  // Act
  const auto res = getPreOrderIdx();

  // Assert
  EXPECT_EQ(answer, res);
}

TEST_F(DFSTest, example3)
{
  // Assign
  buildExample3();
  const auto answer = makePreOrderAnswer({0, 1, 2, 3, 6, 8, 4, 5, 7});
  // Act
  const auto res = getPreOrderIdx();

  // Assert
  EXPECT_EQ(answer, res);
}
