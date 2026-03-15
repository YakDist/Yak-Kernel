#pragma once

#include <stdint.h>

namespace yak::arch::msr {
constexpr uint32_t LAPIC_BASE = 0x1B;
constexpr uint32_t PAT = 0x277;
constexpr uint32_t EFER = 0xC0000080;
constexpr uint32_t STAR = 0xC0000081;
constexpr uint32_t LSTAR = 0xC0000082;
constexpr uint32_t FMASK = 0xC0000083;
constexpr uint32_t FSBASE = 0xC0000100;
constexpr uint32_t GSBASE = 0xC0000101;
constexpr uint32_t KERNEL_GSBASE = 0xC0000102;
} // namespace yak::arch::msr
