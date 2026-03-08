#include <cstddef>
#include <x86_64/asm.h>
#include <yak/log.h>

namespace yak {

constexpr int COM1 = 0x3F8;

static void early_putc(const char c) {
  while (!(asm_inb(COM1 + 5) & 0x20))
    asm volatile("pause");

  asm_outb(COM1, c);
}

void early_puts(const char *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (buf[i] == '\n') {
      early_putc('\r');
    }
    early_putc(buf[i]);
  }
}

} // namespace yak
