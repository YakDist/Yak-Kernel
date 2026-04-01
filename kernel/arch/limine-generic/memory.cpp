#define pr_fmt(fmt) "limine: " fmt

#include <cstddef>
#include <limine.h>
#include <limine-generic/request.h>
#include <yak/arch-mm.h>
#include <yak/kernel-file.h>
#include <yak/log.h>
#include <yak/math.h>
#include <yak/vm/address.h>
#include <yak/vm/flags.h>
#include <yak/vm/map.h>
#include <yak/vm/memblock.h>
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

LIMINE_REQ static volatile struct limine_executable_address_request
    executable_address_request = {
        .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
        .revision = 0,
        .response = nullptr,
};
}

namespace yak::arch {
size_t HHDM_BASE;
size_t PMAP_LEVELS;

[[gnu::weak]]
void post_memblock() {}

} // namespace yak::arch

namespace yak::limine {

struct KernelSection {
  const char *name = nullptr;
  vaddr_t start = 0;
  vaddr_t end = 0;
  VMProt prot = 0;
};

static const KernelSection kernel_sections[] = {
    {"limine", reinterpret_cast<vaddr_t>(&__kernel_limine_start),
        reinterpret_cast<vaddr_t>(&__kernel_limine_end), PROT_READ},

    {"text", reinterpret_cast<vaddr_t>(&__kernel_text_start),
        reinterpret_cast<vaddr_t>(&__kernel_text_end),
        PROT_READ | PROT_EXECUTE},

    {"rodata", reinterpret_cast<vaddr_t>(&__kernel_rodata_start),
        reinterpret_cast<vaddr_t>(&__kernel_rodata_end), PROT_READ},

    {"data", reinterpret_cast<vaddr_t>(&__kernel_data_start),
        reinterpret_cast<vaddr_t>(&__kernel_data_end), PROT_READ | PROT_WRITE}};

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
    if (entry->type == LIMINE_MEMMAP_USABLE) {
      boot_memblock.usable.add(entry->base, entry->length, 0);
    } else if (entry->type == LIMINE_MEMMAP_EXECUTABLE_AND_MODULES ||
               entry->type == LIMINE_MEMMAP_RESERVED ||
               entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE ||
               entry->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE ||
               entry->type == LIMINE_MEMMAP_ACPI_NVS) {
      boot_memblock.reserved.add(entry->base, entry->length, 0);
    }
  }

  boot_memblock.coalesce_blocks();

  arch::post_memblock();

  kmap.bootstrap_kernel();

  // Map the HHDM
  for (size_t i = 0; i < memmap->entry_count; i++) {
    auto entry = memmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_BAD_MEMORY)
      continue;

    VMCache cache = CACHE_DEFAULT;
#ifdef x86_64
    if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER)
      cache = arch::CACHE_WRITECOMBINE;
#endif

    kmap.page_map().enter_boot_large(entry->base, entry->base + arch::HHDM_BASE,
        entry->length, PROT_READ | PROT_WRITE, cache);
  }

  vaddr_t kernel_vbase = executable_address_request.response->virtual_base;
  paddr_t kernel_pbase = executable_address_request.response->physical_base;

  // Map all sections
  for (const auto &sec : kernel_sections) {
    vaddr_t start = align_down<arch::PAGE_SIZE>(sec.start);
    vaddr_t end = align_up<arch::PAGE_SIZE>(sec.end);

    pr_debug("remap kernel section %s: %#016lx - %#016lx (%#x)\n", sec.name,
        start, end, sec.prot);

    kmap.page_map().enter_boot_large(
        start - kernel_vbase + kernel_pbase, // physical address offset
        start,                               // virtual address
        end - start,                         // size
        sec.prot, CACHE_DEFAULT);
  }

  kmap.activate();
}

} // namespace yak::limine
