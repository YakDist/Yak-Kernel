#include <cstddef>
#include <yak/log.h>

namespace yak {

static void sbi_putc(const char c) {
  register long a0 asm("a0") = c;
  register long a6 asm("a6") = 0; // SBI_EXT_0_1_CONSOLE_PUTCHAR
  register long a7 asm("a7") = 0x01;
  asm volatile("ecall" : "+r"(a0) : "r"(a6), "r"(a7) : "memory");
}

void early_puts(const char *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (buf[i] == '\n') {
      sbi_putc('\r');
    }
    sbi_putc(buf[i]);
  }
}

} // namespace yak
