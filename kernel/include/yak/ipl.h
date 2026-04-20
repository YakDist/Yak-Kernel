#pragma once

#include <compare>
#include <utility>
#include <yak/arch-ipl.h>

namespace yak {
enum class Ipl : unsigned int {
  passive = 0,
  apc = 1,
  dispatch = 2,
  device = arch::ipl_device_min,
  clock = arch::ipl_device_max + 1,
  high = arch::ipl_device_max + 2,
};

constexpr auto operator<=>(Ipl a, Ipl b) {
  return std::to_underlying(a) <=> std::to_underlying(b);
}

constexpr bool operator==(Ipl a, Ipl b) {
  return std::to_underlying(a) == std::to_underlying(b);
}

Ipl iplget();
Ipl iplr(Ipl ipl);
void iplx(Ipl ipl);

} // namespace yak
