	.text
.globl fast_longjmp
	.type fast_longjmp, @function
fast_longjmp:

	movq (6*8)(%rdi),%r8
	movq (1*8)(%rdi),%r9
	movq (7*8)(%rdi),%rdx

	movq (0*8)(%rdi),%rbx
	movq (2*8)(%rdi),%r12
	movq (3*8)(%rdi),%r13
	movq (4*8)(%rdi),%r14
	movq (5*8)(%rdi),%r15

	mov %esi, %eax
	movq %r8,%rsp
	movq %r9,%rbp
	jmpq *%rdx
	.size fast_longjmp,.-fast_longjmp
