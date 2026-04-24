# struct ThreadPcb {
#   uint64_t sp;
#   uint64_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
# };

.extern sched_finalize_switch
.extern __thread_exit_trampoline

.globl __riscv64_sched_switch
.type __riscv64_sched_switch, @function
# a0: current (ThreadPcb*)
# a1: next (ThreadPcb*)
__riscv64_sched_switch:
	# save current thread's callee-saved state
	sd ra,  0*8(a0)
	sd sp,  1*8(a0)
	sd s0,  2*8(a0)
	sd s1,  3*8(a0)
	sd s2,  4*8(a0)
	sd s3,  5*8(a0)
	sd s4,  6*8(a0)
	sd s5,  7*8(a0)
	sd s6,  8*8(a0)
	sd s7,  9*8(a0)
	sd s8,  10*8(a0)
	sd s9,  11*8(a0)
	sd s10, 12*8(a0)
	sd s11, 13*8(a0)

	# switch to next thread's stack
	ld sp, 1*8(a1)

	# preserve next pointer across call into C.
	# s11 is callee-saved so sched_finalize_switch won't clobber it.
	mv s11, a1
	call sched_finalize_switch

 	# restore next thread's callee-saved state
	ld ra,  0*8(s11)
	# sp already restored
	ld s0,  2*8(s11)
	ld s1,  3*8(s11)
	ld s2,  4*8(s11)
	ld s3,  5*8(s11)
	ld s4,  6*8(s11)
	ld s5,  7*8(s11)
	ld s6,  8*8(s11)
	ld s7,  9*8(s11)
	ld s8,  10*8(s11)
	ld s9,  11*8(s11)
	ld s10, 12*8(s11)
	ld s11, 13*8(s11) # restore s11 last, same as x86 r15 trick
	ret

.globl __riscv64_sched_trampoline
.type __riscv64_sched_trampoline, @function
# s2: entry
# s3: context1
# s4: context2
__riscv64_sched_trampoline:
    # establish a clean stack frame with null return address
    # so that backtraces terminate here
    li   fp, 0
    addi sp, sp, -16
    sd   fp, 8(sp) # return address = 0
    sd   sp, 0(sp) # saved fp = 0
    addi fp, sp, 16

    # enable interrupts
    csrsi sstatus, 0x2

    # call entry(context1, context2)
    mv   a0, s3
    mv   a1, s4
    jalr s2

    call __thread_exit_trampoline

    # should never reach here
    unimp
