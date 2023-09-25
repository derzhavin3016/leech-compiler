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

TEST(iList, fillAfter)
{
  std::vector<ConcreteNode> storage;
  storage.reserve(10);

  for (std::size_t i = 0; i < storage.capacity(); ++i)
    storage.emplace_back(i * 5);

  for (std::size_t i = 1; i < storage.size(); ++i)
    storage[i].insertAfter(storage[i - 1]);

  const auto *pNode = &storage.front();
  for (std::size_t i = 0; i < storage.size();
       ++i, pNode = pNode->getNext<decltype(*pNode)>())
  {
    ASSERT_NE(pNode, nullptr);
    EXPECT_EQ(storage[i].getElem(), pNode->getElem());
  }
}

TEST(iList, fillBefore)
{
  std::vector<ConcreteNode> storage;
  storage.reserve(10);

  for (std::size_t i = 0; i < storage.capacity(); ++i)
    storage.emplace_back(i * 5);

  for (std::size_t i = 0; i < storage.size() - 1; ++i)
    storage[i].insertBefore(storage[i + 1]);

  const auto *pNode = &storage.front();
  for (std::size_t i = 0; i < storage.size();
       ++i, pNode = pNode->getNext<decltype(*pNode)>())
  {
    ASSERT_NE(pNode, nullptr);
    EXPECT_EQ(storage[i].getElem(), pNode->getElem());
  }
}
