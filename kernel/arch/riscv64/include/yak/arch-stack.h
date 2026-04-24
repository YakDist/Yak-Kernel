#pragma once

#include <cstddef>
#include <yak/arch-mm.h>

namespace yak::arch {
// 16kb
constexpr size_t KSTACK_SIZE = arch::PAGE_SIZE * 4;
} // namespace yak::arch
