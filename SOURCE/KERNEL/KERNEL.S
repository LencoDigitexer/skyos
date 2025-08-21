/************************************************************************/
/* Sky Operating System V2.0j
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\kernel.s
/* Last Update: 31.05.1999
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux, Minix, all other operating systems,...
/************************************************************************/
/* Definition:
/*   The main kernel file. Initialize all processor specific code, and
/*   jumps to the C kernel file (main.c).
/*   Loaded at adress 0x10000
/*   Protected Mode Selector CS and DS set to adress 0x0
/*   Offset is 0x10000
/*   All this is in 32-Bit
/*   Contains the low level interrupt entry points.
/*
/*   Memory area at 0x80000 is used to store parameters and system info.
/************************************************************************/
.include "/sky/source/include/module.h"

#define CPU_PARAMS	0x80600
#define X86 CPU_PARAMS+0
#define X86_VENDOR	CPU_PARAMS+1
#define X86_MODEL	CPU_PARAMS+2
#define X86_MASK	CPU_PARAMS+3
#define X86_HARD_MATH	CPU_PARAMS+6
#define X86_CPUID	CPU_PARAMS+8
#define X86_CAPABILITY	CPU_PARAMS+12
#define X86_VENDOR_ID	CPU_PARAMS+16


NR_TASKS = 20
.globl text, _reboot, _idt, _gdt, _do_irq_timer, _do_irq_key, _do_nothing, _setpixel, _do_irq_floppy, _do_irq_ide
.globl _do_irq_detect_3,_do_irq_detect_4,_do_irq_detect_5,_do_irq_detect_6,_do_irq_detect_7
.globl _do_irq_detect_8,_do_irq_detect_9,_do_irq_detect_10,_do_irq_detect_11,_do_irq_detect_12
.globl _do_irq_detect_13,_do_irq_detect_14,_do_irq_detect_15,_do_irq_detect_16
.globl _do_page_fault, _do_gpf, _object_map, _do_irq_el3, _do_irq_unknown, _do_irq_mouse1, _do_irq_mouse2
.globl _loop, _do_invalid_tss, _do_irq_sb
.text

start_32:
  jmp start_over_data

_kernel_identifier:
_object_map:
  .long 0xAAAAAAAA              /* don't modifie this flag without
                                   changing this is startup.asm.
                                   This flag is used to determine in startup.asm
                                   wheter the kernel was loaded correct */

start_over_data:

# set all needed selectors
  xor %eax, %eax
  movl $0x18, %eax
  movw %ax, %ds
  movw %ax, %es

# set kernel stack

# lss _stack_start, %esp       ??? should we use this ???
  movw %ax, %ss
  movl $0x1FFF00, %esp

  pushl %eax
  popl %eax

  xor %ebx, %ebx

mem_test2:                      /* check memory size */
  add  $0x400000, %ebx
  movb $0xAF, %es:(%ebx)
  movb %es:(%ebx), %cl
  cmpb $0xAF, %cl
  je mem_test2

  movl %ebx, %edx
  xorl %ebx, %ebx
  movl $0x82000, %ebx

  movl %edx, %es:(%ebx)

# now we have to init paging

  call setup_idt
  call setup_paging

# here is the CPUID code from linux kernel 2.2.1
# file: arch/i386/head.S

# first zero the structure

  movl $0x18, %eax
  movw %ax, %es

  movl $0x80600, %ebx
  movl $0, %es:(%ebx)
  movl $0x80610, %ebx
  movl $0, %es:(%ebx)
  movl $0x80614, %ebx
  movl $0, %es:(%ebx)
  movl $0x80618, %ebx
  movl $0, %es:(%ebx)

#	movl $-1, X86_CPUID		#  -1 for no CPUID initially

  movl $0x80600, %ebx
  movl $-1, %es:(%ebx)

/* check if it is 486 or 386. */
/*
 * XXX - this does a lot of unnecessary setup.  Alignment checks don't
 * apply at our cpl of 0 and the stack ought to be aligned already, and
 * we don't need to preserve eflags.
 */

//	movl $3,X86		# at least 386

   movl $0x80600, %ebx
   movl $3, %es:(%ebx)

   pushfl			# push EFLAGS
	popl %eax		# get EFLAGS
	movl %eax,%ecx		# save original EFLAGS
	xorl $0x40000,%eax	# flip AC bit in EFLAGS
	pushl %eax		# copy to EFLAGS
	popfl			# set EFLAGS
	pushfl			# get new EFLAGS
	popl %eax		# put it in eax
	xorl %ecx,%eax		# change in flags
	andl $0x40000,%eax	# check if AC bit changed
	je is386

#	movl $4,X86		# at least 486

   movl $0x80600, %ebx
   movl $4, %es:(%ebx)

	movl %ecx,%eax
	xorl $0x200000,%eax	# check ID flag
	pushl %eax
	popfl			# if we are on a straight 486DX, SX, or
	pushfl			# 487SX we can't change it
	popl %eax
	xorl %ecx,%eax
	pushl %ecx		# restore original EFLAGS
	popfl
	andl $0x200000,%eax
	je is486

	/* get vendor info */
	xorl %eax,%eax			# call CPUID with 0 -> return vendor ID
	cpuid

#	movl %eax,X86_CPUID		# save CPUID level
#	movl %ebx,X86_VENDOR_ID		# lo 4 chars
#	movl %edx,X86_VENDOR_ID+4	# next 4 chars
#	movl %ecx,X86_VENDOR_ID+8	# last 4 chars

   pushl %ebx

   movl $0x80608, %ebx        # store the CPUID level
   movl %eax, %es:(%ebx)

   popl %eax


   movl $0x80610, %ebx
   movl %eax, %es:(%ebx)
   movl $0x80614, %ebx
   movl %edx, %es:(%ebx)
   movl $0x80618, %ebx
   movl %ecx, %es:(%ebx)

	orl %eax,%eax			# do we have processor info as well?
	je is486

	movl $1,%eax		# Use the CPUID instruction to get CPU type
	cpuid
	movb %al,%cl		# save reg for future use
	andb $0x0f,%ah		# mask processor family


#   movb %ah,X86
   pushl %ebx
   movl $0x80600, %ebx
   movb %ah, %es:(%ebx)
   popl %ebx
#

	andb $0xf0,%al		# mask model
	shrb $4,%al

#	movb %al,X86_MODEL
   pushl %ebx
   movl $0x80602, %ebx
   movb %ah, %es:(%ebx)
   popl %ebx
#

   andb $0x0f,%cl		# mask mask revision

#	movb %cl,X86_MASK
#	movl %edx,X86_CAPABILITY
   pushl %ebx
   movl $0x80603, %ebx
   movb %cl, %es:(%ebx)
   movl $0x8060C, %ebx
   movl %edx, %es:(%ebx)
   popl %ebx

is486:
	movl %cr0,%eax		# 486 or better
	andl $0x80000011,%eax	# Save PG,PE,ET
	orl $0x50022,%eax	# set AM, WP, NE and MP
	jmp 2f

is386:	pushl %ecx		# restore original EFLAGS
	popfl
	movl %cr0,%eax		# 386
	andl $0x80000011,%eax	# Save PG,PE,ET
	orl $2,%eax		# set MP

2:	movl %eax,%cr0

/*
 * We depend on ET to be correct. This checks for 287/387.
 */
/*check_x87:
	movb $0,X86_HARD_MATH
	clts
	fninit
	fstsw %ax
	cmpb $0,%al
	je 1f
	movl %cr0,%eax		/* no coprocessor: have to set bits */
/*	xorl $4,%eax		/* set EM */
/*	movl %eax,%cr0
	ret
	ALIGN
1:	movb $1,X86_HARD_MATH
	.byte 0xDB,0xE4		/* fsetpm for 287, ignored by 387 */
//	ret

# now we try to switch to the new gdt (paging) table
# Then, linear adress should be 0xC0000000.
  cli

  lgdt _gdt_desc                         # reload new gdt
  lidt idt_descr                         # load idt

  ljmp $0x10, $new_gdt                  # set cs
new_gdt:

  xor %eax, %eax                        # reload all the segment registers
                                        # after changing gdt.
  movl $0x18, %eax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss

  # Ok, Paging is active. Go on with system initialisation...
  call _start_kernel

.align 4
setup_paging:
  movl $1024*2, %ecx                    # clear page dir and table 0
  xorl %eax, %eax
  movl $0x1000, %edi
  cld
  rep
  stosl

  movl $0x2007 , %ds:(0x1000)           # 1:1 translation
  movl $0x2007 , %ds:(0x1000+3072)      # the real place is 0xC0000000

  movl $0x2FFC , %edi                   # address of page0 + 4092
  movl $0x3ff007, %eax                  # size to fill the table
  std
1:stosl
  subl $0x1000, %eax
  jge 1b
  cld
  movl $0x1000, %eax

  movl %eax, %cr3                       # activate paging mechanism
  movl %cr0, %eax
  orl $0x80000000, %eax
  movl %eax, %cr0                       # set paging bit

  ret

.align 4
setup_idt:
  lea ignore_int, %edx
  movl $0x10 << 16, %eax
  movw %dx, %ax
  movw $0x8E00, %dx

  lea _idt, %edi
  mov $256, %ecx
rp_sidt:
  movl %eax, (%edi)
  movl %edx, 4(%edi)
  addl $8,   %edi
  dec %ecx
  jne rp_sidt
  ret

.align 4
int_msg:
  .asciz "\nUnknown interrupt occured.\nSystem halted.\n"

.macro SAVE_ALL
	push %gs
	push %fs
	push %es
	push %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	movl $0x18, %eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
        nop
.endm


.macro RESTORE_ALL
	popl %ebx
	popl %ecx
	popl %edx
        popl %esi
	popl %edi
	popl %ebp
	popl %eax
	pop %ds
	pop %es
	pop %fs
   pop %gs
.endm

.align 4
_do_irq_unknown:
ignore_int:
  movw 0x378, %dx
  movb 0x5, %al

  outb %al, %dx

  SAVE_ALL




  pushl $int_msg
  call _printk

  RESTORE_ALL

  die: jmp die
  iret


.align 4
_do_irq_floppy:
  SAVE_ALL
  call _floppy_interrupt
  RESTORE_ALL

  pushl %eax
  movb $0x20, %al
  outb %al, $0x20
  popl %eax

  iret

.align 4
_do_irq_el3:
  SAVE_ALL
  call _el3_interrupt
  RESTORE_ALL

  pushl %eax
  movb $0x20, %al
  outb %al, $0x20
  popl %eax

  iret

.align 4
_do_irq_ide:
  SAVE_ALL
        pushl %eax              # push eax

        pushf                   # clear nested task flag
        pop %eax
        andl $0xBFFF, %eax
        push %eax
        popf                    # done

        popl %eax
  call _ide_interrupt
  RESTORE_ALL

  pushl %eax
  movb $0x20, %al
  outb %al, $0x20
  popl %eax

  iret

.align 4
_do_irq_mouse1:
  SAVE_ALL
        pushl %eax              # push eax

        pushf                   # clear nested task flag
        pop %eax
        andl $0xBFFF, %eax
        push %eax
        popf                    # done

        popl %eax

  call _mouse1_interrupt

  RESTORE_ALL

  pushl %eax
  movb $0x20, %al
  outb %al, $0x20
  popl %eax

  iret

.align 4
_do_irq_mouse2:
  SAVE_ALL

  call _mouse2_interrupt

  RESTORE_ALL

  pushl %eax
  movb $0x20, %al
  outb %al, $0x20
  popl %eax

  iret

.align 4
_do_irq_timer:
  cli

  SAVE_ALL
  pushl %eax

  movb $0x20, %al
  outb %al, $0x20

  popl %eax
  call _do_timer

  RESTORE_ALL

        pushl %eax              # push eax

        pushf                   # clear nested task flag
        pop %eax
        andl $0xBFFF, %eax
        push %eax
        popf                    # done

        popl %eax
  iret

_do_irq_sb:
  SAVE_ALL
        pushl %eax              # push eax

        pushf                   # clear nested task flag
        pop %eax
        andl $0xBFFF, %eax
        push %eax
        popf                    # done

        popl %eax

  call _sb_interrupt

  RESTORE_ALL

  pushl %eax
  movb $0x20, %al
  outb %al, $0x20
  popl %eax

  iret

.align 4
_do_irq_key:

  SAVE_ALL
        pushl %eax              # push eax

        pushf                   # clear nested task flag
        pop %eax
        andl $0xBFFF, %eax
        push %eax
        popf                    # done

        popl %eax

  call _key_interrupt
  RESTORE_ALL

  pushl %eax
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

.align 4
_do_nothing:
  jmp _do_nothing

  SAVE_ALL

  pushl $200
  pushl %esp
  call _mem_dump
  jop:
    jmp jop



  movb $0x20, %al
  outb %al, $0x20

  pushf
  pop %eax
  andl $0xBFFF, %eax
  push %eax
  popf

  popl %eax

  iret

_do_irq_detect_3:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_3
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_4:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_4
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_5:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_5
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_6:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_6
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_7:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_7
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_8:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_8
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_9:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_9
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_10:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_10
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_11:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_11
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_12:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_12
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_13:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_13
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_14:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_14
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_15:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_15
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

_do_irq_detect_16:
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  push %ds
  push %es
  push %fs
  movl $0x18, %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  call _irq_detect_16
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  movb $0x20, %al
  outb %al, $0x20
  popl %eax
  iret

.align 4
_reboot:
   cli
   lidt idt_null_desc                         # load idt
   int $0
   jmp _reboot

.align 4
_loop:
   xor %eax, %eax
   xor %ebx, %ebx
   mov %ds, %ax
   mov $0x2b, %bx
   cmp %ax, %bx

   je _loop
   int $0x0
   jmp _loop



.align 4
.word 0
idt_null_desc:
    .word 256*8-1
    .long 0xC0000000+_idt_null

.align 4
.word
_idt_null:
    .fill 256,8,0

.align 4
.word 0
idt_descr:
    .word 256*8-1
    .long 0xC0000000+_idt

.align 4
_idt:
    .fill 256,8,0

.align 4
.word 0
_gdt_desc:
	.word (8*2*NR_TASKS)*8-1
	.long 0xC0000000+_gdt	        # gdt base = 0xCxxxxxxxx

.align 4
_gdt:
	.quad   0x0000000000000000        # Null descriptor
        .quad   0x0000000000000000        # not used
        .quad   0xc0c39a000000ffff        # 0x10 kernel 1GB code at 0xC0000000
        .quad   0xc0c392000000ffff        # 0x18 kernel 1GB data at 0xC0000000
        .quad   0x00cbfa000000ffff        # 0x23 user   3GB code at 0x00000000
        .quad   0x00cbf2000000ffff        # 0x2b user   3GB data at 0x00000000

        .quad   0x0000000000000000
        .quad   0x0000000000000000
        .fill 2*NR_TASKS,8,0              # space for LDT's and TSS's etc

.align 4
errorcode:
  .long 0
eip:
  .long 0
ecs:
  .long 0
error_cs:
  .long 0
error_flags:
  .long 0
