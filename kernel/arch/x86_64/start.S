.extern kernel_entry
.extern __percpu_start
.extern __kernel_entry_stack_top

#include <yak/config.h>

.text
	
	.globl exe_entry 
	.balign 16
	.type exe_entry, @function
exe_entry:
	# Switch to our own stack
	lea __kernel_entry_stack_top(%rip), %rsp	

#if CONFIG_STACK_CANARIES
	# First, setup kernel stack canary

	# Check for RDRAND support
	# Leaf 1, ECX bit 30
	mov $1, %eax
	cpuid
	bt $30, %ecx
	# If no RDRAND, jump to fallback
	jnc 1f

2:
	# Generate random 64-bit value
	rdrand %rax
	# Either succeed and jump to storage
	jc 3f
	# Or retry 
	pause
	jmp 2b

1:
	# EDX:EAX = timestamp
	rdtsc 
	salq $32, %rdx
	orq %rdx, %rax
	xorq $0x55AA55AA, %rax

3:
	# RAX now contains some (hopefully) random value
	movq %rax, __stack_chk_guard(%rip)
#endif

	call kernel_entry

	cli
	ud2

#if CONFIG_STACK_CANARIES
.data
	.globl __stack_chk_guard
	.type __stack_chk_guard, @object
__stack_chk_guard:
	.quad 0
#endif

.section .note.GNU-stack,"",%progbits
