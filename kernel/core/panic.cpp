#include <yak/log.h>
#include <yak/panic.h>

namespace yak {
[[noreturn]]
void panic(const char *msg) {
  pr_fail("PANIC: %s\n", msg);
  for (;;)
    asm volatile("cli; hlt");
}
} // namespace yak
