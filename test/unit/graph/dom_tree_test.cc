#include "graph_test_builder.hh"

#include "graph/dom_tree.hh"

class DomTreeTest : public ljit::testing::GraphTestBuilder
{
protected:
  DomTreeTest() = default;
};

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
   */
   makeEdge(0, 1);

   makeEdge(1, 2);
   makeEdge(1, 5);

   makeEdge(2, 3);

   // Act

}
