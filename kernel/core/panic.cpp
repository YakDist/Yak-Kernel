#include <yak/arch-cpu.h>
#include <yak/log.h>
#include <yak/math.h>
#include <yak/panic.h>
#include <yak/vm/address.h>

namespace yak {

struct StackFrame {
  StackFrame *next_fp;
  uintptr_t return_address;
};

static void print_stack_trace() {
  StackFrame *fp = reinterpret_cast<StackFrame *>(__builtin_frame_address(0));
  pr_error("Stack trace:\n");
  int frame_num = 0;
  while (fp != NULL) {
    pr_error("#%d %#lx\n", frame_num++, fp->return_address);

    fp = fp->next_fp;
    if (!is_aligned_pow2<8>((vaddr_t) fp))
      break;

    if ((vaddr_t) fp == 0)
      break;
  }
}

[[noreturn]]
void panic(const char *msg) {
  pr_fail("PANIC: %s\n", msg);
  print_stack_trace();
  for (;;)
    arch::hcf();
}
} // namespace yak
