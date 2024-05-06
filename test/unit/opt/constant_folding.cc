#include <gtest/gtest.h>

#include "opt/constant_folding.hh"

#include "../graph/graph_test_builder.hh"

class ConstFoldTest : public ljit::testing::GraphTestBuilder
{

};

TEST(ConstantFolding, basic)
{
  EXPECT_TRUE(true);
}
