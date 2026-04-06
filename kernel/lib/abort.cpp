
#include <yak/panic.h>

extern "C" [[noreturn]] void abort() {
  yak::panic("abort!");
}
