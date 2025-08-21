	.file	"desc.cpp"
gcc2_compiled.:
___gnu_compiled_cplusplus:
.text
	.p2align 2
.globl ___15CArchDescriptor
___15CArchDescriptor:
LFB1:
	pushl %ebp
LCFI0:
	movl %esp,%ebp
LCFI1:
	pushl %ebx
LCFI2:
	movl 8(%ebp),%ebx
	movl $__vt$15CArchDescriptor,8(%ebx)
	pushl $8
LCFI3:
	pushl $0
LCFI4:
	pushl %ebx
LCFI5:
	call _memset
	addl $12,%esp
LCFI6:
L2:
	movl %ebx,%eax
	jmp L1
	.align 2,0x90
L1:
	movl -4(%ebp),%ebx
	leave
	ret
LFE1:
	.p2align 2
.globl ___19CArchDescriptorCode
___19CArchDescriptorCode:
LFB2:
	pushl %ebp
LCFI7:
	movl %esp,%ebp
LCFI8:
	pushl %ebx
LCFI9:
	movl 8(%ebp),%ebx
	pushl %ebx
LCFI10:
	call ___15CArchDescriptor
	addl $4,%esp
LCFI11:
	movl $__vt$19CArchDescriptorCode,8(%ebx)
L4:
	movl %ebx,%eax
	jmp L3
	.align 2,0x90
L3:
	movl -4(%ebp),%ebx
	leave
	ret
LFE2:
	.p2align 2
.globl ___19CArchDescriptorCodeUiUiUi
___19CArchDescriptorCodeUiUiUi:
LFB3:
	pushl %ebp
LCFI12:
	movl %esp,%ebp
LCFI13:
	pushl %ebx
LCFI14:
	movl 8(%ebp),%ebx
	pushl %ebx
LCFI15:
	call ___15CArchDescriptor
	addl $4,%esp
LCFI16:
	movl $__vt$19CArchDescriptorCode,8(%ebx)
	movl 12(%ebp),%eax
	movw %ax,14(%ebx)
	movb 16(%ebp),%al
	movb %al,17(%ebx)
	movb 20(%ebp),%al
	movb %al,18(%ebx)
L6:
	movl %ebx,%eax
	jmp L5
	.align 2,0x90
L5:
	movl -4(%ebp),%ebx
	leave
	ret
LFE3:
LC0:
	.ascii "loading code descriptor at address...\0"
	.p2align 2
.globl _Load__19CArchDescriptorCodeUi
_Load__19CArchDescriptorCodeUi:
LFB4:
	pushl %ebp
LCFI17:
	movl %esp,%ebp
LCFI18:
	pushl %ebx
LCFI19:
	movl 8(%ebp),%ebx
	pushl $LC0
LCFI20:
	call _show_msgf
	addl $4,%esp
LCFI21:
L7:
	movl -4(%ebp),%ebx
	leave
	ret
LFE4:
	.p2align 2
.globl _desc_module
_desc_module:
LFB5:
	pushl %ebp
LCFI22:
	movl %esp,%ebp
LCFI23:
	subl $24,%esp
LCFI24:
	pushl %esi
LCFI25:
	pushl %ebx
LCFI26:
	pushl $1
LCFI27:
	pushl $1
LCFI28:
	pushl $1
LCFI29:
	leal -24(%ebp),%eax
	pushl %eax
LCFI30:
	call ___19CArchDescriptorCodeUiUiUi
	addl $16,%esp
LCFI31:
	leal -24(%ebp),%ecx
	movl %ecx,-4(%ebp)
	movl -4(%ebp),%eax
	movl 8(%eax),%ebx
	addl $8,%ebx
	pushl $10
LCFI32:
	movswl (%ebx),%eax
	movl %eax,%edx
	addl -4(%ebp),%edx
	pushl %edx
LCFI33:
	movl 4(%ebx),%esi
	call *%esi
	addl $8,%esp
LCFI34:
L8:
	leal -32(%ebp),%esp
	popl %ebx
	popl %esi
	leave
	ret
LFE5:
.globl __vt$19CArchDescriptorCode
	.p2align 2
__vt$19CArchDescriptorCode:
	.word 0
	.word 0
	.long ___tf19CArchDescriptorCode
	.word 0
	.word 0
	.long _Load__19CArchDescriptorCodeUi
.comm ___ti19CArchDescriptorCode,12
LC1:
	.ascii "19CArchDescriptorCode\0"
	.p2align 2
.globl ___tf19CArchDescriptorCode
___tf19CArchDescriptorCode:
LFB6:
	pushl %ebp
LCFI35:
	movl %esp,%ebp
LCFI36:
	cmpl $0,___ti19CArchDescriptorCode
	jne L10
	call ___tf15CArchDescriptor
	movl %eax,%eax
	pushl $___ti15CArchDescriptor
LCFI37:
	pushl $LC1
LCFI38:
	pushl $___ti19CArchDescriptorCode
LCFI39:
	call ___rtti_si
	addl $12,%esp
LCFI40:
L10:
	movl $___ti19CArchDescriptorCode,%eax
	jmp L9
	.align 2,0x90
L9:
	leave
	ret
LFE6:

	.section .eh_frame
___FRAME_BEGIN__:
	.long LECIE1-LSCIE1

LSCIE1:
	.long 0

		.byte	0x1
		.byte	0x0
		.byte	0x1
		.byte	0x7c
		.byte	0x8
		.byte	0xc
		.byte	0x5
		.byte	0x4
		.byte	0x88
		.byte	0x1
	.p2align 2
LECIE1:
	.long LEFDE1-LSFDE1

LSFDE1:
	.long LSFDE1-___FRAME_BEGIN__

	.long LFB1

	.long LFE1-LFB1

		.byte	0x4
	.long LCFI0-LFB1

		.byte	0xe
		.byte	0x8
		.byte	0x84
		.byte	0x2
		.byte	0x4
	.long LCFI1-LCFI0

		.byte	0xd
		.byte	0x4
		.byte	0x4
	.long LCFI2-LCFI1

		.byte	0x83
		.byte	0x3
		.byte	0x4
	.long LCFI3-LCFI2

		.byte	0x2e
		.byte	0x4
		.byte	0x4
	.long LCFI4-LCFI3

		.byte	0x2e
		.byte	0x8
		.byte	0x4
	.long LCFI5-LCFI4

		.byte	0x2e
		.byte	0xc
		.byte	0x4
	.long LCFI6-LCFI5

		.byte	0x2e
		.byte	0x0
	.p2align 2
LEFDE1:
	.long LEFDE3-LSFDE3

LSFDE3:
	.long LSFDE3-___FRAME_BEGIN__

	.long LFB2

	.long LFE2-LFB2

		.byte	0x4
	.long LCFI7-LFB2

		.byte	0xe
		.byte	0x8
		.byte	0x84
		.byte	0x2
		.byte	0x4
	.long LCFI8-LCFI7

		.byte	0xd
		.byte	0x4
		.byte	0x4
	.long LCFI9-LCFI8

		.byte	0x83
		.byte	0x3
		.byte	0x4
	.long LCFI10-LCFI9

		.byte	0x2e
		.byte	0x4
		.byte	0x4
	.long LCFI11-LCFI10

		.byte	0x2e
		.byte	0x0
	.p2align 2
LEFDE3:
	.long LEFDE5-LSFDE5

LSFDE5:
	.long LSFDE5-___FRAME_BEGIN__

	.long LFB3

	.long LFE3-LFB3

		.byte	0x4
	.long LCFI12-LFB3

		.byte	0xe
		.byte	0x8
		.byte	0x84
		.byte	0x2
		.byte	0x4
	.long LCFI13-LCFI12

		.byte	0xd
		.byte	0x4
		.byte	0x4
	.long LCFI14-LCFI13

		.byte	0x83
		.byte	0x3
		.byte	0x4
	.long LCFI15-LCFI14

		.byte	0x2e
		.byte	0x4
		.byte	0x4
	.long LCFI16-LCFI15

		.byte	0x2e
		.byte	0x0
	.p2align 2
LEFDE5:
	.long LEFDE7-LSFDE7

LSFDE7:
	.long LSFDE7-___FRAME_BEGIN__

	.long LFB4

	.long LFE4-LFB4

		.byte	0x4
	.long LCFI17-LFB4

		.byte	0xe
		.byte	0x8
		.byte	0x84
		.byte	0x2
		.byte	0x4
	.long LCFI18-LCFI17

		.byte	0xd
		.byte	0x4
		.byte	0x4
	.long LCFI19-LCFI18

		.byte	0x83
		.byte	0x3
		.byte	0x4
	.long LCFI20-LCFI19

		.byte	0x2e
		.byte	0x4
		.byte	0x4
	.long LCFI21-LCFI20

		.byte	0x2e
		.byte	0x0
	.p2align 2
LEFDE7:
	.long LEFDE9-LSFDE9

LSFDE9:
	.long LSFDE9-___FRAME_BEGIN__

	.long LFB5

	.long LFE5-LFB5

		.byte	0x4
	.long LCFI22-LFB5

		.byte	0xe
		.byte	0x8
		.byte	0x84
		.byte	0x2
		.byte	0x4
	.long LCFI23-LCFI22

		.byte	0xd
		.byte	0x4
		.byte	0x4
	.long LCFI25-LCFI23

		.byte	0x86
		.byte	0x9
		.byte	0x4
	.long LCFI26-LCFI25

		.byte	0x83
		.byte	0xa
		.byte	0x4
	.long LCFI27-LCFI26

		.byte	0x2e
		.byte	0x4
		.byte	0x4
	.long LCFI28-LCFI27

		.byte	0x2e
		.byte	0x8
		.byte	0x4
	.long LCFI29-LCFI28

		.byte	0x2e
		.byte	0xc
		.byte	0x4
	.long LCFI30-LCFI29

		.byte	0x2e
		.byte	0x10
		.byte	0x4
	.long LCFI31-LCFI30

		.byte	0x2e
		.byte	0x0
		.byte	0x4
	.long LCFI32-LCFI31

		.byte	0x2e
		.byte	0x4
		.byte	0x4
	.long LCFI33-LCFI32

		.byte	0x2e
		.byte	0x8
		.byte	0x4
	.long LCFI34-LCFI33

		.byte	0x2e
		.byte	0x0
	.p2align 2
LEFDE9:
	.long LEFDE11-LSFDE11

LSFDE11:
	.long LSFDE11-___FRAME_BEGIN__

	.long LFB6

	.long LFE6-LFB6

		.byte	0x4
	.long LCFI35-LFB6

		.byte	0xe
		.byte	0x8
		.byte	0x84
		.byte	0x2
		.byte	0x4
	.long LCFI36-LCFI35

		.byte	0xd
		.byte	0x4
		.byte	0x4
	.long LCFI37-LCFI36

		.byte	0x2e
		.byte	0x4
		.byte	0x4
	.long LCFI38-LCFI37

		.byte	0x2e
		.byte	0x8
		.byte	0x4
	.long LCFI39-LCFI38

		.byte	0x2e
		.byte	0xc
		.byte	0x4
	.long LCFI40-LCFI39

		.byte	0x2e
		.byte	0x0
	.p2align 2
LEFDE11:
