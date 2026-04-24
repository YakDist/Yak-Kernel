#pragma once

#include <x86_64/asm.h>
#include <x86_64/cpu_features.h>
#include <yak/arch-mm.h>
#include <yak/size-macros.h>
#include <yak/vm/flags.h>
#include <yak/vm/generic_pagemap.h>

namespace yak {

struct X86_Paging {
  using Pte = uint64_t;

  enum pte_masks : uint64_t {
    ptePresent = 0x1,
    pteWrite = 0x2,
    pteUser = 0x4,
    ptePwt = 0x8,
    ptePcd = 0x10,
    pteAccess = 0x20,
    pteModified = 0x40,
    ptePat = 0x80,
    ptePagesize = 0x80,
    pteGlobal = 0x100,
    ptePatLarge = (1 << 12),
    pteAddress = 0x000FFFFFFFFFF000,
    pteLargeAddress = 0x000FFFFFFFFFe000,
    pteNoExecute = 0x8000000000000000,
  };

  static constexpr size_t LEVEL_SHIFTS[] = {12, 21, 30, 39, 48};
  static constexpr size_t LEVEL_BITS[] = {9, 9, 9, 9, 9};
  static constexpr size_t LEVEL_ENTRIES[] = {512, 512, 512, 512, 512};
  static constexpr size_t PAGE_SIZES[] = {KiB(4), MiB(2), GiB(1)};

  static constexpr size_t MAX_LEVELS = 5;

  static inline size_t levels() {
    return arch::PMAP_LEVELS;
  }

  static inline size_t max_page_size_idx() {
    if (!arch::bsp_cpu_features.pdpe1gb)
      return 1;
    return 2;
  }

  static inline void activate(paddr_t top_level) {
    asm_wrcr3(top_level);
  }

  static inline bool pte_is_zero(Pte pte) {
    return pte == 0;
  }
  static inline bool pte_is_large(Pte pte, size_t level) {
    return level > 0 && (pte & ptePagesize) != 0;
  }

  static inline uintptr_t pte_addr(Pte pte) {
    return pte & pteAddress;
  }

  static inline Pte make_dir(paddr_t pa) {
    return ptePresent | pteWrite | pteUser | pa;
  }

  static inline uintptr_t pat_bits(size_t level, VMCache cache) {
    uintptr_t bits = 0;

    switch (cache) {
    case arch::CACHE_WRITECOMBINE:
      if (level > 0)
        bits = ptePatLarge;
      else
        bits = ptePat;
      bits |= ptePwt;
      break;
    case arch::CACHE_WRITETHROUGH:
      bits = ptePwt;
      break;
    case arch::CACHE_WRITEBACK:
      break;
    case arch::CACHE_UNCACHED:
      bits = ptePwt | ptePwt;
      break;
    default:
      bits = ptePcd | ptePwt;
    }

    return bits;
  }

  static inline Pte make_leaf(paddr_t pa, size_t level, VMProt prot,
                              VMCache cache) {
    Pte pte = 0;

    if (prot & PROT_READ)
      pte |= ptePresent;

    if (prot & PROT_WRITE)
      pte |= ptePresent | pteWrite;

    if (!(prot & PROT_EXECUTE))
      pte |= pteNoExecute;

    if (prot & PROT_USER)
      pte |= pteUser;

    if (level > 0)
      pte |= ptePagesize;

    pte |= pat_bits(level, cache);

    pte |= pa;

    return pte;
  }
};

using GenericPageMapTraits = X86_Paging;
using PageMap = GenericPageMap<GenericPageMapTraits>;
} // namespace yak
