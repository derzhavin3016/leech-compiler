#include <gtest/gtest.h>

#include <memory>

#include "../graph/graph_test_builder.hh"
#include "analysis/regalloc.hh"

TEST(RegisterPool, basic)
{
  ljit::RegisterPool<3> pool;

  const auto reg = pool.allocateReg();

  ASSERT_TRUE(reg.has_value());
  EXPECT_EQ(reg.value(), 0);
  EXPECT_EQ(pool.getUseCount(), 1);
}

TEST(RegisterPool, basicDealloc)
{
  ljit::RegisterPool<3> pool;

  const auto reg = pool.allocateReg();
  const auto reg2 = pool.allocateReg();

  ASSERT_TRUE(reg.has_value());
  ASSERT_TRUE(reg2.has_value());

  pool.deallocateReg(reg.value());

  EXPECT_EQ(reg.value(), 0);
  EXPECT_EQ(reg2.value(), 1);
  EXPECT_EQ(pool.getUseCount(), 1);
}

TEST(RegisterPool, full)
{
  ljit::RegisterPool<3> pool;

  const auto reg = pool.allocateReg();
  const auto reg2 = pool.allocateReg();
  const auto reg3 = pool.allocateReg();
  const auto reg4 = pool.allocateReg();

  ASSERT_TRUE(reg.has_value());
  ASSERT_TRUE(reg2.has_value());
  ASSERT_TRUE(reg3.has_value());
  ASSERT_FALSE(reg4.has_value());

  EXPECT_EQ(reg.value(), 0);
  EXPECT_EQ(reg2.value(), 1);
  EXPECT_EQ(reg3.value(), 2);
  EXPECT_EQ(pool.getUseCount(), 3);
}

namespace
{
class RegAllocTest : public ljit::testing::GraphTestBuilder
{
protected:
  RegAllocTest() = default;
  void buildRegAllocator()
  {
    regAlloc =
      std::make_unique<std::decay_t<decltype(*regAlloc)>>(func->makeBBGraph());
  }

  [[nodiscard]] auto checkLocation(ljit::Value *val,
                                   ljit::RegAllocator::Location loc) const
  {
    const auto gotLoc = regAlloc->getLocation(val);
    if (!gotLoc.has_value())
    {
      return testing::AssertionFailure()
             << "No location found for value " << val;
    }
    const auto &v = gotLoc.value();
    if (v.stack != loc.stack || v.locId != loc.locId)
    {
      return testing::AssertionFailure()
             << "For value " << val << ". Expected: " << loc.locId << ' '
             << (loc.stack ? "stack" : "register") << " Got: " << v.locId << ' '
             << (v.stack ? "stack" : "register");
    }

    return testing::AssertionSuccess();
  }

  std::unique_ptr<ljit::RegAllocator> regAlloc;
};
} // namespace
TEST_F(RegAllocTest, lecture)
{
  auto insns = buildLivLectureExample();

  buildRegAllocator();

  EXPECT_TRUE(checkLocation(insns[0], {0, false}));
  EXPECT_TRUE(checkLocation(insns[1], {4, false}));
  EXPECT_TRUE(checkLocation(insns[2], {6, false}));
  EXPECT_TRUE(checkLocation(insns[3], {10, false}));
  EXPECT_TRUE(checkLocation(insns[4], {10, false}));
  EXPECT_TRUE(checkLocation(insns[5], {12, false}));
  EXPECT_TRUE(checkLocation(insns[7], {18, false}));
  EXPECT_TRUE(checkLocation(insns[8], {20, false}));
  EXPECT_TRUE(checkLocation(insns[9], {26, false}));
}
