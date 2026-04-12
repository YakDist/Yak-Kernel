#pragma once

#include <cstddef>
#include <optional>
#include <yak/fixed_arena.h>
#include <yak/math.h>
#include <yak/spinlock.h>
#include <yak/types.h>
#include <yak/vm/address.h>
#include <yak/vm/flags.h>
#include <yak/vm/page.h>

namespace yak {

#define FREELIST_ORDERS 10

struct MemoryDomain {
  IplSpinlock lock;
  PageList free_list[FREELIST_ORDERS];

  std::optional<Page *> allocate(unsigned int desired_order);
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

constexpr inline unsigned int pmm_size_to_order(size_t size) {
  return next_ilog2(size) - arch::PAGE_SHIFT;
}

void pmm_add_region(paddr_t base, paddr_t end);

[[nodiscard]]
std::optional<Page *> pmm_alloc(unsigned int order, PageUse use,
                                OptionBits alloc_flags);

void page_release(Page *page);

} // namespace yak
