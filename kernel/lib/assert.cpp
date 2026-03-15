#include <assert.h>
#include <yak/log.h>

extern "C" [[gnu::noreturn]]
void __assert_fail(const char *assertion, const char *file, unsigned int line,
                   const char *function) {
  pr_error("Assertion failure: %s at %s:%u in %s\n", assertion, file, line,
           function);
  for (;;)
    asm volatile("cli; hlt");
}
