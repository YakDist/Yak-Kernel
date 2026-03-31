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
} // namespace yak
