#include <assert.h>

extern "C" [[gnu::noreturn]]
void __assert_fail(const char *assertion, const char *file, unsigned int line,
                   const char *function) {
  asm volatile("cli; hlt");
}
