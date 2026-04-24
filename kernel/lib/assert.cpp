#include <assert.h>
#include <nanoprintf.h>
#include <yak/panic.h>

namespace yak {

extern "C" [[gnu::noreturn]]
void __assert_fail(const char *assertion, const char *file, unsigned int line,
                   const char *function) {
  char buf[1024];
  npf_snprintf(buf, 1024, "Assertion failure: %s at %s:%u in %s\n", assertion,
               file, line, function);
  panic(buf);
}

} // namespace yak
