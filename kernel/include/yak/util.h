#pragma once

#include <optional>
#include <yak/panic.h>

namespace yak {
template <typename T> T expect(std::optional<T> opt, const char *msg) {
  if (!opt) [[unlikely]] {
    panic(msg);
  }
  return *opt;
}

#define ENUM_BIT_OPS(name)                                \
  inline constexpr name operator|(name a, name b) {       \
    return static_cast<name>(static_cast<OptionBits>(a) | \
                             static_cast<OptionBits>(b)); \
  }                                                       \
  inline constexpr name operator&(name a, name b) {       \
    return static_cast<name>(static_cast<OptionBits>(a) & \
                             static_cast<OptionBits>(b)); \
  }

} // namespace yak
