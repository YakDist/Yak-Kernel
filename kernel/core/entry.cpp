#include <cstddef>
#include <yak/initgraph.h>
#include <yak/log.h>
#include <yak/version.h>
#include <yak/config.h>

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

#if CONFIG_BOOT_BANNER
const char banner[] = {
#embed "banner.txt" if_empty()
};
#endif

extern "C" void kernel_entry() {
#if CONFIG_BOOT_BANNER
  pr_info("%s", banner);
#endif
  pr_info("Booting Yak v" KERNEL_VERSION_STR " (commit: " KERNEL_GIT_HASH
          ")\n");
  run_init_array();
  asm volatile("cli; hlt");
}

} // namespace yak
