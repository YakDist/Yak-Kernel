#include <stdint.h>
#include <x86_64/gdt.h>
#include <yak/cpudata.h>
#include <yak/percpu.h>

namespace yak::arch {
struct [[gnu::packed]] GdtEntry {
  uint16_t limit;
  uint16_t low;
  uint8_t mid;
  uint8_t access;
  uint8_t granularity;
  uint8_t high;
};

struct [[gnu::packed]] GdtTssEntry {
  uint16_t length;
  uint16_t low;
  uint8_t mid;
  uint8_t access;
  uint8_t flags;
  uint8_t high;
  uint32_t upper;
  uint32_t rsv;
};

struct [[gnu::packed]] Gdt {
  GdtEntry entries[static_cast<int>(GdtIndex::Tss)];
  GdtTssEntry tss;
};

static Gdt pcpu_gdt PERCPU_DECL;
static Gdt pcpu_tss PERCPU_DECL;

static inline GdtEntry *percpu_gdt_entry(GdtIndex index) {
  return PERCPU_PTR(pcpu_gdt.entries[static_cast<int>(index)]);
}

static inline GdtTssEntry *percpu_tss_entry() {
  return PERCPU_PTR(pcpu_gdt.tss);
}

static void gdt_make_entry(GdtIndex index, uint32_t base, uint16_t limit,
                           uint8_t access, uint8_t granularity) {
  GdtEntry entry{
      .limit = limit,
      .low = static_cast<uint16_t>(base),
      .mid = static_cast<uint8_t>(base >> 16),
      .access = access,
      .granularity = granularity,
      .high = static_cast<uint8_t>(base >> 24),
  };

  *percpu_gdt_entry(index) = entry;
}

static void gdt_tss_entry() {
  GdtTssEntry entry{};
  entry.length = 104;
  entry.access = 0x89;

  *percpu_tss_entry() = entry;
}

void gdt_init() {
  gdt_make_entry(GdtIndex::Null, 0, 0, 0, 0);
  gdt_make_entry(GdtIndex::KernelCode, 0, 0, 0x9a, 0x20);
  gdt_make_entry(GdtIndex::KernelData, 0, 0, 0x92, 0);
  gdt_make_entry(GdtIndex::UserData, 0, 0, 0xf2, 0);
  gdt_make_entry(GdtIndex::UserCode, 0, 0, 0xfa, 0x20);

  gdt_tss_entry();
}

void gdt_reload() {
  struct [[gnu::packed]] {
    uint16_t limit;
    uint64_t base;
  } gdtr = {
      .limit = sizeof(Gdt) - 1,
      .base = (uint64_t) PERCPU_PTR(pcpu_gdt),
  };

  asm volatile( //
      "lgdt (%0)\n\t"
      // reload CS
      "pushq %1\n\t"
      "leaq 1f(%%rip), %%rax\n\t"
      "pushq %%rax\n\t"
      "lretq\n\t"
      "1:\n\t"
      // reload DS
      "mov %2, %%ds\n\t"
      "mov %2, %%es\n\t"
      "mov %2, %%ss\n\t"
      "xor %%eax, %%eax\n\t"
      "mov %%ax, %%fs\n\t"
      :
      : "r"(&gdtr), "i"((uint16_t) GDT_SEL_KERNEL_CODE),
        "r"(GDT_SEL_KERNEL_DATA)
      : "rax", "memory");
}

} // namespace yak::arch
