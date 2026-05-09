#include <cstdio>
#include <iostream>
#include <yak/arch.h>

namespace yak::arch {
void early_puts(const char *buf, size_t len) {
  std::cout.write(buf, len);
}

void early_output_init() {}
} // namespace yak::arch
