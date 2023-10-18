#include <algorithm>
#include <cstddef>
#include <gtest/gtest-death-test.h>
#include <initializer_list>
#include <vector>

#include <gtest/gtest.h>

#include "graph/dfs.hh"
#include "ir/basic_block.hh"
#include "ir/function.hh"

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

class DFSTest : public ::testing::Test
{
protected:
  DFSTest() = default;

  void GenBBs(std::size_t size)
  {
    bbs.resize(size);
    std::generate(bbs.begin(), bbs.end(), [this] { return func.appendBB(); });
  }

  [[nodiscard]] auto toConstBBs() const
  {
    std::vector<const ljit::BasicBlock *> cBBs(bbs.size());
    std::copy(bbs.begin(), bbs.end(), cBBs.begin());
    return cBBs;
  }

  [[nodiscard]] static auto makeRPOAnswer(
    std::initializer_list<std::size_t> answ)
  {
    return std::vector<std::size_t>{answ};
  }

  void makeEdge(std::size_t idPred, std::size_t idSucc)
  {
    bbs.at(idPred)->linkSucc(bbs.at(idSucc));
  }

  [[nodiscard]] auto getRPOIdx() const
  {
    std::vector<std::size_t> res;
    res.reserve(bbs.size());
    ljit::graph::depthFirstSearch(
      func.makeBBGraph(), [&](auto pNode) { res.push_back(pNode->getId()); });
    return res;
  }

  ljit::Function func{};
  std::vector<ljit::BasicBlock *> bbs{};
};

TEST_F(DFSTest, simple)
{
  // Assign
  constexpr std::size_t kSize = 5;
  GenBBs(kSize);
  // Just a linear bb structure:
  // bb0 -> bb1 -> bb2 -> bb3 -> bb4
  for (std::size_t i = 0; i < bbs.size() - 1; ++i)
    makeEdge(i, i + 1);

  auto answer = toConstBBs();

  // Act
  auto vis = ljit::graph::depthFirstSearch(func.makeBBGraph());

  // Assert
  EXPECT_EQ(answer, vis);
}

TEST_F(DFSTest, tree)
{
  constexpr std::size_t kSize = 6;
  GenBBs(kSize);

  // Here we just build a simple tree
  /*            bb0
              /     \
            bb1    bb2
           /  \     |
         bb3  bb4  bb5
  So the RPO will be 0 1 3 4 2 5
  */
  const auto answer = makeRPOAnswer({0, 1, 3, 4, 2, 5});

  makeEdge(0, 1);
  makeEdge(0, 2);

  makeEdge(1, 3);
  makeEdge(1, 4);

  makeEdge(2, 5);

  // Act
  const auto res = getRPOIdx();

  // Assert
  EXPECT_EQ(answer, res);
}

TEST_F(DFSTest, cycle)
{
  GenBBs(6);

  // Here we just build a simple tree
  // BUT WITH A LOOP EDGE!!!
  /*            bb0
              /     \
            bb1  --> bb2
           /  \ /    |
         bb3  bb4  bb5
  But the RPO will be still 0 1 3 4 2 5
  */
  const auto answer = makeRPOAnswer({0, 1, 3, 4, 2, 5});

  makeEdge(0, 1);
  makeEdge(0, 2);

  makeEdge(1, 3);
  makeEdge(1, 4);

  makeEdge(2, 5);

  // Loop edge
  makeEdge(4, 2);

  const auto res = getRPOIdx();

  // Assert
  EXPECT_EQ(answer, res);
}
