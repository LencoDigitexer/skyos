/************************************************************************/
/* Sky Operating System V2.0j
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\kernel.s
/* Last Update: 12.11.1998
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
/************************************************************************/
NR_TASKS = 20
.globl text, _reboot, _idt, _gdt, _do_irq_timer, _do_irq_key, _do_nothing, _setpixel, _do_irq_floppy, _do_irq_ide
.globl _do_irq_detect_3,_do_irq_detect_4,_do_irq_detect_5,_do_irq_detect_6,_do_irq_detect_7
.globl _do_irq_detect_8,_do_irq_detect_9,_do_irq_detect_10,_do_irq_detect_11,_do_irq_detect_12
.globl _do_irq_detect_13,_do_irq_detect_14,_do_irq_detect_15,_do_irq_detect_16
.globl _do_page_fault, _object_map, _do_irq_el3, _do_irq_unknown, _do_irq_mouse
.text

start_32:
  jmp start_over_data

_object_map:
  .long 0xFFFFFFFF

start_over_data:

# set all needed selectors
  xor %eax, %eax
  movl $0x18, %eax
  movw %ax, %ds
  movw %ax, %es

# set kernel-processor stack
#  lss _stack_start, %esp
  movw %ax, %ss
  movl $0x1FFF00, %esp

  pushl %eax
  popl %eax

  xor %ebx, %ebx

mem_test2:
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

.align 4
_do_irq_unknown:
ignore_int:
  cld
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
  pushl $int_msg
  call _printk
  popl %eax
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  popl %eax
  die: jmp die
  iret

.align 4
_do_page_fault:
  cld
  popl errorcode
  popl eip
  pushl eip
  pushl errorcode
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
  mov %cr2, %eax
  pushl eip
  pushl %eax
  call _page_fault

_do_irq_floppy:
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
  call _floppy_interrupt
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

_do_irq_el3:
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
  call _el3_interrupt
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

_do_irq_ide:
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
  call _ide_interrupt
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


_do_irq_timer:
  pushl %eax
  movb $0x20, %al
  outb %al, $0x20
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
  call _do_timer
  pop %fs
  pop %es
  pop %ds
  popl %edx
  popl %ecx
  popl %ebx
  popl %eax
  iret

_do_irq_key:
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
  call _key_interrupt
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

_do_irq_mouse:
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
  call _mouse_interrupt
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

_do_nothing:
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
_setpixel:

   xor %eax, %eax
   xor %ebx, %ebx
   xor %ecx, %ecx
   xor %edx, %edx

   mov -8(%ebp),   %ax
   mov $100,  %dx
   mul %dx
   mov -12(%ebp),   %bx
   mov %bl, %cl
   shr $1, %bx
   shr $1, %bx
   shr $1, %bx
   add %ax, %bx
   and $7, %cl
   xor $7, %cl
   mov $1, %ah
   shl %cl, %ah
   mov $0x3ce, %dx
   mov $8, %al
   out %ax, %dx
   mov $517, %ax
   out %ax, %dx

   mov %ds, %ax
   mov %ax, %es
   add $0xA0000, %ebx
   movb %es:(%ebx), %al
   mov -4(%ebp), %ax
   movb %al, %es:(%ebx)
   mov $65288, %ax
   out %ax, %dx
   mov $5, %ax
   out %ax, %dx
   ret

.align 4
_reboot:
   cli
   lidt idt_null_desc                         # load idt
   int $0
   jmp _reboot


.align 4
.word 0
idt_null_desc:
    .word 256*8-1
    .long 0xC0000000+_idt_null
//    .long _idt_null

.align 4
.word
_idt_null:
    .fill 256,8,0

.align 4
.word 0
idt_descr:
    .word 256*8-1
    .long 0xC0000000+_idt
//    .long _idt

.align 4
_idt:
    .fill 256,8,0

.align 4
.word 0
_gdt_desc:
	.word (8*2*NR_TASKS)*8-1
	.long 0xC0000000+_gdt	        # gdt base = 0xCxxxxxxxx
//	.long _gdt	        # gdt base = 0xCxxxxxxxx

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






