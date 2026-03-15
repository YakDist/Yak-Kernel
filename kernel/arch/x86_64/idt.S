.text

.set .L_NO_CODE, 1
.set .L_CODE, 2

.macro CFI_PUSH reg
	push %\reg
	.cfi_adjust_cfa_offset 8
	.cfi_rel_offset %\reg, 0
.endm

.macro CFI_POP reg
	pop %\reg
	.cfi_adjust_cfa_offset -8
	.cfi_restore %\reg
.endm

.macro PUSHA64
	CFI_PUSH rbp
	CFI_PUSH r15
	CFI_PUSH r14
	CFI_PUSH r13
	CFI_PUSH r12
	CFI_PUSH r11
	CFI_PUSH r10
	CFI_PUSH r9
	CFI_PUSH r8
	CFI_PUSH rdi
	CFI_PUSH rsi
	CFI_PUSH rdx
	CFI_PUSH rcx
	CFI_PUSH rbx
	CFI_PUSH rax
.endm

.macro POPA64
	CFI_POP rax
	CFI_POP rbx
	CFI_POP rcx
	CFI_POP rdx
	CFI_POP rsi
	CFI_POP rdi
	CFI_POP r8
	CFI_POP r9
	CFI_POP r10
	CFI_POP r11
	CFI_POP r12
	CFI_POP r13
	CFI_POP r14
	CFI_POP r15
	CFI_POP rbp
.endm

.macro ISR_STUB type, number

	.balign 16
	.type isrStub\number, @function

isrStub\number:
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp, 0
	.cfi_offset %rip, 0
	.cfi_offset %rsp, 24

	.if \type == .L_NO_CODE
		pushq $0
		.cfi_adjust_cfa_offset 8
	.else
		/* CPU pushed the error code */
		.cfi_adjust_cfa_offset 8
	.endif
	
	/* test for user CPL */
	testl $3, 16(%rsp)
	jz 1f
	swapgs
	1:

	/* push irq number */
	pushq $\number
	.cfi_adjust_cfa_offset 8

	/* jump to common handling code */
	jmp isr_common
	.cfi_endproc
.endm

ISR_STUB .L_NO_CODE, 0
ISR_STUB .L_NO_CODE, 1
ISR_STUB .L_NO_CODE, 2
ISR_STUB .L_NO_CODE, 3
ISR_STUB .L_NO_CODE, 4
ISR_STUB .L_NO_CODE, 5
ISR_STUB .L_NO_CODE, 6
ISR_STUB .L_NO_CODE, 7
ISR_STUB .L_CODE, 8
ISR_STUB .L_CODE, 10
ISR_STUB .L_CODE, 11
ISR_STUB .L_CODE, 12
ISR_STUB .L_CODE, 13
ISR_STUB .L_CODE, 14
ISR_STUB .L_NO_CODE, 16
ISR_STUB .L_CODE, 17
ISR_STUB .L_NO_CODE, 18
ISR_STUB .L_NO_CODE, 19

.altmacro
.set i, 32
.rept 224
	ISR_STUB .L_NO_CODE, %i
	.set i, i+1
.endr

	.balign 16
	.extern __isr_c_entry
	.type isr_common, @function
isr_common:
	.cfi_startproc

	PUSHA64

	mov %rsp, %rdi
	xor %ebp, %ebp
	cld
	call __isr_c_entry

	POPA64

	/* drop error code and interrupt number */
	add $2*8, %rsp
	.cfi_adjust_cfa_offset -16

	testl $3, 8(%rsp)
	jz 1f
	swapgs
	1:

	iretq
	
	.cfi_endproc

.macro DEC_INT num
	.quad isrStub\num
.endm

	.section .rodata
	.global itable
	.balign 16
itable:
	DEC_INT 0
	DEC_INT 1
	DEC_INT 2
	DEC_INT 3
	DEC_INT 4
	DEC_INT 5
	DEC_INT 6
	DEC_INT 7
	DEC_INT 8
	.quad 0x00000000
	DEC_INT 10
	DEC_INT 11
	DEC_INT 12
	DEC_INT 13
	DEC_INT 14
	.quad 0x00000000
	DEC_INT 16
	DEC_INT 17
	DEC_INT 18
	DEC_INT 19
.rept 12
	.quad 0x00000000
.endr

.set i,32
.rept 224
	DEC_INT %i
.set i,i+1
.endr
