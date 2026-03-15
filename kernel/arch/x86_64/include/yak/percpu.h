#pragma once

#define CPUDATA_DECL [[gnu::section(".percpu.cpudata")]]
#define PERCPU_DECL [[gnu::section(".percpu")]]

extern char __percpu_start[];

#define PERCPU_OFFSET(sym) (((uintptr_t)&(sym)) - (uintptr_t)&__percpu_start)

#define CPUDATA_OFFSET(field) (__builtin_offsetof(CpuData, field))

#define CPUDATA_TYPE(field) __typeof__(((CpuData *)0)->field)

#define CPUDATA_LOAD(field)                                                    \
  ({                                                                           \
    CPUDATA_TYPE(field) __value;                                               \
    switch (sizeof(CPUDATA_TYPE(field))) {                                     \
    case 1:                                                                    \
    case 2:                                                                    \
    case 4:                                                                    \
    case 8:                                                                    \
      asm volatile("mov %%gs:%c1, %0"                                          \
                   : "=r"(__value)                                             \
                   : "i"(CPUDATA_OFFSET(field)));                              \
      break;                                                                   \
    default:                                                                   \
      __builtin_trap();                                                        \
    }                                                                          \
    __value;                                                                   \
  })

#define CPUDATA_STORE(field, val)                                              \
  ({                                                                           \
    CPUDATA_TYPE(field) __v = (val);                                           \
    switch (sizeof(CPUDATA_TYPE(field))) {                                     \
    case 1:                                                                    \
    case 2:                                                                    \
    case 4:                                                                    \
    case 8:                                                                    \
      asm volatile("mov %0, %%gs:%c1"                                          \
                   :                                                           \
                   : "r"(__v), "i"(CPUDATA_OFFSET(field))                      \
                   : "memory");                                                \
      break;                                                                   \
    default:                                                                   \
      __builtin_trap();                                                        \
    }                                                                          \
  })

#define CPUDATA_XCHG(field, val)                                               \
  ({                                                                           \
    CPUDATA_TYPE(field) __old = (val);                                         \
    switch (sizeof(CPUDATA_TYPE(field))) {                                     \
    case 1:                                                                    \
    case 2:                                                                    \
    case 4:                                                                    \
    case 8:                                                                    \
      asm volatile("xchg %0, %%gs:%c1"                                         \
                   : "+r"(__old)                                               \
                   : "i"(CPUDATA_OFFSET(field))                                \
                   : "memory");                                                \
      break;                                                                   \
    default:                                                                   \
      __builtin_trap();                                                        \
    }                                                                          \
    __old;                                                                     \
  })

#define PERCPU_BASE() ((uintptr_t)CPUDATA_LOAD(self))

#define PERCPU_PTR(sym)                                                        \
  ({                                                                           \
    uintptr_t __base = PERCPU_BASE();                                          \
    uintptr_t __off = PERCPU_OFFSET(sym);                                      \
    (__typeof__(&(sym)))(__base + __off);                                      \
  })

#define PERCPU_LOAD(sym)                                                       \
  ({                                                                           \
    __typeof__(sym) __value;                                                   \
    switch (sizeof(__value)) {                                                 \
    case 1:                                                                    \
    case 2:                                                                    \
    case 4:                                                                    \
    case 8:                                                                    \
      asm volatile("mov %%gs:(%1), %0"                                         \
                   : "=r"(__value)                                             \
                   : "r"(PERCPU_OFFSET(sym)));                                 \
      break;                                                                   \
    default:                                                                   \
      __builtin_trap();                                                        \
    }                                                                          \
    __value;                                                                   \
  })

#define PERCPU_STORE(sym, val)                                                 \
  do {                                                                         \
    __typeof__(sym) __v = (val);                                               \
    switch (sizeof(__v)) {                                                     \
    case 1:                                                                    \
    case 2:                                                                    \
    case 4:                                                                    \
    case 8:                                                                    \
      asm volatile("mov %0, %%gs:(%1)"                                         \
                   :                                                           \
                   : "r"(__v), "r"(PERCPU_OFFSET(sym))                         \
                   : "memory");                                                \
      break;                                                                   \
    default:                                                                   \
      __builtin_trap();                                                        \
    }                                                                          \
  } while (0)
