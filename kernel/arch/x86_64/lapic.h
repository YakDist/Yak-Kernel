#pragma once

#include <cstdint>

namespace yak::arch::lapic {
void init_cpu();
void send_ipi(uint32_t lapic_id, uint8_t vector);
void eoi();
uint32_t id();
} // namespace yak::arch::lapic
