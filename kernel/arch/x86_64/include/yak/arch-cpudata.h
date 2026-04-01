#pragma once

#include <cstdint>
#include <yak/ipl.h>

namespace yak::arch {
struct ArchCpuData {
  void *syscall_scratch;

  uint32_t apic_id;

  Ipl soft_ipl;
  Ipl hard_ipl;
};
} // namespace yak::arch
