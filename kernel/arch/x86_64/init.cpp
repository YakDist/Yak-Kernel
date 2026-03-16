#include <cstdint>
#include <x86_64/asm.h>
#include <x86_64/gdt.h>
#include <x86_64/idt.h>
#include <x86_64/msr.h>
#include <yak/cpudata.h>
#include <yak/init.h>

namespace yak::arch {
extern "C" char __percpu_start[];
extern "C" char __percpu_end[];

extern void syscall_entry(void);

static void setup_syscalls() {
  uint64_t efer = asm_rdmsr(msr::EFER);
  efer |= 1; // SCE bit: enable syscall/sysret
  asm_wrmsr(msr::EFER, efer);
  asm_wrmsr(msr::LSTAR, (uintptr_t)syscall_entry);
  uint64_t star = asm_rdmsr(msr::STAR);
  star |= ((uint64_t)GDT_SEL_USER_SYSCALL << 48);
  star |= ((uint64_t)GDT_SEL_KERNEL_CODE << 32);
  asm_wrmsr(msr::STAR, star);
}

static void setup_cpu() {
  idt_reload();

  // initialize & load GDT (percpu!)
  gdt_init();
  gdt_reload();

  asm_wrcr8(0);

  setup_syscalls();
}

void early_init() {
  // Setup GSBASE so that %gs:0 == the kernel ELF's PERCPU area
  //
  // kernel_entry() makes sure that cpudata->self already points to self
  asm_wrmsr(msr::GSBASE, (uint64_t)__percpu_start);

  idt_init();

  setup_cpu();
}
} // namespace yak::arch
