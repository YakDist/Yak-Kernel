#pragma once

#include <cstdint>
#include <yak/ipl.h>

namespace yak::arch {
struct ArchCpuData {
  void *syscall_scratch;

  uint32_t apic_id;

  Ipl soft_ipl;
  Ipl hard_ipl;

  void md_init() {
    syscall_scratch = nullptr;
    apic_id = -1;
    soft_ipl = hard_ipl = Ipl::passive;
  }
};
} // namespace yak::arch
