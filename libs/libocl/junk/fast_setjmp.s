	.text
.globl fast_setjmp
	.type fast_setjmp, @function
fast_setjmp:
	movq %rbx,(0*8)(%rdi)
	movq %rbp,(1*8)(%rdi)
	movq %r12,(2*8)(%rdi)
	movq %r13,(3*8)(%rdi)
	movq %r14,(4*8)(%rdi)
	movq %r15,(5*8)(%rdi)
	leaq 8(%rsp),%rax
	movq %rax,(6*8)(%rdi)
	movq (%rsp),%rax
	movq %rax,(7*8)(%rdi)
	xorq %rax,%rax
	ret
	.size fast_setjmp,.-fast_setjmp

