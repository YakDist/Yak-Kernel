#define pr_fmt(fmt) "limine: " fmt

#include <assert.h>
#include <cstddef>
#include <limine-generic/request.h>
#include <limine.h>
#include <span>
#include <yak/arch-mm.h>
#include <yak/kernel-file.h>
#include <yak/log.h>
#include <yak/math.h>
#include <yak/util.h>
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
     reinterpret_cast<vaddr_t>(&__kernel_text_end), PROT_READ | PROT_EXECUTE},

    {"rodata", reinterpret_cast<vaddr_t>(&__kernel_rodata_start),
     reinterpret_cast<vaddr_t>(&__kernel_rodata_end), PROT_READ},

    {"data", reinterpret_cast<vaddr_t>(&__kernel_data_start),
     reinterpret_cast<vaddr_t>(&__kernel_data_end), PROT_READ | PROT_WRITE}};

template <typename Traits>
static void map_pfndb_region(vaddr_t virt_base, size_t length, int nid) {
  assert(is_aligned_pow2<arch::PAGE_SIZE>(virt_base));
  assert(is_aligned_pow2<arch::PAGE_SIZE>(length));

  vaddr_t addr = virt_base;
  vaddr_t end = virt_base + length;

  while (addr < end) {
    size_t chosen_level = 0;
    size_t chosen_size = arch::PAGE_SIZE;

    // prefer largest page sizes
    for (size_t i = std::size(Traits::PAGE_SIZES); i-- > 0;) {
      size_t page_size = Traits::PAGE_SIZES[i];

      if (addr + page_size > end)
        continue;

      if (!is_aligned_pow2(addr, page_size))
        continue;

      chosen_level = i;
      chosen_size = page_size;
      break;
    }

    // allocate backing on the same NUMA node
    paddr_t pa = expect(boot_memblock.allocate(chosen_size, chosen_size, nid),
                        "could not map pfndb backing memory");

    kmap.page_map().enter_boot(addr, pa, PROT_READ | PROT_WRITE, CACHE_DEFAULT,
                               chosen_level);

    addr += chosen_size;
  }
}

void mem_init() {
  arch::HHDM_BASE = ensure_request(hhdm_request, "limine missing hhdm").offset;

  pr_info("hhdm at 0x%lx\n", arch::HHDM_BASE);

  auto paging_mode_res =
      ensure_request(paging_mode_request, "limine missing paging mode");

  switch (paging_mode_res.mode) {
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

  auto memmap_response =
      ensure_request(memmap_request, "limine missing memmap");

  auto memmap = std::span(memmap_response.entries, memmap_response.entry_count);

  for (auto entry : memmap) {
    switch (entry->type) {
    case LIMINE_MEMMAP_USABLE:
      boot_memblock.memory.add(entry->base, entry->length, 0);
      boot_memblock.usable.add(entry->base, entry->length, 0);
      break;

    case LIMINE_MEMMAP_RESERVED:
    case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
    case LIMINE_MEMMAP_ACPI_NVS:
    case LIMINE_MEMMAP_BAD_MEMORY:
    case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
    case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
      boot_memblock.memory.add(entry->base, entry->length, 0);
      boot_memblock.reserved.add(entry->base, entry->length, 0);
      break;

    case LIMINE_MEMMAP_FRAMEBUFFER:
    case LIMINE_MEMMAP_RESERVED_MAPPED:
    default:
      boot_memblock.reserved.add(entry->base, entry->length, 0);
      break;
    }
  }

  arch::post_memblock();

  boot_memblock.coalesce_blocks();

  kmap.bootstrap_kernel();

  // Map the virtually contigoouns pfndb
  for (auto &entry : boot_memblock.memory.view()) {
    pr_info("physical memory: %#016lx - %#016lx node id %d\n", entry.base,
            entry.end(), entry.node_id);
  }

  vaddr_t last_mapped_end = arch::PFNDB_BASE;

  for (auto &entry : boot_memblock.memory.view()) {
    size_t start_pfn = entry.base >> arch::PAGE_SHIFT;
    size_t end_pfn = entry.end() >> arch::PAGE_SHIFT;

    vaddr_t raw_start = arch::PFNDB_BASE + start_pfn * sizeof(Page);

    size_t raw_size = (end_pfn - start_pfn) * sizeof(Page);

    vaddr_t region_va = align_down<arch::PAGE_SIZE>(raw_start);
    size_t size = align_up<arch::PAGE_SIZE>(raw_size);

    // skip overlap
    if (region_va < last_mapped_end) {
      size_t overlap = last_mapped_end - region_va;

      // fully backed already
      if (overlap >= size)
        continue;

      region_va = last_mapped_end;
      size -= overlap;
    }

    map_pfndb_region<GenericPageMapTraits>(region_va, size, entry.node_id);

    last_mapped_end = region_va + size;
  }

  // Map the HHDM
  for (auto entry : memmap) {
    if (entry->type == LIMINE_MEMMAP_BAD_MEMORY)
      continue;

    VMCache cache = CACHE_DEFAULT;
#ifdef x86_64
    if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER)
      cache = arch::CACHE_WRITECOMBINE;
#endif

    kmap.page_map().enter_boot_large(entry->base, entry->base + arch::HHDM_BASE,
                                     entry->length, PROT_READ | PROT_WRITE,
                                     cache);
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
