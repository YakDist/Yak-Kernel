#include "os_conf.h"
#include <cstddef>
#include <cstdio>
#include <print>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>
#include <yak/arch-mm.h>
#include <yak/arch-stack.h>
#include <yak/math.h>
#include <yak/size-macros.h>
#include <yak/vm/page.h>

namespace yak::arch {

size_t get_cpu_core_count() {
  size_t cores = std::thread::hardware_concurrency();

  // Fallback in case detection fails
  return (cores == 0) ? 1 : cores;
}

extern "C" void kernel_entry(void *bsp_idle_stack_top);

[[noreturn]]
void switch_stack_and_jmp(void *new_stack_top, void (*func)(void *)) {
#if defined(__x86_64__)

  asm volatile("mov %0, %%rsp\n" // switch stack
               "xor %%ebp, %%ebp\n"
               "mov %%rsp, %%rdi\n"
               "call *%1\n"
               :
               : "r"(new_stack_top), "r"(func)
               : "rsp", "rbp");

  __builtin_unreachable();

#else
#error Unsupported architecture
#endif
}

size_t linux_vcpu_count = -1;
size_t linux_pmem_count = MiB(256);
size_t PAGE_SIZE;
unsigned int PAGE_SHIFT;
size_t KSTACK_SIZE;

vaddr_t PFNDB_BASE;
size_t PFNDB_SIZE;

vaddr_t HHDM_BASE;

} // namespace yak::arch

int main() {
  using namespace yak;
  using namespace yak::arch;

  PAGE_SIZE = getpagesize();
  PAGE_SHIFT = std::countr_zero(PAGE_SIZE);
  std::println("page size {} and page shift {}", PAGE_SIZE, PAGE_SHIFT);
  KSTACK_SIZE = PAGE_SIZE * 32;

  linux_vcpu_count = get_cpu_core_count();

  std::println("running on {} cores", linux_vcpu_count);
  std::println("setting up {}MB of physical memory",
               linux_pmem_count / 1024 / 1024);

  int mfd = memfd_create("physical memory", 0);
  if (mfd < 0) {
    perror("could not setup memfd\n");
    return EXIT_FAILURE;
  }
  ftruncate(mfd, linux_pmem_count);

  void *ret =
      mmap(0, linux_pmem_count, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (ret == MAP_FAILED) {
    perror("could not reserve hhdm space");
    return EXIT_FAILURE;
  }

  HHDM_BASE = (vaddr_t) ret;

  if (mmap((void *) HHDM_BASE, linux_pmem_count, PROT_READ | PROT_WRITE,
           MAP_FIXED | MAP_SHARED, mfd, 0) == MAP_FAILED) {

    perror("failed to map memfd page to hhdm");
    return EXIT_FAILURE;
  }

  PFNDB_SIZE = linux_pmem_count / PAGE_SIZE * sizeof(Page);
  PFNDB_SIZE = align_up(PFNDB_SIZE, PAGE_SIZE);

  ret = mmap(0, PFNDB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
             -1, 0);
  if (ret == MAP_FAILED) {
    perror("could not map pfndb");
    return EXIT_FAILURE;
  }

  PFNDB_BASE = (vaddr_t) ret;

  void *stack_ptr;
  int rc = posix_memalign(&stack_ptr, PAGE_SIZE, KSTACK_SIZE);
  if (rc != 0) {
    perror("posix memalign failed");
    return EXIT_FAILURE;
  }

  vaddr_t stack_bot = (vaddr_t) stack_ptr;
  void *stack_top = reinterpret_cast<void *>((stack_bot + KSTACK_SIZE));

  switch_stack_and_jmp(stack_top, kernel_entry);

  return 0;
}
