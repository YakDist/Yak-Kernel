#pragma once

// define an ASSUME(...) function-style macro so we only need to detect
// compilers in one place

// Comment this out if you don't want assumptions to possibly evaluate.
// This may happen for implementations based on unreachable() functions.entry
#define DANGEROUS_BEHAVIOR_ASSUMPTIONS_ALLOWED_TO_EVALUATE 1

// preferred option: C++ standard attribute
#ifdef __has_cpp_attribute
#if __has_cpp_attribute(assume) >= 202207L
#define ASSUME(...) [[assume(__VA_ARGS__)]]
#endif
#endif
// first fallback: compiler intrinsics/attributes for assumptions
#ifndef ASSUME
#if defined(__clang__)
#define ASSUME(...)                                                            \
  do {                                                                         \
    __builtin_assume(__VA_ARGS__);                                             \
  } while (0)
#elif defined(_MSC_VER)
#define ASSUME(...)                                                            \
  do {                                                                         \
    __assume(__VA_ARGS__);                                                     \
  } while (0)
#elif defined(__GNUC__)
#if __GNUC__ >= 13
#define ASSUME(...) __attribute__((__assume__(__VA_ARGS__)))
#endif
#endif
#endif
// second fallback: possibly evaluating uses of unreachable()
#if !defined(ASSUME) &&                                                        \
    defined(DANGEROUS_BEHAVIOR_ASSUMPTIONS_ALLOWED_TO_EVALUATE)
#if defined(__GNUC__)
#define ASSUME(...)                                                            \
  do {                                                                         \
    if (!bool(__VA_ARGS__))                                                    \
      __builtin_unreachable();                                                 \
  } while (0)
#elif __cpp_lib_unreachable >= 202202L
#include <utility>
#define ASSUME(...)                                                            \
  do {                                                                         \
    if (!bool(__VA_ARGS__))                                                    \
      ::std::unreachable(); ) while(0)
#endif
#endif
// last fallback: define macro as doing nothing
#ifndef ASSUME
#define ASSUME(...)
#endif
