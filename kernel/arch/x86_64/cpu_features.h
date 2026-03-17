#pragma once

namespace yak::arch {
struct CpuFeatures {
  bool nx;
  bool pdpe1gb;
  bool pge;
  bool pat;
  bool pcid;
  bool pse;
  bool xsave;
  bool avx;
};

extern CpuFeatures bsp_cpu_features;

} // namespace yak::arch
