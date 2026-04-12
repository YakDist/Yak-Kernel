#pragma once

#include <cstddef>
#include <yak/types.h>
#include <yak/vm/address.h>

namespace yak::arch {

extern vaddr_t HHDM_BASE; // changes depending on PML4/5
extern size_t PMAP_LEVELS;

constexpr vaddr_t PFNDB_BASE = 0xffffc00000000000; // -64TiB
constexpr size_t PFNDB_SIZE =
    4ULL * 1024 * 1024 * 1024 * 1024; // 4TiB ought to be enough for anybody:
                                      // 4TiB/64B*4096=describe 256TiB of memory

constexpr vaddr_t KERNEL_WIRED_HEAP_BASE = 0xffffc40000000000; // -60TiB
constexpr size_t KERNEL_WIRED_HEAP_SIZE = 4ULL * 1024 * 1024 * 1024 * 1024;

constexpr size_t PAGE_SIZE = 4096;
constexpr unsigned int PAGE_SHIFT = 12;

enum {
  // These map to the PAT bits
  CACHE_UNCACHED = 0,
  CACHE_WRITECOMBINE,
  CACHE_WRITETHROUGH,
  CACHE_WRITEBACK,
  // And these provide the architecture constants
  CACHE_DEFAULT = CACHE_WRITEBACK,
  CACHE_DISABLE = CACHE_UNCACHED,
};

[[gnu::pure]]
inline vaddr_t p2v(paddr_t pa) {
  return static_cast<vaddr_t>(HHDM_BASE + pa);
}

[[gnu::const]]
inline size_t p2pfn(paddr_t pa) {
  return pa >> PAGE_SHIFT;
}

[[gnu::pure]]
inline paddr_t v2p(vaddr_t va) {
  return static_cast<vaddr_t>(va - HHDM_BASE);
}

} // namespace yak::arch
