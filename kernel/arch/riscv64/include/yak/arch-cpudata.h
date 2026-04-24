#pragma once

#include <yak/ipl.h>

namespace yak::arch {
struct ArchCpuData {

  Ipl soft_ipl;
  Ipl hard_ipl;

  void md_init() {
    soft_ipl = hard_ipl = Ipl::passive;
  }
};
} // namespace yak::arch
