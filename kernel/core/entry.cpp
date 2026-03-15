#include <cstddef>
#include <yak/arch.h>
#include <yak/config.h>
#include <yak/cpudata.h>
#include <yak/init.h>
#include <yak/log.h>
#include <yak/percpu.h>
#include <yak/version.h>

namespace yak {

GlobalInitEngine init_engine;
CPUDATA_DECL CpuData bsp_cpu_data;

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

  bsp_cpu_data.self = &bsp_cpu_data;

  arch::early_init();

  run_init_array();

  pr_debug("End Of Kernel reached!\n");

  asm volatile("cli; hlt");
}

} // namespace yak
