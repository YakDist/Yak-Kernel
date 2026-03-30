#pragma once

#include <frg/intrusive.hpp>
#include <frg/list.hpp>
#include <yak/arch-mm.h>
#include <yak/vm/address.h>
#include <yak/vm/alloc.h>

namespace yak {

struct [[gnu::aligned(64)]] Page {
  PageUse usage;
  size_t pfn;
  size_t refcnt;
  unsigned int order, block_order;
  frg::default_list_hook<Page> hook;

  inline paddr_t to_pa() { return pfn << arch::PAGE_SHIFT; }
  inline vaddr_t to_va() {
    paddr_t pa = to_pa();
    return arch::p2v(pa);
  }
  inline void *to_va_ptr() { return reinterpret_cast<void *>(to_va()); }
  inline size_t block_size() { return 1ULL << (order + arch::PAGE_SHIFT); }
};

static_assert(sizeof(Page) == 64);

using PageList = frg::intrusive_list<
    Page, frg::locate_member<Page, frg::default_list_hook<Page>, &Page::hook>>;

} // namespace yak
