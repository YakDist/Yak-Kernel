#pragma once

#include <cstdint>

namespace yak::arch {

static inline bool interrupt_state() {
  uint64_t fq;
  asm volatile("pushfq\n\t"
               "pop %0\n\t"
               : "=rm"(fq)::"memory");
  return (fq & (1 << 9)) != 0;
}

static inline void disable_interrupts() { asm volatile("cli"); }

static inline void enable_interrupts() { asm volatile("sti"); }

} // namespace yak::arch
