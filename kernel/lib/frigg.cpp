#include <frg/macros.hpp>
#include <yak/log.h>
#include <yak/panic.h>

extern "C" {
void frg_log(const char *cstring) {
  pr_info("frigg: %s\n", cstring);
}

void frg_panic(const char *cstring) {
  yak::panic(cstring);
}
}
