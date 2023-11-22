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
  void buildExample3();
  void buildExample4();
  void buildExample5();
  void buildExample6();

  std::unique_ptr<ljit::Function> func{};
  std::vector<ljit::BasicBlock *> bbs{};
};

/* Example 1
 *
 *            +---+
 *            | 0 |
 *            +---+
 *              |
 *              |
 *              v
 *  +---+     +---+
 *  | 2 | <-- | 1 |
 *  +---+     +---+
 *    |         |
 *    |         |
 *    |         v
 *    |       +---+     +---+
 *    |       | 5 | --> | 6 |
 *    |       +---+     +---+
 *    |         |         |
 *    |         |         |
 *    |         v         |
 *    |       +---+       |
 *    |       | 4 |       |
 *    |       +---+       |
 *    |         |         |
 *    |         |         |
 *    |         v         |
 *    |       +---+       |
 *    +-----> | 3 | <-----+
 *            +---+
 */
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

/* Example 2
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

/* Example 3
 *
 *       +------------------------+
 *       |                        |
 *       |                 +---+  |
 *       |                 | 0 |  |
 *       |                 +---+  |
 *       |                   |    |
 *       |                   |    |
 *       |                   v    |
 *     +---+     +---+     +---+  |
 *     | 5 | <-- | 4 | <-- | 1 | <+
 *     +---+     +---+     +---+
 *       |         |         |
 *       |         |         |
 *       v         |         v
 *     +---+       |       +---+
 *  +- | 7 |       |       | 2 | <+
 *  |  +---+       |       +---+  |
 *  |    |         |         |    |
 *  |    |         |         |    |
 *  |    |         |         v    |
 *  |    |         |       +---+  |
 *  |    |         +-----> | 3 |  |
 *  |    |                 +---+  |
 *  |    |                   |    |
 *  |    |                   |    |
 *  |    |                   v    |
 *  |    |                 +---+  |
 *  |    +---------------> | 6 | -+
 *  |                      +---+
 *  |                        |
 *  |                        |
 *  |                        v
 *  |                      +---+
 *  |                      | 8 |
 *  |                      +---+
 *  |                        ^
 *  +------------------------+
 */
inline void GraphTestBuilder::buildExample3()
{
  auto &&toId = [](char letter) {
    return static_cast<std::size_t>(letter) - 'A';
  };
  auto &&edge = [&, this](char pred, char succ) {
    makeEdge(toId(pred), toId(succ));
  };
  genBBs(9);

  edge('A', 'B');

  edge('B', 'C');
  edge('B', 'E');

  edge('C', 'D');

  edge('D', 'G');

  edge('E', 'D');
  edge('E', 'F');

  edge('F', 'B');
  edge('F', 'H');

  edge('G', 'C');
  edge('G', 'I');

  edge('H', 'G');
  edge('H', 'I');
}

/* Example 4
 *
 *            +---+
 *            | 0 |
 *            +---+
 *              |
 *              |
 *              v
 *  +---+     +---+
 *  | 2 | <-- | 1 | <+
 *  +---+     +---+  |
 *              |    |
 *              |    |
 *              v    |
 *            +---+  |
 *            | 3 |  |
 *            +---+  |
 *              |    |
 *              |    |
 *              v    |
 *            +---+  |
 *            | 4 | -+
 *            +---+
 */
inline void GraphTestBuilder::buildExample4()
{
  genBBs(5);
  makeEdge(0, 1);
  makeEdge(1, 2);
  makeEdge(1, 3);
  makeEdge(3, 4);
  makeEdge(4, 1);
}

/* Example 5
 *
 *            +---+
 *            | 0 |
 *            +---+
 *              |
 *              |
 *              v
 *            +---+
 *            | 1 | <+
 *            +---+  |
 *              |    |
 *              |    |
 *              v    |
 *  +---+     +---+  |
 *  | 3 | <-- | 2 |  |
 *  +---+     +---+  |
 *    ^         |    |
 *    |         |    |
 *    |         v    |
 *    |       +---+  |
 *    +------ | 4 |  |
 *            +---+  |
 *              |    |
 *              |    |
 *              v    |
 *            +---+  |
 *            | 5 | -+
 *            +---+
 */
inline void GraphTestBuilder::buildExample5()
{
  genBBs(6);
  makeEdge(0, 1);
  makeEdge(1, 2);
  makeEdge(2, 3);
  makeEdge(2, 4);
  makeEdge(4, 3);
  makeEdge(4, 5);
  makeEdge(5, 1);
}

/* Example 6
 *
 *                 +---+
 *              +> | 0 |
 *              |  +---+
 *              |    |
 *              |    |
 *              |    v
 *  +---+       |  +---+
 *  | 3 | <-----+- | 1 | <+
 *  +---+       |  +---+  |
 *    |         |    |    |
 *    |         |    |    |
 *    |         |    v    |
 *    |         |  +---+  |       +---+
 *    |         |  | 2 | -+-----> | 4 |
 *    |         |  +---+  |       +---+
 *    |         |    |    |
 *    |         |    |    |
 *    |         |    v    |
 *    |         |  +---+  |
 *    +---------+> | 5 |  |
 *              |  +---+  |
 *              |    |    |
 *              |    |    |
 *              |    v    |
 *              |  +---+  |
 *              |  | 6 | -+
 *              |  +---+
 *              |    |
 *              |    |
 *              |    v
 *              |  +---+
 *              +- | 7 |
 *                 +---+
 */
inline void GraphTestBuilder::buildExample6()
{
  genBBs(8);
  makeEdge(0, 1);
  makeEdge(1, 2);
  makeEdge(1, 3);
  makeEdge(2, 4);
  makeEdge(2, 5);
  makeEdge(3, 5);
  makeEdge(5, 6);
  makeEdge(6, 7);
  makeEdge(6, 1);
  makeEdge(7, 0);
}
} // namespace ljit::testing

#endif /* LEECH_JIT_TEST_UNIT_GRAPH_GRAPH_TEST_BUILDER_HH_INCLUDED */
