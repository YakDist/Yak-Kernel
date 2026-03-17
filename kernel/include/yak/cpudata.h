#pragma once

#include <cstddef>
#include <yak/percpu.h>
#include <yak/arch-cpudata.h>

namespace yak {
struct CpuData {
  CpuData *self;
  arch::ArchCpuData md;

  size_t id;
  bool bsp;

  void *kernel_stack_top;
};

static inline bool cpu_is_bsp() {
  return CPUDATA_LOAD(bsp);
}

} // namespace yak
