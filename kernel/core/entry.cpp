#include <cstddef>
#include <yak/arch.h>
#include <yak/config.h>
#include <yak/cpudata.h>
#include <yak/init.h>
#include <yak/log.h>
#include <yak/math.h>
#include <yak/percpu.h>
#include <yak/ps.h>
#include <yak/sched.h>
#include <yak/thread.h>
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
const char boot_banner[] = {
#embed "banner.txt" if_empty()
};
#endif

namespace arch {
bool is_canonical(uintptr_t addr);
}

extern void test_fn();

Thread bsp_idle_thread =
    Thread("idle_thread0", SchedPrio::Idle, &kernel_process, false);

extern "C" void kernel_entry(void *bsp_idle_stack_top) {
  printk(LogLevel::Raw, "\e[0m");

#if CONFIG_BOOT_BANNER
  pr_info("%s", boot_banner);
#endif
  pr_info("Booting Yak/" ARCH_STR " v" KERNEL_VERSION_STR
          " (commit: " KERNEL_GIT_HASH ")\n");

  run_init_array();

  CpuData::initialize(&bsp_cpu_data);
  bsp_cpu_data.bsp = true;

  arch::early_init();

  bsp_idle_thread.kernel_stack_top = bsp_idle_stack_top;
  Scheduler::init(&bsp_cpu_data, &bsp_idle_thread);

  // We now have defined thread-state;
  // Sleeping locks are usable.

  arch::mem_init();

  arch::boot_finalize();

  boot_memblock.done();

  arch::post_pmm();

  auto t = Thread("test", SchedPrio::RealTime, &kernel_process, false);
  const size_t size = 4096;
  [[gnu::aligned(16)]]
  static char buf[size];
  t.init_context((void *) &buf[size],
                 [](void *, void *) {
                   pr_debug("enter!\n");
                   ;
                 },
                 nullptr, nullptr);
  Scheduler::for_this_cpu().resume(&t);

  // XXX: rather run this on the kmain thread!
  init_engine.run();

  bsp_cpu_data.sched->idle_loop();
}

} // namespace yak
