#pragma once

#include <cstdint>

namespace yak::arch {
struct CpuFeatures {
  uint32_t max_leaf;
  bool nx;
  bool pdpe1gb;
  bool pge;
  bool pat;
  bool pcid;
  bool pse;
  bool xsave;
  bool avx;
  bool x2apic;
  bool mwait;
};

extern CpuFeatures bsp_cpu_features;

} // namespace yak::arch
