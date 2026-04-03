#pragma once

#include <yak/spinlock.h>
#include <yak/fixed_arena.h>
#include <yak/types.h>
#include <yak/vm/address.h>
#include <yak/vm/flags.h>
#include <yak/vm/page.h>

#define pmm_bytes_to_order(b) (next_ilog2((b)) - PAGE_SHIFT)

namespace yak {

#define FREELIST_ORDERS 10

struct MemoryDomain {
  IplSpinLock lock;
  PageList free_list[FREELIST_ORDERS];

  Page *allocate(unsigned int desired_order);
  void free(Page *page);
};

struct Affinity {
  int domain;
  paddr_t base;
  size_t length;

  inline paddr_t end() {
    return base + length;
  }
};

// should be enough for everyone, if not, fuck your server
static constexpr size_t MAX_AFFINITIES = 64;
extern FixedArena<Affinity, MAX_AFFINITIES> pmm_affinities;

void pmm_add_region(paddr_t base, paddr_t end);

[[nodiscard]]
Page *pmm_alloc(unsigned int order, PageUse use, OptionBits alloc_flags);

void page_release(Page *page);

} // namespace yak
