#pragma once

#include <stdint.h>

namespace yak::arch {

namespace cr0 {
constexpr uint64_t CD = 1ULL << 30; /* Cache Disable */
constexpr uint64_t NW = 1ULL << 29; /* Not-writethrough */
constexpr uint64_t WP = 1ULL << 16; /* Write-protect */
} // namespace cr0

namespace cr4 {
constexpr uint64_t PSE = 1ULL << 4; /* Page Size Extension */
constexpr uint64_t PGE = 1ULL << 7; /* Page Global Extension */
} // namespace cr4

namespace efer {
constexpr uint64_t SCE = 1ULL << 0;  /* syscall enable */
constexpr uint64_t NXE = 1ULL << 11; /* execute disable bit enable */
} // namespace efer

} // namespace yak::arch
