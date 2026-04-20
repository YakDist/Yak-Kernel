# struct md_pcb {
#	uint64_t rbx, rsp, rbp, r12, r13, r14, r15;
# };

.extern sched_finalize_switch
.extern __thread_exit_trampoline

.globl __x86_sched_switch
.type __x86_sched_switch, @function
# rdi: current (ThreadPcb*)
# rsi: next (ThreadPcb*)
__x86_sched_switch:
	# save current state
	mov %rbx, 0*8(%rdi)
	mov %rsp, 1*8(%rdi)
	mov %rbp, 2*8(%rdi)
	mov %r12, 3*8(%rdi)
	mov %r13, 4*8(%rdi)
	mov %r14, 5*8(%rdi)
	mov %r15, 6*8(%rdi)
	
	# switch to the next threads stack
	mov 1*8(%rsi), %rsp

	# preserve next pointer across call into C
	# the ABI mandates that r15 be saved
	#
	# finalize signales to other code that we have
	# switched of the 'current' stack and unlocks current
	# 
	# XXX: this could be assembly too, no?
	mov %rsi, %r15
	call sched_finalize_switch

	 # restore state from next
	 mov 0*8(%r15), %rbx
	 # already restored rsp
	 mov 2*8(%r15), %rbp
	 mov 3*8(%r15), %r12
	 mov 4*8(%r15), %r13
	 mov 5*8(%r15), %r14
	 # restore r15 last, as this is our pcb pointer
	 mov 6*8(%r15), %r15 

	 ret

.globl __x86_sched_trampoline
.type __x86_sched_trampoline, @function
# r12: entry
# r13: context1
# r14: context2
__x86_sched_trampoline:
	xor %ebp, %ebp
	push %rbp
	mov %rsp, %rbp

	sti

	mov %r13, %rdi
	mov %r14, %rsi
	call *%r12

	call __thread_exit_trampoline

	ud2
