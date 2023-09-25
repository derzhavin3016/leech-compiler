#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "intrusive_list/intrusive_list.hh"

class ConcreteNode final : public ljit::IntrusiveListNode
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

using ConcreteNodePtr = std::unique_ptr<ConcreteNode>;

class IList : public ::testing::Test
{
public:
  IList() : storage(10)
  {
    std::generate(storage.begin(), storage.end(),
                  [idx = std::size_t{}]() mutable {
                    return std::make_unique<ConcreteNode>(idx++ * 5);
                  });
  }

protected:
  std::vector<ConcreteNodePtr> storage;
};

TEST_F(IList, fillAfter)
{
  for (std::size_t i = 1; i < storage.size(); ++i)
    storage[i]->insertAfter(*storage[i - 1]);

  const auto *pNode = storage.front().get();
  for (std::size_t i = 0; i < storage.size();
       ++i, pNode = pNode->getNext<decltype(*pNode)>())
  {
    ASSERT_NE(pNode, nullptr);
    EXPECT_EQ(storage[i]->getElem(), pNode->getElem());
  }
}

TEST_F(IList, fillBefore)
{
  for (std::size_t i = 0; i < storage.size() - 1; ++i)
    storage[i]->insertBefore(*storage[i + 1]);

  const auto *pNode = storage.front().get();
  for (std::size_t i = 0; i < storage.size();
       ++i, pNode = pNode->getNext<decltype(*pNode)>())
  {
    ASSERT_NE(pNode, nullptr);
    EXPECT_EQ(storage[i]->getElem(), pNode->getElem());
  }
}
