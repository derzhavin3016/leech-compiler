#include <gtest/gtest.h>

TEST(example, test)
{
  constexpr int first = 30;
  constexpr int sec = 12;

  ASSERT_EQ(first + sec, 42);
}
