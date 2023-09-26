#ifndef LEECH_JIT_INCLUDE_COMMON_OPCODES_HH_INCLUDED
#define LEECH_JIT_INCLUDE_COMMON_OPCODES_HH_INCLUDED

#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>

#include <magic_enum.hpp>

namespace ljit
{
enum class Opcodes : std::uint8_t
{
  UNKNOWN,
#define LJIT_MAKE_OPCODE(opc) opc,

#include "opcodes.ii"

#undef LJIT_MAKE_OPCODE
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_COMMON_OPCODES_HH_INCLUDED */
