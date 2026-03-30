#define pr_fmt(fmt) "pmm: " fmt

#include <assert.h>
#include <cstddef>
#include <frg/mutex.hpp>
#include <string.h>
#include <yak/arch-mm.h>
#include <yak/log.h>
#include <yak/math.h>
#include <yak/panic.h>
#include <yak/spinlock.h>
#include <yak/vm/address.h>
#include <yak/vm/alloc.h>
#include <yak/vm/page.h>
#include <yak/vm/pmm.h>

namespace yak {

#define FREELIST_ORDERS 10

struct Domain {
  IplSpinLock lock;
  PageList free_list[FREELIST_ORDERS];
};

// TODO: Replace page lookup with a sparsely mapped page array
struct Region {
  size_t base;
  size_t end;
  frg::default_list_hook<Region> hook;
  Page pages[];
};

using RegionList = frg::intrusive_list<
    Region,
    frg::locate_member<Region, frg::default_list_hook<Region>, &Region::hook>>;

constinit RegionList region_list;

constinit Domain g_domain;

[[gnu::pure]]
static size_t pmm_block_size(unsigned int order) {
  return 1ULL << (arch::PAGE_SHIFT + order);
}

Region *pmm_region_lookup(paddr_t base) {
  for (auto reg : region_list) {
    if (base >= reg->base && base < reg->end)
      return reg;
  }
  return nullptr;
}

void pmm_add_region(paddr_t base, paddr_t end) {
  assert(is_aligned_pow2(base, arch::PAGE_SIZE));
  assert(is_aligned_pow2(end, arch::PAGE_SIZE));

  Domain *domain = &g_domain;

  size_t length = end - base;
  size_t npages_total = length >> arch::PAGE_SHIFT;

  size_t used_size = sizeof(Region) + sizeof(Page) * npages_total;
  size_t npages_used = div_roundup(used_size, arch::PAGE_SIZE);

  Region *region = reinterpret_cast<Region *>(arch::p2v(base));
  Page *pages = &region->pages[0];

  // Zero the meta
  memset(pages, 0, sizeof(Page) * npages_total);

  size_t base_pfn = base >> arch::PAGE_SHIFT;
  for (size_t i = 0; i < npages_used; i++) {
    Page &page = pages[i];
    page.usage = PageUse::Reserved;
    page.pfn = base_pfn + i;
    page.refcnt = 1;
    page.order = page.block_order = 0;
  }

  // Discard very small regions
  if (npages_used >= npages_total)
    return;

  for (size_t block_base = base + npages_used * arch::PAGE_SIZE;
       block_base < end;) {
    unsigned int max_order = 0;
    while (max_order < FREELIST_ORDERS - 1) {
      unsigned int next = max_order + 1;
      size_t size = pmm_block_size(next);
      if (block_base + size > end || !is_aligned_pow2(block_base, size))
        break;
      max_order++;
    }

    size_t block_size = pmm_block_size(max_order);
    size_t block_pages = block_size / arch::PAGE_SIZE;

    assert(block_base + block_size <= end);
    assert(is_aligned_pow2(block_base, block_size));
    // clang-format off
    assert(max_order == FREELIST_ORDERS - 1 
	   || block_base + (block_size * 2) > end 
	   || !is_aligned_pow2(block_base, block_size * 2));
    // clang-format on

    size_t block_pfn = block_base >> arch::PAGE_SHIFT;
    Page *block_page = &pages[block_pfn - base_pfn];

    for (size_t i = 0; i < block_pages; i++) {
      Page *page = &block_page[i];
      page->usage = PageUse::Free;
      page->pfn = block_pfn + i;
      page->refcnt = 0;
      page->order = max_order;
      page->block_order = max_order;
    }

    domain->free_list[max_order].push_back(block_page);

    block_base += block_size;
  }

  pr_info("add region: %#016lx - %#016lx (%ld pages; %ld usable)\n", base, end,
          npages_total, npages_total - npages_used);
}

static Page *dom_alloc(Domain *dom, unsigned int desired_order) {
  assert(desired_order < FREELIST_ORDERS);

  if (!dom->free_list[desired_order].empty()) {
    Page *page = dom->free_list[desired_order].pop_front();
    assert(page->usage == PageUse::Free);
    page->refcnt = 1;
    return page;
  }

  unsigned int order = desired_order;
  while (++order < FREELIST_ORDERS && dom->free_list[order].empty()) {
  }

  if (order >= FREELIST_ORDERS) {
    return nullptr;
  }

  Page *page = dom->free_list[order].pop_front();
  Page *buddy = page + (1 << order) / 2;

  while (order != desired_order) {
    order -= 1;
    buddy->order = order;
    dom->free_list[order].push_back(buddy); // put upper half on free list
    buddy = page + (1 << order) / 2;        // recalculate buddy for new order
  }

  assert(page->usage == PageUse::Free);
  page->order = desired_order;
  page->refcnt = 1;

  return page;
}

static void dom_free(Domain *dom, Page *page) {
  while (page->order < page->block_order) {
    auto block_size = pmm_block_size(page->order);
    paddr_t page_addr = page->to_pa();
    paddr_t buddy_addr = page_addr ^ block_size;

    Page *buddy_page;

    // The buddy page is either +block_size or -block_size away
    if (page_addr > buddy_addr) {
      auto delta = (page_addr - buddy_addr) >> arch::PAGE_SHIFT;
      buddy_page = page - delta;
    } else { /* page_addr < buddy_addr */
      auto delta = (buddy_addr - page_addr) >> arch::PAGE_SHIFT;
      buddy_page = page + delta;
    }

    // both pages must belong to the same initial block
    assert(buddy_page->block_order == page->block_order);

    if (buddy_page->usage != PageUse::Free)
      break;
    if (buddy_page->order != page->order)
      break;

    auto &list = dom->free_list[page->order];
    list.erase(list.iterator_to(buddy_page));

    // choose the lower half
    if (buddy_page < page) {
      page = buddy_page;
    }

    page->order += 1;
  }

  page->usage = PageUse::Free;
  dom->free_list[page->order].push_front(page);
}

Page *pmm_alloc(unsigned int order, PageUse use, OptionBits flags) {
  auto dom = &g_domain;
  auto guard = frg::guard(&dom->lock);
  auto page = dom_alloc(dom, order);

  if (!page) {
    panic("handle pmm OOM!");
  }

  page->usage = use;

  if (flags & ALLOC_ZERO) {
    memset(page->to_va_ptr(), 0, page->block_size());
  }

  return page;
}

void page_release(Page *page) {
  auto dom = &g_domain;
  auto guard = frg::guard(&dom->lock);

  if (page->refcnt-- == 1) {
    // NOTE: differentiate based on PageUse in the future
    dom_free(dom, page);
  }
}

} // namespace yak
