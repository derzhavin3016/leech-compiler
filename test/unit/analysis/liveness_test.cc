#include <gtest/gtest.h>
#include <memory>
#include <type_traits>

#include "../graph/graph_test_builder.hh"
#include "analysis/liveness.hh"

class LivenessTest : public ljit::testing::GraphTestBuilder
{
protected:
  LivenessTest() = default;
  void buildLivenessAnalyzer()
  {
    livAnalyzer = std::make_unique<std::decay_t<decltype(*livAnalyzer)>>(
      func->makeBBGraph());
  }

  std::unique_ptr<ljit::LivenessAnalyzer> livAnalyzer;
};


TEST_F(LivenessTest, lecture) {
  // Assign
  buildLivLectureExample();

  // Act
  buildLivenessAnalyzer();
}
