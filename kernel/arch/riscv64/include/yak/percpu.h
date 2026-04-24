#pragma once

#define CPUDATA_DECL [[gnu::section(".percpu.cpudata")]]
#define PERCPU_DECL  [[gnu::section(".percpu")]]

extern char __percpu_start[];
extern char __percpu_end[];

#define PERCPU_OFFSET(sym) (((uintptr_t) &(sym)) - (uintptr_t) &__percpu_start)

#define CPUDATA_OFFSET(field) (__builtin_offsetof(CpuData, field))

#define CPUDATA_TYPE(field) __typeof__(((CpuData *) 0)->field)

#define _PERCPU_SIZE_CHECK(T, macro_name)                             \
  static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || \
                    sizeof(T) == 8,                                   \
                macro_name ": field size must be 1, 2, 4, or 8 bytes")

#define _PERCPU_ATOMIC_SIZE_CHECK(T, macro_name)   \
  static_assert(                                   \
      sizeof(T) == 4 || sizeof(T) == 8, macro_name \
      ": atomic swap requires 4 or 8 byte field (RISC-V AMO limitation)")

#define CPUDATA_LOAD(field)                                       \
  ({                                                              \
    _PERCPU_SIZE_CHECK(CPUDATA_TYPE(field), "CPUDATA_LOAD");      \
    CPUDATA_TYPE(field) __value;                                  \
    constexpr uintptr_t __off = CPUDATA_OFFSET(field);            \
    switch (sizeof(CPUDATA_TYPE(field))) {                        \
    case 1:                                                       \
      asm volatile("lb %0, %1(tp)" : "=r"(__value) : "i"(__off)); \
      break;                                                      \
    case 2:                                                       \
      asm volatile("lh %0, %1(tp)" : "=r"(__value) : "i"(__off)); \
      break;                                                      \
    case 4:                                                       \
      asm volatile("lw %0, %1(tp)" : "=r"(__value) : "i"(__off)); \
      break;                                                      \
    case 8:                                                       \
      asm volatile("ld %0, %1(tp)" : "=r"(__value) : "i"(__off)); \
      break;                                                      \
    default:                                                      \
      __builtin_trap();                                           \
    }                                                             \
    __value;                                                      \
  })

#define CPUDATA_STORE(field, val)                                        \
  ({                                                                     \
    _PERCPU_SIZE_CHECK(CPUDATA_TYPE(field), "CPUDATA_STORE");            \
    CPUDATA_TYPE(field) __v = (val);                                     \
    constexpr uintptr_t __off = CPUDATA_OFFSET(field);                   \
    switch (sizeof(CPUDATA_TYPE(field))) {                               \
    case 1:                                                              \
      asm volatile("sb %0, %1(tp)" : : "r"(__v), "i"(__off) : "memory"); \
      break;                                                             \
    case 2:                                                              \
      asm volatile("sh %0, %1(tp)" : : "r"(__v), "i"(__off) : "memory"); \
      break;                                                             \
    case 4:                                                              \
      asm volatile("sw %0, %1(tp)" : : "r"(__v), "i"(__off) : "memory"); \
      break;                                                             \
    case 8:                                                              \
      asm volatile("sd %0, %1(tp)" : : "r"(__v), "i"(__off) : "memory"); \
      break;                                                             \
    default:                                                             \
      __builtin_trap();                                                  \
    }                                                                    \
    __v;                                                                 \
  })

#define CPUDATA_XCHG(field, val)                                    \
  ({                                                                \
    _PERCPU_ATOMIC_SIZE_CHECK(CPUDATA_TYPE(field), "CPUDATA_XCHG"); \
    CPUDATA_TYPE(field) __new_val = (val);                          \
    CPUDATA_TYPE(field) __old;                                      \
    constexpr uintptr_t __off = CPUDATA_OFFSET(field);              \
                                                                    \
    switch (sizeof(CPUDATA_TYPE(field))) {                          \
    case 4:                                                         \
      asm volatile("add  t0, tp, %2\n\t"                            \
                   "amoswap.w.aqrl %0, %1, (t0)"                    \
                   : "=r"(__old)                                    \
                   : "r"(__new_val), "r"(__off)                     \
                   : "t0", "memory");                               \
      break;                                                        \
    case 8:                                                         \
      asm volatile("add  t0, tp, %2\n\t"                            \
                   "amoswap.d.aqrl %0, %1, (t0)"                    \
                   : "=r"(__old)                                    \
                   : "r"(__new_val), "r"(__off)                     \
                   : "t0", "memory");                               \
      break;                                                        \
    default:                                                        \
      __builtin_trap();                                             \
    }                                                               \
    __old;                                                          \
  })

#define PERCPU_BASE() ((uintptr_t) CPUDATA_LOAD(self))

#define PERCPU_SIZE() \
  ((size_t) ((uintptr_t) __percpu_end - (uintptr_t) __percpu_start))

#define PERCPU_PTR(sym)                    \
  ({                                       \
    uintptr_t __base = PERCPU_BASE();      \
    uintptr_t __off = PERCPU_OFFSET(sym);  \
    (__typeof__(&(sym))) (__base + __off); \
  })

#define PERCPU_LOAD(sym)                                           \
  ({                                                               \
    _PERCPU_SIZE_CHECK(__typeof__(sym), "PERCPU_LOAD");            \
    __typeof__(sym) __value;                                       \
    uintptr_t __addr = (uintptr_t) PERCPU_PTR(sym);                \
    switch (sizeof(__value)) {                                     \
    case 1:                                                        \
      asm volatile("lb  %0, 0(%1)" : "=r"(__value) : "r"(__addr)); \
      break;                                                       \
    case 2:                                                        \
      asm volatile("lh  %0, 0(%1)" : "=r"(__value) : "r"(__addr)); \
      break;                                                       \
    case 4:                                                        \
      asm volatile("lw  %0, 0(%1)" : "=r"(__value) : "r"(__addr)); \
      break;                                                       \
    case 8:                                                        \
      asm volatile("ld  %0, 0(%1)" : "=r"(__value) : "r"(__addr)); \
      break;                                                       \
    }                                                              \
    __value;                                                       \
  })

#define PERCPU_STORE(sym, val)                                            \
  do {                                                                    \
    _PERCPU_SIZE_CHECK(__typeof__(sym), "PERCPU_STORE");                  \
    __typeof__(sym) __v = (val);                                          \
    uintptr_t __addr = (uintptr_t) PERCPU_PTR(sym);                       \
    switch (sizeof(__v)) {                                                \
    case 1:                                                               \
      asm volatile("sb  %0, 0(%1)" : : "r"(__v), "r"(__addr) : "memory"); \
      break;                                                              \
    case 2:                                                               \
      asm volatile("sh  %0, 0(%1)" : : "r"(__v), "r"(__addr) : "memory"); \
      break;                                                              \
    case 4:                                                               \
      asm volatile("sw  %0, 0(%1)" : : "r"(__v), "r"(__addr) : "memory"); \
      break;                                                              \
    case 8:                                                               \
      asm volatile("sd  %0, 0(%1)" : : "r"(__v), "r"(__addr) : "memory"); \
      break;                                                              \
    }                                                                     \
  } while (0)
