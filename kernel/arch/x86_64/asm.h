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
