/************************************************************************/
/* Sky Operating System V2.0j
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\syscall.s
/* Last Update: 10.02.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux, Minix, all other operating systems,...
/************************************************************************/
/* Definition:
/*  Kernel Entry points for syscall's and interrupts/exceptions.
/************************************************************************/
.global _do_syscall

.text
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


function_address:
.long 0

_do_syscall:
/*        cli */
/*        SAVE_ALL

        movl 24(%esp), %eax     # push user-message address
        pushl %eax

        call _syscall           # coded in syshandl.c

        popl %ebx               # pop C-function parameter

        RESTORE_ALL
  */
        pushl %eax              # push eax

        pushf                   # clear nested task flag
        pop %eax
        andl $0xBFFF, %eax
        push %eax
        popf                    # done

        popl %eax

        iret
