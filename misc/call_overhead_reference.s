	.file	"call_overhead_reference.c"
	.text
	.globl	_recurser
	.def	_recurser;	.scl	2;	.type	32;	.endef
_recurser:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %edx
	movl	12(%ebp), %eax
	movl	(%eax), %eax
	cmpl	%eax, %edx
	jl	L2
	jmp	L1
L2:
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	leal	1(%eax), %edx
	movl	8(%ebp), %eax
	movl	%edx, (%eax)
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_recurser
L1:
	leave
	ret
	.globl	_iterator
	.def	_iterator;	.scl	2;	.type	32;	.endef
_iterator:
	pushl	%ebp
	movl	%esp, %ebp
	jmp	L5
L6:
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	leal	1(%eax), %edx
	movl	8(%ebp), %eax
	movl	%edx, (%eax)
L5:
	movl	8(%ebp), %eax
	movl	(%eax), %edx
	movl	12(%ebp), %eax
	movl	(%eax), %eax
	cmpl	%eax, %edx
	jl	L6
	nop
	popl	%ebp
	ret
	.def	___main;	.scl	2;	.type	32;	.endef
	.section .rdata,"dr"
LC0:
	.ascii "%i,%i\12\0"
	.text
	.globl	_main
	.def	_main;	.scl	2;	.type	32;	.endef
_main:
	pushl	%ebp
	movl	%esp, %ebp
	andl	$-16, %esp
	subl	$32, %esp
	call	___main
	movl	$0, 28(%esp)
	movl	$0, 24(%esp)
	movl	$10, 20(%esp)
	leal	20(%esp), %eax
	movl	%eax, 4(%esp)
	leal	28(%esp), %eax
	movl	%eax, (%esp)
	call	_recurser
	leal	20(%esp), %eax
	movl	%eax, 4(%esp)
	leal	24(%esp), %eax
	movl	%eax, (%esp)
	call	_iterator
	movl	24(%esp), %edx
	movl	28(%esp), %eax
	movl	%edx, 8(%esp)
	movl	%eax, 4(%esp)
	movl	$LC0, (%esp)
	call	_printf
	movl	28(%esp), %edx
	movl	24(%esp), %eax
	addl	%edx, %eax
	leave
	ret
	.ident	"GCC: (GNU) 4.9.2"
	.def	_printf;	.scl	2;	.type	32;	.endef
