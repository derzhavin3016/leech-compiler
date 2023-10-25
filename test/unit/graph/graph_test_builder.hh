#ifndef LEECH_JIT_TEST_UNIT_GRAPH_GRAPH_TEST_BUILDER_HH_INCLUDED
#define LEECH_JIT_TEST_UNIT_GRAPH_GRAPH_TEST_BUILDER_HH_INCLUDED

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "ir/function.hh"

namespace ljit::testing
{
class GraphTestBuilder : public ::testing::Test
{
protected:
  GraphTestBuilder() = default;

  void genBBs(std::size_t size)
  {
    bbs.clear();
    func = std::make_unique<ljit::Function>();

    bbs.resize(size);
    std::generate(bbs.begin(), bbs.end(), [this] { return func->appendBB(); });
  }

  [[nodiscard]] auto toConstBBs() const
  {
    std::vector<const ljit::BasicBlock *> cBBs(bbs.size());
    std::copy(bbs.begin(), bbs.end(), cBBs.begin());
    return cBBs;
  }

  void makeEdge(std::size_t idPred, std::size_t idSucc)
  {
    bbs.at(idPred)->linkSucc(bbs.at(idSucc));
  }

  void buildExample1();
  void buildExample2();

  std::unique_ptr<ljit::Function> func{};
  std::vector<ljit::BasicBlock *> bbs{};
};

/* Example 1
 *
 *             +---+
 *             | 0 |
 *             +---+
 *               |
 *               |
 *               v
 *   +---+     +---+
 *   | 2 | <-- | 1 |
 *   +---+     +---+
 *     |         |
 *     |         |
 *     |         v
 *     |       +---+     +---+
 *     |       | 5 | --> | 6 |
 *     |       +---+     +---+
 *     |         |         |
 *     |         |         |
 *     |         v         |
 *     |       +---+       |
 *     |       | 4 |       |
 *     |       +---+       |
 *     |         |         |
 *     |         |         |
 *     |         v         |
 *     |       +---+       |
 *     +-----> | 3 | <-----+
 *             +---+
 **/
inline void GraphTestBuilder::buildExample1()
{
  genBBs(7);
  makeEdge(0, 1);

  makeEdge(1, 2);
  makeEdge(1, 5);

  makeEdge(2, 3);

  makeEdge(5, 4);
  makeEdge(5, 6);

  makeEdge(4, 3);

  makeEdge(6, 3);
}

/*
 *
 *            +----+
 *            | 0  |
 *            +----+
 *              |
 *              |
 *              v
 *            +----+
 *    +-----> | 1  | -+
 *    |       +----+  |
 *    |         |     |
 *    |         |     |
 *    |         v     |
 *    |       +----+  |
 *    |       | 9  |  |
 *    |       +----+  |
 *    |         |     |
 *    |         |     |
 *    |         v     |
 *    |       +----+  |
 *    |    +> | 2  | <+
 *    |    |  +----+
 *    |    |    |
 *    |    |    |
 *    |    |    v
 *    |    |  +----+
 *    |    +- | 3  |
 *    |       +----+
 *    |         |
 *    |         |
 *    |         v
 *    |       +----+
 *    |       | 4  | <+
 *    |       +----+  |
 *    |         |     |
 *    |         |     |
 *    |         v     |
 *    |       +----+  |
 *    |       | 5  | -+
 *    |       +----+
 *    |         |
 *    |         |
 *    |         v
 *  +---+     +----+
 *  | 7 | <-- | 6  |
 *  +---+     +----+
 *              |
 *              |
 *              v
 *            +----+
 *            | 8  |
 *            +----+
 *              |
 *              |
 *              v
 *            +----+
 *            | 10 |
 *            +----+
 */
inline void GraphTestBuilder::buildExample2()
{
  genBBs(11);
  makeEdge(0, 1);

  makeEdge(1, 9);
  makeEdge(1, 2);

  makeEdge(2, 3);

  makeEdge(3, 2);
  makeEdge(3, 4);

  makeEdge(4, 5);

  makeEdge(5, 4);
  makeEdge(5, 6);

  makeEdge(6, 7);
  makeEdge(6, 8);

  makeEdge(7, 1);

  makeEdge(8, 10);

  makeEdge(9, 2);
}
} // namespace ljit::testing

#endif /* LEECH_JIT_TEST_UNIT_GRAPH_GRAPH_TEST_BUILDER_HH_INCLUDED */
