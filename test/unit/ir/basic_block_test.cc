#include <gtest/gtest.h>

#include "ir/basic_block.hh"

TEST(BasicBlock, print)
{
  ljit::BasicBlock bb{0};
  bb.print(std::cout);
}
