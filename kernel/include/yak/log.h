#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <yak/config.h>

namespace yak {

enum class LogLevel : uint8_t {
  Raw = 0,
  Debug,
  Trace,
  Info,
  Warn,
  Error,
  Fail,
};

namespace arch {
/**
 * @brief architecture-defined way to output characters during boot
 */
void early_puts(const char *buf, size_t len);
} // namespace arch

void vprintk(LogLevel level, const char *fmt, va_list args);

/**
 * @brief Implements logging for the kernel
 *
 * @param level loglevel
 * @param fmt printf-compatible format string
 */
[[gnu::format(printf, 2, 3)]]
void printk(LogLevel level, const char *fmt, ...);

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

// clang-format off
#if CONFIG_DEBUG
#define pr_debug(fmt, ...) yak::printk(yak::LogLevel::Debug, pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#else
#define pr_debug(...) do {} while(0)
#endif
#define pr_raw(fmt, ...)   yak::printk(yak::LogLevel::Raw,   pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#define pr_trace(fmt, ...) yak::printk(yak::LogLevel::Trace, pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#define pr_info(fmt, ...)  yak::printk(yak::LogLevel::Info,  pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#define pr_warn(fmt, ...)  yak::printk(yak::LogLevel::Warn,  pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#define pr_error(fmt, ...) yak::printk(yak::LogLevel::Error, pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
#define pr_fail(fmt, ...)  yak::printk(yak::LogLevel::Fail,  pr_fmt(fmt) __VA_OPT__(,) __VA_ARGS__)
// clang-format on

} // namespace yak
