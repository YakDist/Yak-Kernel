#pragma once

#include <cstddef>
#include <yak/arch-cpudata.h>
#include <yak/percpu.h>
#include <yak/sched.h>
#include <yak/thread.h>

namespace yak {

struct CpuData {
  CpuData *self;
  arch::ArchCpuData md;

  unsigned int numa_domain;

  Thread *current_thread;
  Scheduler sched;

  size_t id;
  bool bsp;

  void *kernel_stack_top;
};

static inline CpuData &get_cpu_self() {
  return *CPUDATA_LOAD(self);
}

static inline bool cpu_is_bsp() {
  return CPUDATA_LOAD(bsp);
}

} // namespace yak
