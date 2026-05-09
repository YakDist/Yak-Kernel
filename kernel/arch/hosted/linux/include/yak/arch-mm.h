#pragma once

#include <cstddef>
#include <yak/types.h>
#include <yak/vm/address.h>

namespace yak::arch {

extern vaddr_t HHDM_BASE;
extern vaddr_t PFNDB_BASE;
extern size_t PFNDB_SIZE;
extern size_t PAGE_SIZE;
extern unsigned int PAGE_SHIFT;

enum {
  CACHE_DEFAULT = 0,
  CACHE_DISABLE = CACHE_DEFAULT,
};

inline vaddr_t p2v(paddr_t pa) {
  return static_cast<vaddr_t>(HHDM_BASE + pa);
}

inline paddr_t v2p(vaddr_t va) {
  return static_cast<vaddr_t>(va - HHDM_BASE);
}

inline size_t p2pfn(paddr_t pa) {
  return pa >> PAGE_SHIFT;
}

} // namespace yak::arch
