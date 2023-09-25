#ifndef LEECH_JIT_INCLUDE_COMMON_OPCODES_HH_INCLUDED
#define LEECH_JIT_INCLUDE_COMMON_OPCODES_HH_INCLUDED

#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace ljit
{
enum class Opcodes : std::uint8_t
{
  UNKNOWN,
#define LJIT_MAKE_OPCODE(opc) opc,

#include "opcodes.ii"

#undef LJIT_MAKE_OPCODE
};

class OpcodeConv final
{
private:
  static const auto &getStrToOpcodeMap()
  {
    static std::unordered_map<std::string_view, Opcodes> toOpcodeMap{
#define LJIT_MAKE_OPCODE(opc) {#opc, Opcodes::opc},

#include "opcodes.ii"

#undef LJIT_MAKE_OPCODE
    };
    return toOpcodeMap;
  }

  static const auto &getOpcodeToStrMap()
  {
    static std::unordered_map<Opcodes, std::string_view> toStrMap{
#define LJIT_MAKE_OPCODE(opc) {Opcodes::opc, #opc},

#include "opcodes.ii"

#undef LJIT_MAKE_OPCODE
    };
    return toStrMap;
  }

  template <typename Map, typename Key>
  static std::optional<typename Map::mapped_type> getOptFromMap(const Map &map,
                                                                const Key &key)
  {
    if (auto found = map.find(key); found != map.end())
      return found->second;
    return std::nullopt;
  }

public:
  OpcodeConv() = delete;

  static auto fromName(std::string_view name)
  {
    const auto &toOpcodeMap = getStrToOpcodeMap();
    return getOptFromMap(toOpcodeMap, name);
  }

  static auto toName(Opcodes opcode)
  {
    const auto &toStrMap = getOpcodeToStrMap();
    return getOptFromMap(toStrMap, opcode);
  }
};
} // namespace ljit

#endif /* LEECH_JIT_INCLUDE_COMMON_OPCODES_HH_INCLUDED */
