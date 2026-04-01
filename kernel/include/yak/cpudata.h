#pragma once

#include <cstddef>
#include <yak/arch-cpudata.h>
#include <yak/percpu.h>

namespace yak {

struct Affinity {
  int memory_domain;
};

struct CpuData {
  CpuData *self;
  arch::ArchCpuData md;

  Affinity affinity;

  size_t id;
  bool bsp;

  void *kernel_stack_top;
};

static inline bool cpu_is_bsp() {
  return CPUDATA_LOAD(bsp);
}

} // namespace yak
