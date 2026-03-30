#define pr_fmt(fmt) "limine: " fmt

#include <cstddef>
#include <limine-generic/limine.h>
#include <limine-generic/request.h>
#include <yak/arch-mm.h>
#include <yak/log.h>
#include <yak/vm/address.h>
#include <yak/vm/pmm.h>

extern "C" {

LIMINE_REQ static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

LIMINE_REQ static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 3,
    .response = nullptr,
};

LIMINE_REQ static volatile struct limine_paging_mode_request
    paging_mode_request = {
        .id = LIMINE_PAGING_MODE_REQUEST_ID,
        .revision = 1,
        .response = nullptr,
#ifdef x86_64
        .mode = LIMINE_PAGING_MODE_X86_64_5LVL,
        .max_mode = LIMINE_PAGING_MODE_X86_64_5LVL,
        .min_mode = LIMINE_PAGING_MODE_X86_64_4LVL,
#elif defined(riscv64)
        .mode = LIMINE_PAGING_MODE_RISCV_SV57,
        .max_mode = LIMINE_PAGING_MODE_RISCV_SV57,
        .min_mode = LIMINE_PAGING_MODE_RISCV_SV39,
#endif
};
}

namespace yak::arch {
size_t HHDM_BASE;
size_t PMAP_LEVELS;
} // namespace yak::arch

namespace yak::limine {

void mem_init() {
  arch::HHDM_BASE = hhdm_request.response->offset;

  pr_info("hhdm at 0x%lx\n", arch::HHDM_BASE);

  switch (paging_mode_request.response->mode) {
#if defined(x86_64)
  case LIMINE_PAGING_MODE_X86_64_5LVL:
    arch::PMAP_LEVELS = 5;
    break;
  case LIMINE_PAGING_MODE_X86_64_4LVL:
    arch::PMAP_LEVELS = 4;
    break;
#elif defined(riscv64)
  case LIMINE_PAGING_MODE_RISCV_SV57:
    arch::PMAP_LEVELS = 5;
    break;
  case LIMINE_PAGING_MODE_RISCV_SV48:
    arch::PMAP_LEVELS = 4;
    break;
  case LIMINE_PAGING_MODE_RISCV_SV39:
    arch::PMAP_LEVELS = 3;
    break;
#else
#error "Port limine-generic to your platform"
#endif
  }

  pr_info("%ld pmap levels\n", arch::PMAP_LEVELS);

  auto memmap = memmap_request.response;

  for (size_t i = 0; i < memmap->entry_count; i++) {
    auto entry = memmap->entries[i];
    if (entry->type != LIMINE_MEMMAP_USABLE)
      continue;

    pmm_add_region(entry->base, entry->base + entry->length);
  }

  for (size_t i = 0; i < memmap->entry_count; i++) {
    auto entry = memmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_BAD_MEMORY)
      continue;

    paddr_t start = entry->base;
    paddr_t end = start + entry->length;

    // XXX: map to hhdm
  }
}

} // namespace yak::limine
