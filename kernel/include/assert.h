#pragma once

#include <yak/config.h>

#ifdef __cplusplus
extern "C" {
#endif

[[gnu::noreturn]]
void __assert_fail(const char *assertion, const char *file, unsigned int line,
                   const char *function);

#ifdef __cplusplus
}
#endif

#if CONFIG_DEBUG
#define assert(ignore) ((void)0)
#else

#undef assert
// clang-format off
#define assert(expr) \
  do { \
    if (__builtin_expect(!(bool)(expr), 0)) \
      __assert_fail(#expr, __FILE__, __LINE__, __func__); \
  } while (0)
// clang-format on

#endif
