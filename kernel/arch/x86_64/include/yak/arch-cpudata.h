#pragma once

#include <yak/ipl.h>

namespace yak::arch {
struct ArchCpuData {
  void *syscall_scratch;

  Ipl soft_ipl;
  Ipl hard_ipl;
};
} // namespace yak::arch
