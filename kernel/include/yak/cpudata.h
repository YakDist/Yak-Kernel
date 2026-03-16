#pragma once

#include <cstddef>
#include <yak/arch-cpudata.h>

namespace yak {
struct CpuData {
  CpuData *self;
  arch::ArchCpuData md;

  size_t id;

  void *kernel_stack_top;
};
} // namespace yak
