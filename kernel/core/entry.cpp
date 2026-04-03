#include <cstddef>
#include <yak/arch.h>
#include <yak/config.h>
#include <yak/cpudata.h>
#include <yak/init.h>
#include <yak/log.h>
#include <yak/math.h>
#include <yak/percpu.h>
#include <yak/version.h>
#include <yak/vm/memblock.h>
#include <yak/vm/page.h>
#include <yak/vm/pmm.h>

namespace yak {

constinit GlobalInitEngine init_engine;
CPUDATA_DECL CpuData bsp_cpu_data;

/* Declared in the linker script */
extern "C" void (*__init_array_start[])();
extern "C" void (*__init_array_end[])();

void run_init_array() {
  size_t count = __init_array_end - __init_array_start;

  pr_debug("running %zu constructors\n", count);

  for (size_t i = 0; i < count; i++) {
    auto constructor = __init_array_start[i];
    constructor();
  }
}

#if CONFIG_BOOT_BANNER
const char banner[] = {
#embed "banner.txt" if_empty()
};
#endif

namespace arch {
bool is_canonical(uintptr_t addr);
}

extern "C" void kernel_entry() {
#if CONFIG_BOOT_BANNER
  pr_info("%s", banner);
#endif
  pr_info("Booting Yak/" ARCH_STR " v" KERNEL_VERSION_STR
          " (commit: " KERNEL_GIT_HASH ")\n");

  bsp_cpu_data.self = &bsp_cpu_data;

  arch::early_init();

  run_init_array();

  arch::mem_init();

  arch::boot_finalize();

  boot_memblock.done();

  // XXX: rather run this on the kmain thread!
  init_engine.run();

  panic("End Of Kernel reached!\n");

  asm volatile("cli; hlt");
}

} // namespace yak
