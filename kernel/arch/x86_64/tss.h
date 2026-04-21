#pragma once

#include <stddef.h>
#include <stdint.h>

namespace yak::arch {
struct [[gnu::packed]] Tss {
  uint32_t unused0;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t unused1;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t unused2;
  uint32_t iopb;
};

void tss_cpu_init();

static_assert(offsetof(Tss, unused0) == 0, "Tss::unused0 offset mismatch");
static_assert(offsetof(Tss, rsp0) == 4, "Tss::rsp0 offset mismatch");
static_assert(offsetof(Tss, rsp1) == 12, "Tss::rsp1 offset mismatch");
static_assert(offsetof(Tss, rsp2) == 20, "Tss::rsp2 offset mismatch");
static_assert(offsetof(Tss, unused1) == 28, "Tss::unused1 offset mismatch");
static_assert(offsetof(Tss, ist1) == 36, "Tss::ist1 offset mismatch");
static_assert(offsetof(Tss, ist2) == 44, "Tss::ist2 offset mismatch");
static_assert(offsetof(Tss, ist3) == 52, "Tss::ist3 offset mismatch");
static_assert(offsetof(Tss, ist4) == 60, "Tss::ist4 offset mismatch");
static_assert(offsetof(Tss, ist5) == 68, "Tss::ist5 offset mismatch");
static_assert(offsetof(Tss, ist6) == 76, "Tss::ist6 offset mismatch");
static_assert(offsetof(Tss, ist7) == 84, "Tss::ist7 offset mismatch");
static_assert(offsetof(Tss, unused2) == 92, "Tss::unused2 offset mismatch");
static_assert(offsetof(Tss, iopb) == 100, "Tss::iopb offset mismatch");
static_assert(sizeof(Tss) == 104, "Tss size mismatch");
} // namespace yak::arch
