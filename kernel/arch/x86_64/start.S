.extern kstart
.extern __kernel_entry_stack_end

.text

	.globl _start
	.balign 16
	.type _start, @function
_start:
	# switch off of limine's stack
	# we want to reclaim bootloader memory
	lea __kernel_entry_stack_end(%rip), %rsp
	jmp kstart 

.section .note.GNU-stack,"",%progbits
