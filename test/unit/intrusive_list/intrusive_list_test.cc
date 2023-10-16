#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "common/common.hh"
#include "intrusive_list/intrusive_list.hh"

TEST(IList, setGet)
{
  // Assign
  ljit::IListNode node;
  ljit::IListNode node1;
  ljit::IListNode node2;

  // Act
  node1.setPrev(&node);
  node1.setNext(&node2);

  // Assert
  EXPECT_EQ(node1.getNext(), &node2);
  EXPECT_EQ(node1.getPrev(), &node);
}

class ConcreteNode final : public ljit::IListNode
{
  int m_elem{};

public:
  ConcreteNode() = default;
  explicit ConcreteNode(int elem) : m_elem{elem}
  {}

  [[nodiscard]] auto getElem() const noexcept
  {
    return m_elem;
  }
};

struct IListConcrete final : public ljit::IListBase
{
  using ljit::IListBase::insertAfter;
  using ljit::IListBase::insertBefore;
  using ljit::IListBase::moveBefore;
  using ljit::IListBase::remove;
};

template <std::size_t Size>
class IListBaseTest : public ::testing::Test
{
protected:
  IListBaseTest()
    : values(Size),
      val1(values.front()),
      val2(values[1]),
      nodes(Size),
      node1(nodes.front()),
      node2(nodes[1])
  {
    static constexpr int kInitVal = 10;
    std::generate(values.begin(), values.end(),
                  [i = int{1}]() mutable { return i++ * kInitVal; });

    std::transform(values.begin(), values.end(), nodes.begin(), [](auto val) {
      return std::make_unique<ConcreteNode>(val);
    });
  }
  void linkAll()
  {
    link(0, nodes.size());
  }
  void link(std::size_t start, std::size_t end)
  {
    LJIT_ASSERT(start < nodes.size());
    LJIT_ASSERT(end <= nodes.size());
    LJIT_ASSERT(end > 1ULL);

    for (; start < end - 1; ++start)
    {
      IListConcrete::insertAfter(nodes[start].get(), nodes[start + 1].get());
    }
  }
  std::vector<int> values{};
  const int &val1;
  const int &val2;
  std::vector<std::unique_ptr<ConcreteNode>> nodes{};

  std::unique_ptr<ConcreteNode> &node1;
  std::unique_ptr<ConcreteNode> &node2;
};

using IListBasePairTest = IListBaseTest<2>;

TEST_F(IListBasePairTest, insertAfter)
{
  // Act
  IListConcrete::insertAfter(node1.get(), node2.get());

  // Assert
  ASSERT_EQ(node1->getElem(), val1); // Something really bad happened
  ASSERT_EQ(node2->getElem(), val2); // And here

  EXPECT_EQ(node1->getNext(), node2.get());
  EXPECT_EQ(node1->getPrev(), nullptr);

  EXPECT_EQ(node2->getNext(), nullptr);
  EXPECT_EQ(node2->getPrev(), node1.get());
}

TEST_F(IListBasePairTest, insertBefore)
{
  // Act
  IListConcrete::insertBefore(node2.get(), node1.get());

  // Assert
  ASSERT_EQ(node1->getElem(), val1); // Something really bad happened
  ASSERT_EQ(node2->getElem(), val2); // And here

  EXPECT_EQ(node1->getNext(), node2.get());
  EXPECT_EQ(node1->getPrev(), nullptr);

  EXPECT_EQ(node2->getNext(), nullptr);
  EXPECT_EQ(node2->getPrev(), node1.get());
}

using IListBaseTripleTest = IListBaseTest<3>;

TEST_F(IListBaseTripleTest, remove)
{
  // Assign
  link(0, 2);
  auto &node3 = nodes.back();
  IListConcrete::insertAfter(node2.get(), node3.get());

  // Act
  IListConcrete::remove(node2.get());

  // Assert
  ASSERT_EQ(node1->getElem(), val1);      // Something really bad happened
  ASSERT_EQ(node2->getElem(), val2);      // And here
  ASSERT_EQ(node3->getElem(), values[2]); // And here

  EXPECT_EQ(node1->getNext(), node3.get());
  EXPECT_EQ(node1->getPrev(), nullptr);

  EXPECT_EQ(node2->getNext(), nullptr);
  EXPECT_EQ(node2->getPrev(), nullptr);

  EXPECT_EQ(node3->getNext(), nullptr);
  EXPECT_EQ(node3->getPrev(), node1.get());
}

TEST_F(IListBasePairTest, moveBeforeFirst)
{
  // Assign
  linkAll();

  // Act & Assert
  EXPECT_DEATH(IListConcrete::moveBefore(node1.get(), node1.get(), node2.get()),
               "Trying to insert last before first");
}

TEST_F(IListBasePairTest, moveBeforeLast)
{
  // Assign
  linkAll();

  // Act
  IListConcrete::moveBefore(node2.get(), node1.get(), node2.get());
  // Assert
  ASSERT_EQ(node1->getElem(), val1); // Something really bad happened
  ASSERT_EQ(node2->getElem(), val2); // And here

  EXPECT_EQ(node1->getNext(), node2.get());
  EXPECT_EQ(node1->getPrev(), nullptr);

  EXPECT_EQ(node2->getNext(), nullptr);
  EXPECT_EQ(node2->getPrev(), node1.get());
}

TEST_F(IListBasePairTest, moveBeforeEmpty)
{
  // Assign
  linkAll();

  // Act
  IListConcrete::moveBefore(node2.get(), node1.get(), node1.get());
  // Assert
  ASSERT_EQ(node1->getElem(), val1); // Something really bad happened
  ASSERT_EQ(node2->getElem(), val2); // And here

  EXPECT_EQ(node1->getNext(), node2.get());
  EXPECT_EQ(node1->getPrev(), nullptr);

  EXPECT_EQ(node2->getNext(), nullptr);
  EXPECT_EQ(node2->getPrev(), node1.get());
}

using IListBase6Test = IListBaseTest<6>;

TEST_F(IListBase6Test, moveBefore)
{
  // Assign
  link(0, 2);
  link(2, 6);

  auto &node3 = nodes[2];
  auto &node4 = nodes[3];
  auto &node5 = nodes[4];
  auto &node6 = nodes[5];

  // Act
  /**
   * Here we have two lists
   * 10 -> 20
   * 30 -> 40 -> 50 -> 60
   * We need to transform they to
   * 10 -> 40 -> 50 -> 20
   * 30 -> 60
   */

  IListConcrete::moveBefore(node2.get(), node4.get(), node6.get());

  // Assert
  ASSERT_EQ(node1->getElem(), val1);      // Something really bad happened
  ASSERT_EQ(node2->getElem(), val2);      // And here
  ASSERT_EQ(node3->getElem(), values[2]); // And here
  ASSERT_EQ(node4->getElem(), values[3]); // And here
  ASSERT_EQ(node5->getElem(), values[4]); // And here
  ASSERT_EQ(node6->getElem(), values[5]); // And here

  EXPECT_EQ(node1->getNext(), node4.get());
  EXPECT_EQ(node1->getPrev(), nullptr);

  EXPECT_EQ(node2->getNext(), nullptr);
  EXPECT_EQ(node2->getPrev(), node5.get());

  EXPECT_EQ(node3->getNext(), node6.get());
  EXPECT_EQ(node3->getPrev(), nullptr);

  EXPECT_EQ(node4->getNext(), node5.get());
  EXPECT_EQ(node4->getPrev(), node1.get());

  EXPECT_EQ(node5->getNext(), node2.get());
  EXPECT_EQ(node5->getPrev(), node4.get());

  EXPECT_EQ(node6->getNext(), nullptr);
  EXPECT_EQ(node6->getPrev(), node3.get());
}

template <std::size_t Size>
class IListTest : public IListBaseTest<Size>
{};

using IListPairTest = IListTest<2>;

TEST(IListTest, empty)
{
  // Assign
  ljit::IList<ConcreteNode> ilist;

  // Act & Assert
  EXPECT_TRUE(ilist.empty());
  EXPECT_EQ(ilist.begin(), ilist.end());
  EXPECT_EQ(ilist.size(), 0);
}

TEST_F(IListPairTest, stateCheck)
{
  // Assign
  ljit::IList<ConcreteNode> list;

  // Act
  list.push_back(node1.release());
  list.push_back(node2.release());

  // Assert
  EXPECT_EQ(list.front().getElem(), val1);
  EXPECT_EQ(list.back().getElem(), val2);
}
