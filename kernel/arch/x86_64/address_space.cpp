#include <cstdint>
#include <x86_64/asm.h>

namespace yak::arch {
enum {
  CR4_LA57_BIT = (1UL << 12),
};

static int get_va_bits() {
  uint64_t cr4 = asm_rdcr4();
  return (cr4 & CR4_LA57_BIT) ? 57 : 48;
}

bool is_canonical(uintptr_t addr) {
  static int va_bits = 0;
  if (!va_bits)
    va_bits = get_va_bits();

  int shift = va_bits - 1;
  uint64_t sign_ext = addr >> shift;
  return (sign_ext == 0 || sign_ext == UINT64_MAX >> shift);
}
} // namespace yak::arch
