.text

.globl memset
.type memset, @function
# void *memset(void *p, int c, size_t n)
memset:
.cfi_startproc
	push %rdi
.cfi_adjust_cfa_offset 8
	mov %rdx, %rcx
	mov %sil, %al
	rep stosb
	pop %rax
.cfi_adjust_cfa_offset -8
	ret
.cfi_endproc
.size memset, .-memset

.globl memcpy
.type memcpy, @function
# memcpy(void *dst, const void *src, size_t n)
memcpy:
.cfi_startproc
    mov  %rdi, %rax
    mov  %rdx, %rcx
    rep  movsb
    ret
.cfi_endproc
.size memcpy, .-memcpy

.globl memcmp
.type  memcmp, @function
# memcmp(const void *s1, const void *s2, size_t n)
# returns 0 if equal, <0 if s1 < s2, >0 if s1 > s2
memcmp:
.cfi_startproc
	test %rdx, %rdx # n==0
	jz .Lequal
	mov %rdx, %rcx
	xor %eax, %eax
	repe cmpsb
	je .Lequal
	movzbl -1(%rdi), %eax
	movzbl -1(%rsi), %ecx
	sub  %ecx, %eax
	ret
.Lequal:
	xor %eax, %eax
	ret
.cfi_endproc
.size memcmp, .-memcmp
