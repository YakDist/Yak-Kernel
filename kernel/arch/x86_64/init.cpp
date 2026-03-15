#include <x86_64/asm.h>
#include <x86_64/gdt.h>
#include <x86_64/idt.h>
#include <x86_64/msr.h>
#include <yak/init.h>

namespace yak::arch {
extern "C" char __percpu_start[];
extern "C" char __percpu_end[];

static void setup_cpu() {
  idt_reload();

  // initialize & load GDT (percpu!)
  gdt_init();
  gdt_reload();

  asm_wrcr8(0);
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
