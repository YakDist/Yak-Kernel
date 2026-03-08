#include <cstddef>
#include <yak/log.h>

namespace yak {

/* Declared in the linker script */
extern "C" void (*__init_array_start[])(void);
extern "C" void (*__init_array_end[])(void);

void run_init_array() {
  size_t count = __init_array_end - __init_array_start;

  for (size_t i = 0; i < count; i++) {
    __init_array_start[i]();
  }
}

extern "C" void kernel_entry() {
  printk_early("Booting Yak v%d.%d.%d\n", 1, 0, 0);
  run_init_array();
  asm volatile("cli; hlt");
}

} // namespace yak
