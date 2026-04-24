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

#define CPUDATA_LOAD(field)                                  \
  ({                                                         \
    _PERCPU_SIZE_CHECK(CPUDATA_TYPE(field), "CPUDATA_LOAD"); \
    CPUDATA_TYPE(field) __value;                             \
    switch (sizeof(CPUDATA_TYPE(field))) {                   \
    case 1:                                                  \
    case 2:                                                  \
    case 4:                                                  \
    case 8:                                                  \
      asm volatile("mov %%gs:%c1, %0"                        \
                   : "=r"(__value)                           \
                   : "i"(CPUDATA_OFFSET(field)));            \
      break;                                                 \
    }                                                        \
    __value;                                                 \
  })

#define CPUDATA_STORE(field, val)                             \
  ({                                                          \
    _PERCPU_SIZE_CHECK(CPUDATA_TYPE(field), "CPUDATA_STORE"); \
    CPUDATA_TYPE(field) __v = (val);                          \
    switch (sizeof(CPUDATA_TYPE(field))) {                    \
    case 1:                                                   \
    case 2:                                                   \
    case 4:                                                   \
    case 8:                                                   \
      asm volatile("mov %0, %%gs:%c1"                         \
                   :                                          \
                   : "r"(__v), "i"(CPUDATA_OFFSET(field))     \
                   : "memory");                               \
      break;                                                  \
    }                                                         \
  })

#define CPUDATA_XCHG(field, val)                             \
  ({                                                         \
    _PERCPU_SIZE_CHECK(CPUDATA_TYPE(field), "CPUDATA_XCHG"); \
    CPUDATA_TYPE(field) __old = (val);                       \
    switch (sizeof(CPUDATA_TYPE(field))) {                   \
    case 1:                                                  \
    case 2:                                                  \
    case 4:                                                  \
    case 8:                                                  \
      asm volatile("xchg %0, %%gs:%c1"                       \
                   : "+r"(__old)                             \
                   : "i"(CPUDATA_OFFSET(field))              \
                   : "memory");                              \
      break;                                                 \
    }                                                        \
    __old;                                                   \
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

#define PERCPU_LOAD(sym)                                    \
  ({                                                        \
    _PERCPU_SIZE_CHECK(CPUDATA_TYPE(field), "PERCPU_LOAD"); \
    __typeof__(sym) __value;                                \
    switch (sizeof(__value)) {                              \
    case 1:                                                 \
    case 2:                                                 \
    case 4:                                                 \
    case 8:                                                 \
      asm volatile("mov %%gs:(%1), %0"                      \
                   : "=r"(__value)                          \
                   : "r"(PERCPU_OFFSET(sym)));              \
      break;                                                \
    }                                                       \
    __value;                                                \
  })

#define PERCPU_STORE(sym, val)                               \
  do {                                                       \
    _PERCPU_SIZE_CHECK(CPUDATA_TYPE(field), "PERCPU_STORE"); \
    __typeof__(sym) __v = (val);                             \
    switch (sizeof(__v)) {                                   \
    case 1:                                                  \
    case 2:                                                  \
    case 4:                                                  \
    case 8:                                                  \
      asm volatile("mov %0, %%gs:(%1)"                       \
                   :                                         \
                   : "r"(__v), "r"(PERCPU_OFFSET(sym))       \
                   : "memory");                              \
      break;                                                 \
    }                                                        \
  } while (0)
