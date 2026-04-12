#define pr_fmt(fmt) "apic: " fmt

#include <assert.h>
#include <cstdint>
#include <x86_64/asm.h>
#include <x86_64/cpu_features.h>
#include <x86_64/msr.h>
#include <yak/arch-intr.h>
#include <yak/arch-mm.h>
#include <yak/panic.h>
#include <yak/vm/map.h>

namespace yak::arch::lapic {

enum {
  LAPIC_REG_ID = 0x20,
  LAPIC_REG_VERSION = 0x30,
  LAPIC_REG_TPR = 0x80,
  LAPIC_REG_APR = 0x90,
  LAPIC_REG_PPR = 0xA0,
  LAPIC_REG_EOI = 0xB0,
  LAPIC_REG_RRD = 0xC0,
  LAPIC_REG_LOGICAL_DEST = 0xD0,
  LAPIC_REG_DEST_FORMAT = 0xE0,
  LAPIC_REG_SPURIOUS = 0xF0,
  LAPIC_REG_ISR = 0x100,
  LAPIC_REG_TMR = 0x180,
  LAPIC_REG_IRR = 0x200,
  LAPIC_REG_STATUS = 0x280,
  LAPIC_REG_CMCI = 0x2F0,
  LAPIC_REG_ICR0 = 0x300,
  LAPIC_REG_ICR1 = 0x310,
  LAPIC_REG_LVT_TIMER = 0x320,
  LAPIC_REG_LVT_THERMAL = 0x330,
  LAPIC_REG_LVT_PERFMON = 0x340,
  LAPIC_REG_LVT_LINT0 = 0x350,
  LAPIC_REG_LVT_LINT1 = 0x360,
  LAPIC_REG_LVT_ERROR = 0x370,

  LAPIC_REG_TIMER_INITIAL = 0x380,
  LAPIC_REG_TIMER_CURRENT = 0x390,
  LAPIC_REG_TIMER_DIVIDE = 0x3E0,
};

enum {
  X2APIC_MSR_ID = 0x802,
};

static vaddr_t apic_vbase = -1;
static bool use_x2apic = false;

static inline uintptr_t xapic_base() {
  return asm_rdmsr(msr::LAPIC_BASE) & 0xffffffffff000;
}

static inline void xapic_write(uint16_t offset, uint32_t value) {
  assert((offset & 15) == 0);
  asm volatile("movl %1, (%0)" ::"r"((void *) (apic_vbase + offset)), "r"(value)
               : "memory");
}

static inline uint32_t xapic_read(uint16_t offset) {
  uint32_t val;
  assert((offset & 15) == 0);

  asm volatile("movl (%1), %0"
               : "=r"(val)
               : "r"((void *) (apic_vbase + offset))
               : "memory");

  return val;
}

uint32_t id() {
  if (use_x2apic) {
    return asm_rdmsr(X2APIC_MSR_ID) & 0xffff;
  } else {
    return xapic_read(LAPIC_REG_ID) >> 24;
  }
}

void eoi() {
  if (use_x2apic) {
  } else {
    xapic_write(LAPIC_REG_EOI, 0);
  }
}

void send_ipi(uint32_t lapic_id, uint8_t vector) {
  if (use_x2apic) {

  } else {
    auto state = interrupt_state();
    disable_interrupts();

    xapic_write(LAPIC_REG_ICR1, lapic_id << 24);
    xapic_write(LAPIC_REG_ICR0, vector);

    if (state)
      enable_interrupts();
  }
}

void init_cpu() {
  if (bsp_cpu_features.x2apic) {
    use_x2apic = true;
  }

  if (use_x2apic) {
    auto apic_base = asm_rdmsr(msr::LAPIC_BASE);
    // enable x2apic
    asm_wrmsr(msr::LAPIC_BASE, apic_base | (1 << 10));
  } else {
    panic("setup apic pages\n");
  }
}

} // namespace yak::arch::lapic
