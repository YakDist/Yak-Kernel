#include <cstdint>
#include <x86_64/adress_space.h>
#include <x86_64/gdt.h>
#include <yak/cpudata.h>

namespace yak::arch {

struct TrapFrame {
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t rbp;

  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t user_rsp;
  uint64_t ss;
};

[[gnu::noreturn]]
void syscall_iret_return(TrapFrame *tf) {
  asm volatile("cli\n\t"
               "mov %0, %%rsp\n\t"
               // restore GPRs
               "pop %%rax\n\t"
               "pop %%rbx\n\t"
               "pop %%rcx\n\t"
               "pop %%rdx\n\t"
               "pop %%rdi\n\t"
               "pop %%rsi\n\t"
               "pop %%r8\n\t"
               "pop %%r9\n\t"
               "pop %%r10\n\t"
               "pop %%r11\n\t"
               "pop %%r12\n\t"
               "pop %%r13\n\t"
               "pop %%r14\n\t"
               "pop %%r15\n\t"
               "pop %%rbp\n\t"

               "swapgs\n\t"
               "iretq\n\t"
               //
               ::"r"(tf)
               : "memory");

  __builtin_unreachable();
}

[[gnu::noreturn]]
void syscall_sysret_return(TrapFrame *tf) {
  asm volatile("cli\n\t"
               "mov %0, %%rsp\n\t"
               // restore GPRs
               "pop %%rax\n\t"
               "pop %%rbx\n\t"
               "pop %%rcx\n\t" // will be overwritten below
               "pop %%rdx\n\t"
               "pop %%rdi\n\t"
               "pop %%rsi\n\t"
               "pop %%r8\n\t"
               "pop %%r9\n\t"
               "pop %%r10\n\t"
               "pop %%r11\n\t" // will be overwritten below
               "pop %%r12\n\t"
               "pop %%r13\n\t"
               "pop %%r14\n\t"
               "pop %%r15\n\t"
               "pop %%rbp\n\t"
               // rsp now points at rip in the iretq frame
               "mov (%%rsp), %%rcx\n\t"   // RIP
               "mov 16(%%rsp), %%r11\n\t" // RFLAGS (skip CS at +8)
               "mov 24(%%rsp), %%rsp\n\t" // user_rsp
               "swapgs\n\t"
               "sysretq\n\t" ::"r"(tf)
               : "memory");

  __builtin_unreachable();
}

extern "C" [[gnu::used]]
void syscall_handler(TrapFrame *tf) {
  // RIP must be canonical, as sysretq with a non-canonical RIP faults in ring0,
  // causing a GPF with user RSP loaded!
  if (!is_canonical(tf->rip)) {
    return syscall_iret_return(tf);
  }

  // CS and SS must be the standard selectors, as sysretq retrieves the
  // selectors from the STAR MSR
  if (tf->cs != GDT_SEL_USER_CODE || tf->ss != GDT_SEL_USER_DATA) {
    return syscall_iret_return(tf);
  }

  return syscall_sysret_return(tf);
}

[[gnu::naked]]
void syscall_entry() {
  asm volatile(
      // disable interrupts ASAP
      "cli\n\t"

      // switch to kernel gsbase
      "swapgs\n\t"

      // switch to the kernel stack
      // store user RSP to scratch
      "mov %%rsp, %%gs:%c0\n\t"
      // load kernel RSP
      "mov %%gs:%c1, %%rsp\n\t"

      // build iretq frame on kernel stack
      "pushq %[ss]\n\t"    // SS (user data selector)
      "pushq %%gs:%c0\n\t" // RSP from scratch
      "pushq %%r11\n\t"    // RFLAGS
      "pushq %[cs]\n\t"    // CS (user code selector)
      "pushq %%rcx\n\t"    // RIP

      // save GPRs
      "push %%rbp\n\t"
      "push %%r15\n\t"
      "push %%r14\n\t"
      "push %%r13\n\t"
      "push %%r12\n\t"
      "pushq $0\n\t" // r11
      "push %%r10\n\t"
      "push %%r9\n\t"
      "push %%r8\n\t"
      "push %%rsi\n\t"
      "push %%rdi\n\t"
      "push %%rdx\n\t"
      "pushq $0\n\t" // rcx
      "push %%rbx\n\t"
      "push %%rax\n\t"

      // enable interrupts
      "sti\n\t"

      // call syscall handler
      "xor %%ebp, %%ebp\n\t"
      "mov %%rsp, %%rdi\n\t"
      "cld\n\t"
      "jmp syscall_handler\n\t"
      :
      : "i"(offsetof(CpuData, md.syscall_scratch)),
        "i"(offsetof(CpuData, kernel_stack_top)), [cs] "i"(GDT_SEL_USER_CODE),
        [ss] "i"(GDT_SEL_USER_DATA));
}
} // namespace yak::arch
