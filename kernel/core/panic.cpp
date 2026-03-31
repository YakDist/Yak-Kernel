#include <yak/log.h>
#include <yak/panic.h>

namespace yak {
void panic(const char *msg) {
  pr_fail("PANIC: %s\n", msg);
  for (;;)
    asm volatile("cli; hlt");
}
} // namespace yak
