#include <stdint.h>
#include <x86_64/gdt.h>
#include <x86_64/tss.h>
#include <yak/arch-intr.h>
#include <yak/cpudata.h>
#include <yak/percpu.h>
#include <yak/vm/address.h>
#include <yak/vm/stack.h>

namespace yak::arch {
struct GdtEntry {
  uint16_t limit;
  uint16_t low;
  uint8_t mid;
  uint8_t access;
  uint8_t granularity;
  uint8_t high;
};
static_assert(offsetof(GdtEntry, limit) == 0);
static_assert(offsetof(GdtEntry, low) == 2);
static_assert(offsetof(GdtEntry, mid) == 4);
static_assert(offsetof(GdtEntry, access) == 5);
static_assert(offsetof(GdtEntry, granularity) == 6);
static_assert(offsetof(GdtEntry, high) == 7);
static_assert(sizeof(GdtEntry) == 8);

struct GdtTssEntry {
  uint16_t length;
  uint16_t low;
  uint8_t mid;
  uint8_t access;
  uint8_t flags;
  uint8_t high;
  uint32_t upper;
  uint32_t rsv;
};
static_assert(offsetof(GdtTssEntry, length) == 0);
static_assert(offsetof(GdtTssEntry, low) == 2);
static_assert(offsetof(GdtTssEntry, mid) == 4);
static_assert(offsetof(GdtTssEntry, access) == 5);
static_assert(offsetof(GdtTssEntry, flags) == 6);
static_assert(offsetof(GdtTssEntry, high) == 7);
static_assert(offsetof(GdtTssEntry, upper) == 8);
static_assert(offsetof(GdtTssEntry, rsv) == 12);
static_assert(sizeof(GdtTssEntry) == 16);

struct Gdt {
  GdtEntry entries[std::to_underlying(GdtIndex::Tss)];
  GdtTssEntry tss;
};
static_assert(sizeof(Gdt) ==
              std::to_underlying(GdtIndex::Tss) * sizeof(GdtEntry) +
                  sizeof(GdtTssEntry));

[[gnu::aligned(16)]]
static Gdt pcpu_gdt PERCPU_DECL;

[[gnu::aligned(16)]]
static Tss pcpu_tss PERCPU_DECL;

static inline GdtEntry *percpu_gdt_entry(GdtIndex index) {
  return PERCPU_PTR(pcpu_gdt.entries[std::to_underlying(index)]);
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

static inline void tss_reload() {
  auto state = interrupt_state();
  disable_interrupts();

  vaddr_t tss_addr = (vaddr_t) PERCPU_PTR(pcpu_tss);

  GdtTssEntry tss_entry;
  tss_entry.low = (uint16_t) (tss_addr);
  tss_entry.mid = (uint8_t) (tss_addr >> 16);
  tss_entry.high = (uint8_t) (tss_addr >> 24);
  tss_entry.upper = (uint32_t) (tss_addr >> 32);
  tss_entry.flags = 0;
  tss_entry.access = 0b10001001;
  tss_entry.rsv = 0;

  *percpu_tss_entry() = tss_entry;

  asm volatile("ltr %0" ::"rm"((uint16_t) GDT_SEL_TSS) : "memory");

  if (state)
    enable_interrupts();
}

void tss_cpu_init() {
  Tss *tss = PERCPU_PTR(pcpu_tss);
  memset(tss, 0, sizeof(Tss));
  tss->ist1 = kstack_alloc();
  tss->ist2 = kstack_alloc();
  tss->ist3 = kstack_alloc();
  tss_reload();
}

} // namespace yak::arch
