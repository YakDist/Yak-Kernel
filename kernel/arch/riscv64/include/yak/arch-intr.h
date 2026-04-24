#pragma once

#include <cstdint>

namespace yak::arch {

static inline void interrupt_wait() {
  asm volatile("wfi" ::: "memory");
}

static inline bool interrupt_state() {
  uintptr_t sstatus;
  asm volatile("csrr %0, sstatus" : "=r"(sstatus)::"memory");
  return (sstatus & (1 << 1)) != 0; // SIE is bit 1
}

static inline void disable_interrupts() {
  asm volatile("csrci sstatus, 0x2" ::: "memory");
}

static inline void enable_interrupts() {
  asm volatile("csrsi sstatus, 0x2" ::: "memory");
}

} // namespace yak::arch
