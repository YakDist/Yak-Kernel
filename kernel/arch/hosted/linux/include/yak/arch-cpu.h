#pragma once

#include <yak/arch-intr.h>

namespace yak::arch {
static inline void hcf(bool int_state = false) {
  while (true) {
    if (int_state) {
      enable_interrupts();
    } else {
      disable_interrupts();
    }
    interrupt_wait();
  }
}
} // namespace yak::arch
