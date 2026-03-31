#pragma once

#include <yak/arch-mm.h>
#include <yak/types.h>

namespace yak {

enum VMProt : OptionBits {
  PROT_NONE = 0,
  PROT_READ = 1 << 0,
  PROT_WRITE = 1 << 1,
  PROT_EXECUTE = 1 << 2,
  PROT_USER = 1 << 3,
};

using VMCache = arch::VMCache;

enum AllocFlags : OptionBits {
  ALLOC_NOWAIT = 1 << 0, // must not sleep
  ALLOC_ZERO = 1 << 1,   // zero memory
};

constexpr int NUMA_LOCAL = -1;
constexpr int NUMA_ANY = -2;

} // namespace yak
