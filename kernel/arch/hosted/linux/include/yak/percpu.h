#pragma once

#define CPUDATA_DECL __thread
#define PERCPU_DECL  __thread

namespace yak {
struct CpuData;
extern __thread CpuData bsp_cpu_data;
} // namespace yak

#define _PERCPU_SIZE_CHECK(T, macro_name)                             \
  static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || \
                    sizeof(T) == 8,                                   \
                macro_name ": field size must be 1, 2, 4, or 8 bytes")

#define CPUDATA_LOAD(field)                                 \
  ({                                                        \
    _PERCPU_SIZE_CHECK(bsp_cpu_data.field, "CPUDATA_LOAD"); \
    bsp_cpu_data.field;                                     \
  })

#define CPUDATA_STORE(field, val)                            \
  ({                                                         \
    _PERCPU_SIZE_CHECK(bsp_cpu_data.field, "CPUDATA_STORE"); \
    bsp_cpu_data.field = (val);                              \
  })

#define CPUDATA_XCHG(field, val)                                       \
  ({                                                                   \
    _PERCPU_SIZE_CHECK(bsp_cpu_data.field, "CPUDATA_XCHG");            \
    __atomic_exchange_n(&bsp_cpu_data.field, (val), __ATOMIC_SEQ_CST); \
  })

#define PERCPU_PTR(sym) (&(sym))

#define PERCPU_LOAD(sym)                    \
  ({                                        \
    _PERCPU_SIZE_CHECK(sym, "PERCPU_LOAD"); \
    (sym);                                  \
  })

#define PERCPU_STORE(sym, val)               \
  do {                                       \
    _PERCPU_SIZE_CHECK(sym, "PERCPU_STORE"); \
    (sym) = (val);                           \
  } while (0)
