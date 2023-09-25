#include <gtest/gtest.h>

#include <string>

#include "common/opcodes.hh"

using namespace ljit;

TEST(StringToOpcode, string)
{
  const std::string add = "ADD";
  const std::string err = "HAHA";

  const auto opcode = OpcodeConv::fromName(add);
  const auto opcodeErr = OpcodeConv::fromName(err);

  EXPECT_EQ(opcode.value(), Opcodes::ADD);
  EXPECT_THROW(opcodeErr.value(), std::bad_optional_access);
}

TEST(StringToOpcode, stringView)
{
  constexpr std::string_view correct = "ADD";
  constexpr std::string_view err = "HAHA";

  const auto opcode = OpcodeConv::fromName(correct);
  const auto opcodeErr = OpcodeConv::fromName(err);

  EXPECT_EQ(opcode.value(), Opcodes::ADD);
  EXPECT_THROW(opcodeErr.value(), std::bad_optional_access);
}

TEST(StringToOpcode, CString)
{
  constexpr const char *correct = "ADD";
  constexpr const char *err = "HAHA";

  const auto opcode = OpcodeConv::fromName(correct);
  const auto opcodeErr = OpcodeConv::fromName(err);

  EXPECT_EQ(opcode.value(), Opcodes::ADD);
  EXPECT_THROW(opcodeErr.value(), std::bad_optional_access);
}

TEST(OpcodeToString, basic)
{
  constexpr auto correct = Opcodes::ADD;
  constexpr auto err = Opcodes::UNKNOWN;

  const auto name = OpcodeConv::toName(correct);
  const auto nameErr = OpcodeConv::toName(err);

  EXPECT_EQ(name.value(), "ADD");
  EXPECT_THROW(nameErr.value(), std::bad_optional_access);
}
