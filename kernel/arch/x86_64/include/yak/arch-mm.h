#pragma once

#include <cstddef>
#include <yak/vm/address.h>

namespace yak::arch {

extern vaddr_t HHDM_BASE;
extern size_t PMAP_LEVELS;

constexpr size_t PAGE_SIZE = 4096;
constexpr unsigned int PAGE_SHIFT = 12;

[[gnu::pure]]
inline vaddr_t p2v(paddr_t pa) {
  return static_cast<vaddr_t>(HHDM_BASE + pa);
}

} // namespace yak::arch
