#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS   1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS       0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS       1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS       1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS      1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS   1
#define NANOPRINTF_USE_ALT_FORM_FLAG                 1
#define NANOPRINTF_VISIBILITY_STATIC                 1
#include <nanoprintf.h>

#include <cstdarg>
#include <cstring>
#include <frg/mutex.hpp>
#include <yak/log.h>
#include <yak/spinlock.h>

namespace yak {

static bool printk_available = false;

static constinit InterruptSpinLock early_printk_lock;

char early_buf[4096];
constinit size_t early_buf_pos = 0;

[[gnu::format(printf, 1, 0)]]
static void printk_early(const char *fmt, va_list args) {
  auto guard = frg::guard(&early_printk_lock);

  size_t remaining = sizeof(early_buf) - early_buf_pos;
  if (remaining <= 1) {
    early_buf_pos = 0;
    remaining = sizeof(early_buf);
  }

  int written = npf_vsnprintf(early_buf + early_buf_pos, remaining, fmt, args);

  if (written > 0) {
    size_t actual_len =
        ((size_t) written >= remaining) ? (remaining - 1) : (size_t) written;

    early_puts(early_buf + early_buf_pos, actual_len);

    early_buf_pos += actual_len;

    early_buf[early_buf_pos] = '\0';
  }

  if (early_buf_pos > sizeof(early_buf) - 64) {
    early_buf_pos = 0;
  }
}

[[gnu::format(printf, 2, 0)]]
void vprintk(LogLevel level, const char *fmt, va_list args) {}

[[gnu::format(printf, 2, 3)]]
void printk(LogLevel level, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (!printk_available) {
    printk_early(fmt, args);
    return;
  }

  va_end(args);
}

} // namespace yak
