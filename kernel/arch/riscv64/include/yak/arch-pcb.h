#pragma once
#include <stddef.h>
#include <stdint.h>
#include <yak/vm/address.h>

namespace yak::arch {

struct ThreadPcb {
  uint64_t ra;
  uint64_t sp;
  uint64_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
};

static_assert(offsetof(ThreadPcb, ra) == 0);
static_assert(offsetof(ThreadPcb, sp) == 8);
static_assert(offsetof(ThreadPcb, s0) == 16);
static_assert(offsetof(ThreadPcb, s1) == 24);
static_assert(offsetof(ThreadPcb, s2) == 32);
static_assert(offsetof(ThreadPcb, s3) == 40);
static_assert(offsetof(ThreadPcb, s4) == 48);
static_assert(offsetof(ThreadPcb, s5) == 56);
static_assert(offsetof(ThreadPcb, s6) == 64);
static_assert(offsetof(ThreadPcb, s7) == 72);
static_assert(offsetof(ThreadPcb, s8) == 80);
static_assert(offsetof(ThreadPcb, s9) == 88);
static_assert(offsetof(ThreadPcb, s10) == 96);
static_assert(offsetof(ThreadPcb, s11) == 104);

} // namespace yak::arch
