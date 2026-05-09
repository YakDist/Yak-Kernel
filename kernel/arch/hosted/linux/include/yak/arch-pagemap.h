#pragma once

#include <x86_64/asm.h>
#include <x86_64/cpu_features.h>
#include <yak/arch-mm.h>
#include <yak/size-macros.h>
#include <yak/vm/flags.h>
#include <yak/vm/generic_pagemap.h>

namespace yak {

struct PageMap {
  void bootstrap_kernel() {}
  void activate() {}
};

} // namespace yak
