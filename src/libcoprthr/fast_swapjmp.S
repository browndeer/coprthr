#define JB_RBX 0
#define JB_RBP 1
#define JB_R12 2
#define JB_R13 3
#define JB_R14 4
#define JB_R15 5
#define JB_RSP 6
#define JB_PC  7
#define VCDATA_JB_THIS	2
#define VCDATA_JB_NEXT	3
	.text
	.align 	16
	.globl	fast_swapjmp
	.type fast_swapjmp,@function
fast_swapjmp:
	movl %edi,%ebx
	movq %rsp,%r15
	andq $-16384,%r15
	movq (VCDATA_JB_THIS*8)(%r15),%rdi
	call .LBB_setjmp
	testl %eax,%eax
	jne .LBB_done
	movl  %ebx, %esi
	movq  (VCDATA_JB_NEXT*8)(%r15),%rdi
   movq (JB_RSP*8)(%rdi),%r8
   movq (JB_RBP*8)(%rdi),%r9
   movq (JB_PC*8)(%rdi),%rdx
   movq (JB_RBX*8)(%rdi),%rbx
   movq (JB_R12*8)(%rdi),%r12
   movq (JB_R13*8)(%rdi),%r13
   movq (JB_R14*8)(%rdi),%r14
   movq (JB_R15*8)(%rdi),%r15
   testl %esi,%esi
   movl $1,%eax
   cmove %eax,%esi
   movl %esi,%eax
   movq %r8,%rsp
   movq %r9,%rbp
   jmpq *%rdx
.LBB_setjmp:
	movq %rbx,(JB_RBX*8)(%rdi)
	movq %rbp,(JB_RBP*8)(%rdi)
	movq %r12,(JB_R12*8)(%rdi)
	movq %r13,(JB_R13*8)(%rdi)
	movq %r14,(JB_R14*8)(%rdi)
	movq %r15,(JB_R15*8)(%rdi)
	leaq 8(%rsp),%rdx
	movq %rdx,(JB_RSP*8)(%rdi)
	movq (%rsp),%rax
	movq %rax,(JB_PC*8)(%rdi)
	xorl %eax,%eax
.LBB_done:
	ret
	.size fast_swapjmp, .-fast_swapjmp
