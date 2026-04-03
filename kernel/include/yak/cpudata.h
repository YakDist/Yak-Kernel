#pragma once

#include <cstddef>
#include <yak/arch-cpudata.h>
#include <yak/percpu.h>

namespace yak {

struct CpuData {
  CpuData *self;
  arch::ArchCpuData md;

  unsigned int numa_domain;

  size_t id;
  bool bsp;

  void *kernel_stack_top;
};

static inline bool cpu_is_bsp() {
  return CPUDATA_LOAD(bsp);
}

} // namespace yak
