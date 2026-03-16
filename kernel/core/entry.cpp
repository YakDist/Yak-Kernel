#include <cstddef>
#include <yak/arch.h>
#include <yak/bitset.h>
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

namespace arch {
bool is_canonical(uintptr_t addr);
}

extern "C" void kernel_entry() {
#if CONFIG_BOOT_BANNER
  pr_info("%s", banner);
#endif
  pr_info("Booting Yak v" KERNEL_VERSION_STR " (commit: " KERNEL_GIT_HASH
          ")\n");

  bsp_cpu_data.self = &bsp_cpu_data;

  arch::early_init();

  run_init_array();
  init_engine.run();

  Bitset<64> bset;
  bset.set(4);
  bset.set(8);

  bset.for_each_set_bit([](size_t bit) {
    pr_debug("bit set: %ld\n", bit);
    ;
  });

  pr_debug("canonical checks:\n");
  uint64_t addrs[] = {
      // 48-bit canonical
      0x0000'7FFF'FFFF'FFFF,
      0xFFFF'8000'0000'0000,
      0xFFFF'FFFF'FFFF'FFFF,
      0x0000'0000'0000'0000,
      // 48-bit non-canonical
      0x0000'8000'0000'0000,
      0xFFFF'7FFF'FFFF'FFFF,
  };

  for (auto &addr : addrs) {
    pr_debug("%lx => %s\n", addr, arch::is_canonical(addr) ? "yes" : "no");
  }

  pr_debug("End Of Kernel reached!\n");

  asm volatile("cli; hlt");
}

} // namespace yak
