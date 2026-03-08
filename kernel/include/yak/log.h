#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>

namespace yak {

#define CONFIG_LOG_TIMESTAMPS 1
#undef CONFIG_LOG_TIMESTAMPS

enum class LogLevel : uint8_t {
  Debug = 1,
  Trace,
  Info,
  Warn,
  Error,
  Fail,
};

struct LogRecord {
#if CONFIG_LOG_TIMESTAMPS
  // TODO: implement log timestamps
#endif
  LogLevel level;
};

/**
 * @brief architecture-defined way to characters during boot
 */
void early_puts(const char *buf, size_t len);

/**
 * @brief printk implementation used during boot
 *
 * This function writes to a static temporary buffer and immediatly flushes
 * to the early console.
 *
 * WARNING: this function is NOT thread-safe. Logs during an IRQ WILL be
 * silently dropped if logging was interrupted.
 */
[[gnu::format(printf, 1, 2)]]
void printk_early(const char *fmt, ...);

void vprintk(LogLevel level, const char *fmt, va_list args);

[[gnu::format(printf, 2, 3)]]
void printk(LogLevel level, const char *fmt, ...);

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

// clang-format off
#define pr_debug(fmt, ...) printk(LogLevel::Debug, pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#define pr_trace(fmt, ...) printk(LogLevel::Trace, pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#define pr_info(fmt, ...)  printk(LogLevel::Info,  pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#define pr_warn(fmt, ...)  printk(LogLevel::Warn,  pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#define pr_error(fmt, ...) printk(LogLevel::Error, pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#define pr_fail(fmt, ...)  printk(LogLevel::Fail,  pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
// clang-format on

} // namespace yak
