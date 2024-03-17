#include "gtest/gtest.h"
#include <algorithm>
#include <cstddef>
#include <gtest/gtest.h>
#include <memory>
#include <numeric>
#include <type_traits>
#include <unordered_map>

#include "../graph/graph_test_builder.hh"
#include "analysis/liveness.hh"
#include "ir/basic_block.hh"
#include "ir/inst.hh"

class LivenessTest : public ljit::testing::GraphTestBuilder
{
protected:
  LivenessTest() = default;
  void buildLivenessAnalyzer()
  {
    livAnalyzer = std::make_unique<std::decay_t<decltype(*livAnalyzer)>>(
      func->makeBBGraph());
  }

  [[nodiscard]] auto checkLiveInterval(ljit::Value *val,
                                       const ljit::LiveInterval &expIn) const
  {
    const auto gotLiveIn = livAnalyzer->getLiveInterval(val);
    if (!gotLiveIn.has_value())
    {
      return testing::AssertionFailure()
             << "No interval found for value" << val;
    }
    const auto &v = gotLiveIn.value();
    if (v != expIn)
    {
      return testing::AssertionFailure()
             << "For value" << val << ". Expected: " << expIn << " Got: " << v;
    }

    return testing::AssertionSuccess();
  }

  [[nodiscard]] auto checkLiveIntervals(
    const std::unordered_map<ljit::Value *, ljit::LiveInterval> &data) const
  {
    for (auto &&[val, exp] : data)
    {
      if (const auto bad = !checkLiveInterval(val, exp))
        return bad;
    }

    return testing::AssertionSuccess();
  }

  std::unique_ptr<ljit::LivenessAnalyzer> livAnalyzer;
};

TEST_F(LivenessTest, lecture)
{
  // Assign
  const auto &vals = buildLivLectureExample();

  std::vector<ljit::Inst *> insns(vals.size());
  std::transform(vals.begin(), vals.end(), insns.begin(),
                 [](ljit::Value *in) { return static_cast<ljit::Inst *>(in); });

  std::vector<std::size_t> liveNums{2, 4, 6, 10, 10, 12, 14, 18, 20, 26};

  // Act
  buildLivenessAnalyzer();

  // Assert
  ASSERT_EQ(liveNums.size(), insns.size());
  for (std::size_t i = 0; i < insns.size(); ++i)
  {
    EXPECT_EQ(insns[i]->getLiveNum(), liveNums[i]);
  }
  // EXPECT_TRUE(checkLiveIntervals({vals[0], {insns}}));
}
