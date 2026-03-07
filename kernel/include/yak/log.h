#pragma once

#include <cstdarg>
#include <cstdint>

enum class LogLevel : uint16_t {
  Debug = 1,
  Trace,
  Info,
  Warn,
  Error,
  Fail,
};

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
