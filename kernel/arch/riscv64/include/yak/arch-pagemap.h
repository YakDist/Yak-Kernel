#pragma once

#include <cstdint>
#include <iterator>
#include <yak/arch-mm.h>
#include <yak/size-macros.h>
#include <yak/vm/flags.h>
#include <yak/vm/generic_pagemap.h>

namespace yak {

struct Riscv64_Paging {
  using Pte = uint64_t;

  enum pte_masks : uint64_t {
    pteValid = UINT64_C(1) << 0,
    pteRead = UINT64_C(1) << 1,
    pteWrite = UINT64_C(1) << 2,
    pteExecute = UINT64_C(1) << 3,
    pteUser = UINT64_C(1) << 4,
    pteGlobal = UINT64_C(1) << 5,
    pteAccess = UINT64_C(1) << 6,
    pteDirty = UINT64_C(1) << 7,
    ptePpnMask = (((UINT64_C(1) << 44) - 1) << 10),
    // PBMT (Svpbmt), bits 62:61
    ptePbmtPma = UINT64_C(0) << 61, // default, caching per PMA
    ptePbmtNc = UINT64_C(1) << 61,  // non-cacheable, idempotent
    ptePbmtIo = UINT64_C(2) << 61,  // non-cacheable, non-idempotent (MMIO)
    ptePbmtMask = UINT64_C(3) << 61,
  };

  // Sv39: levels 3, Sv48: 4, Sv57: 5
  static constexpr size_t LEVEL_SHIFTS[] = {12, 21, 30, 39, 48};
  static constexpr size_t LEVEL_BITS[] = {9, 9, 9, 9, 9};
  static constexpr size_t LEVEL_ENTRIES[] = {512, 512, 512, 512, 512};
  static constexpr size_t MAX_LEVELS = 5;

  // Large pages: 2MiB (level 1), 1GiB (level 2), 512GiB (level 3, Sv48+),
  //              256TiB (level 4, Sv57+)
  static constexpr size_t PAGE_SIZES[] = {
      KiB(4), MiB(2), GiB(1), GiB(512), TiB(256),
  };

  static inline size_t levels() {
    return arch::PMAP_LEVELS;
  }

  static inline size_t max_page_size_idx() {
    // Sv39: 1 GiB
    // Sv48: 512 GiB
    // Sv57: 256TiB
    return arch::PMAP_LEVELS - 1;
  }

  static inline void activate(paddr_t top_level) {
    uint64_t mode = 8 + (arch::PMAP_LEVELS - 3);
    uint64_t ppn = top_level >> arch::PAGE_SHIFT;
    uint64_t satp = (mode << 60) | ppn;
    asm volatile("csrw satp, %0\nsfence.vma" ::"r"(satp) : "memory");
  }

  static inline bool pte_is_zero(Pte pte) {
    return pte == 0;
  }

  // A leaf PTE has R or X set; V=1 with R=W=X=0 is a directory entry.
  static inline bool pte_is_large(Pte pte, size_t level) {
    return level > 0 && (pte & (pteRead | pteExecute)) != 0;
  }

  static inline uintptr_t pte_addr(Pte pte) {
    return (pte & ptePpnMask) << 2;
  }

  // Directory entry: V=1, R=W=X=0
  static inline Pte make_dir(paddr_t pa) {
    return pteValid | (pa >> 2);
  }

  // NOTE: Svpbmt required for cache control. Without it these bits are
  // reserved.
  static inline uintptr_t pbmt_bits(VMCache cache) {
    switch (cache) {
    case arch::CACHE_WRITEBACK:
    case arch::CACHE_WRITETHROUGH: // no WT in PBMT; fall back to PMA
      return ptePbmtPma;
    case arch::CACHE_WRITECOMBINE:
    case arch::CACHE_UNCACHED:
      return ptePbmtNc;
    default:
      return ptePbmtIo;
    }
  }

  static inline Pte make_leaf(paddr_t pa, [[maybe_unused]] size_t level,
                              VMProt prot, VMCache cache) {
    Pte pte = pteValid;

    // R=1 is required whenever W=1 (W=1,R=0 is reserved)
    if (prot & PROT_READ)
      pte |= pteRead;
    if (prot & PROT_WRITE)
      pte |= pteRead | pteWrite;
    if (prot & PROT_EXECUTE)
      pte |= pteExecute;

    if (prot & PROT_USER) {
      pte |= pteUser;
    } else {
      // Eagerly set A/D bits;
      // Don't cause page faults when Svadu is not implemented
      pte |= pteAccess | pteDirty;
    }

    pte |= pbmt_bits(cache);
    pte |= (pa >> 2);
    return pte;
  }
};

using GenericPageMapTraits = Riscv64_Paging;
using PageMap = GenericPageMap<GenericPageMapTraits>;
} // namespace yak
