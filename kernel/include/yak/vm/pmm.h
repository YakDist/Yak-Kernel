#pragma once

#include <yak/types.h>
#include <yak/vm/address.h>
#include <yak/vm/flags.h>
#include <yak/vm/page.h>

#define pmm_bytes_to_order(b) (next_ilog2((b)) - PAGE_SHIFT)

namespace yak {
void pmm_add_region(paddr_t base, paddr_t end);

[[nodiscard]]
Page *pmm_alloc(unsigned int order, PageUse use, OptionBits alloc_flags);

void page_release(Page *page);

} // namespace yak
