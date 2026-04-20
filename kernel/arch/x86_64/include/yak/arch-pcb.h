#pragma once

#include <stddef.h>
#include <stdint.h>
#include <yak/vm/address.h>

namespace yak::arch {

struct ThreadPcb {
  uint64_t rbx, rsp, rbp, r12, r13, r14, r15;
  
};

static_assert(offsetof(ThreadPcb, rbx) == 0);
static_assert(offsetof(ThreadPcb, rsp) == 8);
static_assert(offsetof(ThreadPcb, rbp) == 16);
static_assert(offsetof(ThreadPcb, r12) == 24);
static_assert(offsetof(ThreadPcb, r13) == 32);
static_assert(offsetof(ThreadPcb, r14) == 40);
static_assert(offsetof(ThreadPcb, r15) == 48);

} // namespace yak::arch
