#include <yak/arch-cpu.h>
#include <yak/log.h>
#include <yak/panic.h>

namespace yak {
[[noreturn]]
void panic(const char *msg) {
  pr_fail("PANIC: %s\n", msg);
  for (;;)
    arch::hcf();
}
} // namespace yak
