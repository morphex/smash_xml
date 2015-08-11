	.file	"call_overhead.c"
	.text
	.globl	_recurser
	.def	_recurser;	.scl	2;	.type	32;	.endef
_recurser:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %eax
	cmpl	12(%ebp), %eax
	jl	L2
	movl	8(%ebp), %eax
	jmp	L3
L2:
	movl	8(%ebp), %eax
	leal	1(%eax), %edx
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	_recurser
L3:
	leave
	ret
	.globl	_iterator
	.def	_iterator;	.scl	2;	.type	32;	.endef
_iterator:
	pushl	%ebp
	movl	%esp, %ebp
	jmp	L5
L6:
	addl	$1, 8(%ebp)
	addl	$1, 8(%ebp)
L5:
	movl	8(%ebp), %eax
	cmpl	12(%ebp), %eax
	jle	L6
	movl	8(%ebp), %eax
	popl	%ebp
	ret
	.def	___main;	.scl	2;	.type	32;	.endef
	.globl	_main
	.def	_main;	.scl	2;	.type	32;	.endef
_main:
	pushl	%ebp
	movl	%esp, %ebp
	andl	$-16, %esp
	subl	$32, %esp
	call	___main
	movl	$0, 28(%esp)
	movl	$10, 24(%esp)
	movl	24(%esp), %eax
	movl	%eax, 4(%esp)
	movl	28(%esp), %eax
	movl	%eax, (%esp)
	call	_recurser
	movl	%eax, 20(%esp)
	movl	24(%esp), %eax
	movl	%eax, 4(%esp)
	movl	28(%esp), %eax
	movl	%eax, (%esp)
	call	_iterator
	movl	%eax, 16(%esp)
	movl	20(%esp), %edx
	movl	16(%esp), %eax
	addl	%edx, %eax
	leave
	ret
	.ident	"GCC: (GNU) 4.9.2"
