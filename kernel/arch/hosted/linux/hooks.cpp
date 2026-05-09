#include "os_conf.h"
#include <asm/prctl.h>
#include <cstdio>
#include <cstdlib>
#include <sys/syscall.h>
#include <unistd.h>
#include <yak/arch.h>
#include <yak/percpu.h>
#include <yak/vm/memblock.h>

namespace yak::arch {
void early_init() {}

void mem_init() {
  boot_memblock.memory.add(0, linux_pmem_count, 0);
  boot_memblock.usable.add(0, linux_pmem_count, 0);
}
void boot_finalize() {}
void post_pmm() {}
} // namespace yak::arch
