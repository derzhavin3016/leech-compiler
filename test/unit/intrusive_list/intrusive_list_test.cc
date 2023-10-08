#include <algorithm>
#include <cstddef>
#include <memory>
#include <tuple>

#include <gtest/gtest.h>

#include "intrusive_list/intrusive_list.hh"
#include "ir/inst.hh"

class ConcreteNode final : public ljit::IntrusiveListNode<ConcreteNode>
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

TEST(INode, insertAfter)
{
  auto node1 = std::make_unique<ConcreteNode>(10);
  auto node2 = std::make_unique<ConcreteNode>(20);

  node2->insertAfter(node1.get());

  EXPECT_EQ(node1->getElem(), 10);
  EXPECT_EQ(node1->getPrev(), nullptr);

  ASSERT_EQ(node1->getNext(), node2.get());

  EXPECT_EQ(node2->getElem(), 20);
  EXPECT_EQ(node2->getNext(), nullptr);
  ASSERT_EQ(node2->getPrev(), node1.get());

  std::ignore = node2.release();
}

TEST(INode, insertBefore)
{
  auto node1 = std::make_unique<ConcreteNode>(10);
  auto node2 = std::make_unique<ConcreteNode>(20);

  node1->insertBefore(node2.get());

  EXPECT_EQ(node1->getElem(), 10);
  EXPECT_EQ(node1->getPrev(), nullptr);
  EXPECT_EQ(node2->getElem(), 20);
  EXPECT_EQ(node2->getNext(), nullptr);

  ASSERT_EQ(node1->getNext(), node2.get());
  ASSERT_EQ(node2->getPrev(), node1.get());

  std::ignore = node2.release();
}

TEST(IListClass, Append)
{
  ljit::IntrusiveList<ConcreteNode> ilist;

  ilist.emplaceBack(std::make_unique<ConcreteNode>(10));
  ilist.emplaceBack(std::make_unique<ConcreteNode>(20));

  EXPECT_EQ(ilist.size(), 2);
  EXPECT_EQ(ilist.getFirst()->getElem(), 10);
  EXPECT_EQ(ilist.getLast()->getElem(), 20);
}
