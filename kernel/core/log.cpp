#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_ALT_FORM_FLAG 1
#define NANOPRINTF_VISIBILITY_STATIC 1
#include <nanoprintf.h>

#include <cstdarg>
#include <cstring>
#include <yak/log.h>

#include <x86_64/asm.h>

static constinit char early_buf[4096] = {0};
static constinit size_t early_buf_pos = 0;

constexpr int COM1 = 0x3F8;

void early_putc(const char c) {
  while (!(asm_inb(COM1 + 5) & 0x20))
    asm volatile("pause");

  asm_outb(COM1, c);
}

void early_puts(const char *buf) {
  for (size_t i = 0; buf[i] != '\0'; i++) {
    if (buf[i] == '\n') {
      early_putc('\r');
    }
    early_putc(buf[i]);
  }
}

void printk_early(const char *fmt, ...) {

  va_list args;
  va_start(args, fmt);

  int written = npf_vsnprintf(early_buf + early_buf_pos,
                              sizeof(early_buf) - early_buf_pos, fmt, args);

  va_end(args);

  if (written > 0) {
    early_puts(early_buf + early_buf_pos);

    early_buf_pos += (size_t)written;
    if (early_buf_pos >= sizeof(early_buf)) {
      early_buf_pos = 0;
      memset(early_buf, 0, sizeof(early_buf));
    }

    early_buf[early_buf_pos] = '\0';
  }
}

void vprintk(LogLevel level, const char *fmt, va_list args) {}

[[gnu::format(printf, 2, 3)]]
void printk(LogLevel level, const char *fmt, ...) {}
