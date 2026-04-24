.extern kernel_entry
.extern __percpu_start
.extern __kernel_entry_stack_top
#include <yak/config.h>

.text

    .globl exe_entry
    .balign 4
    .type exe_entry, @function
exe_entry:
    // Switch to our own stack
    la sp, __kernel_entry_stack_top
    // And setup the BSP's tp
    la tp, __percpu_start

#if CONFIG_STACK_CANARIES
    // Generate a canary value.
    // We use the cycle CSR (low 64-bits of mcycle, readable from S-mode
    // if scounteren.CY is set by firmware) as an entropy source, xored
    // with the time CSR for a bit more variance.
    csrr a0, cycle
    csrr a1, time
    xor  a0, a0, a1
    li   a1, 0x55AA55AA
    xor  a0, a0, a1
    la   a1, __stack_chk_guard
    sd   a0, 0(a1)
#endif

    la   a0, __kernel_entry_stack_top
    call kernel_entry

    csrci sstatus, 0x2
1:
    wfi
    j 1b

#if CONFIG_STACK_CANARIES
.data
    .globl __stack_chk_guard
    .type __stack_chk_guard, @object
__stack_chk_guard:
    .quad 0
#endif

.section .note.GNU-stack,"",%progbits
