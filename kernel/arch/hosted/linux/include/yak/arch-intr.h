#pragma once

namespace yak::arch {

static inline void interrupt_wait() {}

static inline bool interrupt_state() {
  return false;
}

static inline void disable_interrupts() {}

static inline void enable_interrupts() {}

} // namespace yak::arch
