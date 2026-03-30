#include <stdint.h>
#include <x86_64/asm.h>
#include <x86_64/gdt.h>
#include <x86_64/idt.h>
#include <yak/arch-intr.h>
#include <yak/config.h>
#include <yak/cpudata.h>
#include <yak/ipl.h>
#include <yak/log.h>
#include <yak/panic.h>

namespace yak::arch {
struct [[gnu::packed]] IdtEntry {
  uint16_t isr_low;
  uint16_t kernel_cs;
  uint8_t ist;
  uint8_t attributes;
  uint16_t isr_mid;
  uint32_t isr_high;
  uint32_t rsv;
};

static void idt_make_entry(IdtEntry *entry, uint64_t isr) {
  entry->isr_low = (uint16_t)(isr);
  entry->isr_mid = (uint16_t)(isr >> 16);
  entry->isr_high = (uint32_t)(isr >> 32);
  entry->kernel_cs = GDT_SEL_KERNEL_CODE;
  entry->rsv = 0;
  entry->ist = 0;
  entry->attributes = 0x8E;
}

static void idt_set_ist(IdtEntry *entry, uint8_t ist) { entry->ist = ist; }

[[gnu::aligned(16)]]
static IdtEntry idt[256];
extern "C" uint64_t itable[256];

void idt_init() {
  for (int i = 0; i < 256; i++) {
    idt_make_entry(&idt[i], itable[i]);
  }

  // nmi
  idt_set_ist(&idt[2], 1);

  // double fault
  idt_set_ist(&idt[8], 2);

  // debug / breakpoint
  idt_set_ist(&idt[1], 3);
  idt_set_ist(&idt[3], 3);
}

void idt_reload() {
  struct [[gnu::packed]] {
    uint16_t limit;
    uint64_t base;
  } idtr = {.limit = sizeof(IdtEntry) * 256 - 1, .base = (uint64_t)&idt[0]};

  asm volatile("lidt %0" ::"m"(idtr) : "memory");
}

struct [[gnu::packed]] Context {
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t rbp;

  uint64_t number;
  uint64_t error;

  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
};

#define ANSI_RESET "\033[0m"
#define ANSI_GRAY "\033[90m"
#define ANSI_CYAN "\033[36m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_GREEN "\033[32m"
#define ANSI_MAGENTA "\033[35m"

static void dump_context(const Context *ctx, bool irq) {
  uintptr_t cr0 = asm_rdcr0(), cr2 = asm_rdcr2(), cr3 = asm_rdcr3(),
            cr4 = asm_rdcr4(), cr8 = asm_rdcr8();

  pr_raw("====== CONTEXT DUMP ======\n");

  pr_raw("Register dump:\n");
  pr_raw(" " ANSI_GRAY "RAX=" ANSI_CYAN "%016lx" ANSI_RESET "  " ANSI_GRAY
         "RBX=" ANSI_CYAN "%016lx" ANSI_RESET "  " ANSI_GRAY "RCX=" ANSI_CYAN
         "%016lx" ANSI_RESET "  " ANSI_GRAY "RDX=" ANSI_CYAN
         "%016lx\n" ANSI_RESET,
         ctx->rax, ctx->rbx, ctx->rcx, ctx->rdx);

  pr_raw(" " ANSI_GRAY "RSI=" ANSI_CYAN "%016lx" ANSI_RESET "  " ANSI_GRAY
         "RDI=" ANSI_CYAN "%016lx" ANSI_RESET "   " ANSI_GRAY "R8=" ANSI_CYAN
         "%016lx" ANSI_RESET "   " ANSI_GRAY "R9=" ANSI_CYAN
         "%016lx\n" ANSI_RESET,
         ctx->rsi, ctx->rdi, ctx->r8, ctx->r9);

  pr_raw(" " ANSI_GRAY "R10=" ANSI_CYAN "%016lx" ANSI_RESET "  " ANSI_GRAY
         "R11=" ANSI_CYAN "%016lx" ANSI_RESET "  " ANSI_GRAY "R12=" ANSI_CYAN
         "%016lx" ANSI_RESET "  " ANSI_GRAY "R13=" ANSI_CYAN
         "%016lx\n" ANSI_RESET,
         ctx->r10, ctx->r11, ctx->r12, ctx->r13);

  pr_raw(" " ANSI_GRAY "R14=" ANSI_CYAN "%016lx" ANSI_RESET "  " ANSI_GRAY
         "R15=" ANSI_CYAN "%016lx" ANSI_RESET "  " ANSI_GRAY "RBP=" ANSI_CYAN
         "%016lx\n" ANSI_RESET,
         ctx->r14, ctx->r15, ctx->rbp);

  if (irq) {
    pr_raw("Exception info:\n");
    pr_raw(" " ANSI_GRAY "NUMBER=" ANSI_YELLOW "%lx" ANSI_RESET "  " ANSI_GRAY
           "ERROR=" ANSI_YELLOW "%lx\n" ANSI_RESET,
           ctx->number, ctx->error);
  }

  pr_raw("Execution state:\n");
  pr_raw(" " ANSI_GRAY "RIP=" ANSI_GREEN "%016lx" ANSI_RESET "      " ANSI_GRAY
         "CS=" ANSI_GREEN "%016lx\n" ANSI_RESET,
         ctx->rip, ctx->cs);

  pr_raw(" " ANSI_GRAY "RFLAGS=" ANSI_MAGENTA "%016lx" ANSI_RESET "  " ANSI_GRAY
         "RSP=" ANSI_GREEN "%016lx" ANSI_RESET "  " ANSI_GRAY "SS=" ANSI_GREEN
         "%016lx\n" ANSI_RESET,
         ctx->rflags, ctx->rsp, ctx->ss);

  pr_debug("Control registers:\n");
  pr_debug(" " ANSI_GRAY "CR0=" ANSI_GREEN "%016lx" ANSI_RESET "  " ANSI_GRAY
           "CR2=" ANSI_GREEN "%016lx" ANSI_RESET "  " ANSI_GRAY
           "CR3=" ANSI_GREEN "%016lx\n" ANSI_RESET,
           cr0, cr2, cr3);

  pr_debug(" " ANSI_GRAY "CR4=" ANSI_GREEN "%016lx" ANSI_RESET "  " ANSI_GRAY
           "CR8=" ANSI_GREEN "%016lx\n" ANSI_RESET,
           cr4, cr8);
}

static const char *exception_names[] = {
    "Divide by Zero",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device not available",
    "Double Fault",
    "",
    "Invalid TSS",
    "Segment not present",
    "Stack Segment fault",
    "GPF",
    "Page Fault",
    "",
    "x87 Floating point exception",
    "Alignment check",
    "Machine check",
    "SIMD Floating point exception",
    "Virtualization Exception",
    "Control protection exception",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor injection exception",
    "VMM Communication exception",
    "Security exception",
    "",
};

extern "C" void __isr_c_entry(Context *frame) {
  if (frame->number >= 0 && frame->number <= 31) {
    if (frame->number == 14) {
      uintptr_t addr = asm_rdcr2();
      pr_error("Unhandled #PF at 0x%lx\n", addr);
    }
    pr_error("Exception %s received (0x%lx)\n", exception_names[frame->number],
             frame->number);
  } else {
    Ipl ipl = iplget();
    Ipl level_ipl = static_cast<Ipl>(frame->number >> 4);

#if CONFIG_LAZY_IPL
    if (ipl >= level_ipl && CPUDATA_LOAD(md.hard_ipl) < level_ipl) {
      asm_wrcr8(static_cast<unsigned int>(level_ipl));
      CPUDATA_STORE(md.hard_ipl, level_ipl);
      panic("impl: defer interrupt");
      return;
    }
#endif

    iplr(level_ipl);
    enable_interrupts();

    pr_error("Unhandled IRQ: #%ld\n", frame->number);

    disable_interrupts();
    iplx(ipl);
  }

  dump_context(frame, true);
  asm volatile("cli\n\thlt\n\t");
}

} // namespace yak::arch
