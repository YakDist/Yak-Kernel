#pragma once

namespace yak {
enum AllocFlags {
  ALLOC_NOWAIT = 1 << 0, // must not sleep
  ALLOC_ZERO = 1 << 1,   // zero memory
};

enum class PageUse {
  null,
  Reserved,
  Free,
};

constexpr int NUMA_LOCAL = -1;
constexpr int NUMA_ANY = -2;
} // namespace yak
