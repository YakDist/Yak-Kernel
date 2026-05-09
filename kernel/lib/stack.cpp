#include <stdint.h>

extern "C" {
void __stack_chk_fail() {
  __builtin_trap();
}

[[gnu::used]]
uint64_t __stack_chk_guard;
}
