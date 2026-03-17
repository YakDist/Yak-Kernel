#pragma once

#include <cstdint>

static inline void asm_outb(uint16_t port, uint8_t data) {
  asm volatile("out %%al, %%dx" : : "a"(data), "d"(port));
}

static inline void asm_outw(uint16_t port, uint16_t data) {
  asm volatile("out %%ax, %%dx" : : "a"(data), "d"(port));
}

static inline void asm_outl(uint16_t port, uint32_t data) {
  asm volatile("out %%eax, %%dx" : : "a"(data), "d"(port));
}

static inline uint8_t asm_inb(uint16_t port) {
  uint8_t ret;
  asm volatile("in %%dx, %%al" : "=a"(ret) : "d"(port));
  return ret;
}

static inline uint16_t asm_inw(uint16_t port) {
  uint16_t ret;
  asm volatile("in %%dx, %%ax" : "=a"(ret) : "d"(port));
  return ret;
}

static inline uint32_t asm_inl(uint32_t port) {
  uint32_t ret;
  asm volatile("in %%dx, %%eax" : "=a"(ret) : "d"(port));
  return ret;
}

static inline void asm_wrmsr(uint32_t msr, uint64_t value) {
  uint32_t low = (uint32_t)value;
  uint32_t high = (uint32_t)(value >> 32);
  asm volatile("wrmsr" ::"c"(msr), "a"(low), "d"(high) : "memory");
}

static inline uint64_t asm_rdmsr(uint32_t msr) {
  uint32_t low, high;
  asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr) : "memory");
  return ((uint64_t)high << 32) | (uint64_t)low;
}

static inline void asm_cpuid(int leaf, int subleaf, uint32_t *eax,
                             uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
  asm volatile("cpuid"
               : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
               : "a"(leaf), "c"(subleaf));
}

#define FN_CR(REG)                                                             \
  static inline uint64_t asm_rdcr##REG() {                                     \
    uint64_t data;                                                             \
    asm volatile("mov %%cr" #REG ", %0" : "=r"(data));                         \
    return data;                                                               \
  }                                                                            \
  static inline void asm_wrcr##REG(uint64_t val) {                             \
    asm volatile("mov %0, %%cr" #REG ::"a"(val));                              \
  }

FN_CR(0);
FN_CR(1);
FN_CR(2);
FN_CR(3);
FN_CR(4);
FN_CR(8);
