#pragma once

#include <yak/cpumask.h>
#include <yak/vm/pmm.h>

namespace yak {

struct Domain {
  unsigned int id = 0;
  unsigned int firmware_id = 0;

  CpuMask cpus;

  MemoryDomain memory;

  static Domain &from_id(unsigned int id);
  static Domain &from_firmware_id(unsigned int firmware_id);
};

} // namespace yak
