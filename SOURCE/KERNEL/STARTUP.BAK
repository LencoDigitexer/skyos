;/************************************************************************/
;/* Sky Operating System V2.0j
;/* Copyright (c) 1996 -1998 by Szeleney Robert
;/*
;/* Project Members: Szeleney Robert
;/*                  Resl Christian
;/*                  Hayrapetian Gregory
;/************************************************************************/
;/* File       : kernel\startup.asm
;/* Last Update: 31.05.1999
;/* Version    : beta
;/* Coded by   : Szeleney Robert
;/* Docus      : Internet, Linux, Minix, all other operating systems,...
;/************************************************************************/
;/* Definition:
;/*   Initial startup code. 16 Bit.
;/*   Loaded at adress 0x90000
;/*   Segment is 0x9000
;/*   Offset  is 0
;/*   Initalizes Protected Mode and Jumps to System at adress 0x10000
;/*   (seg 0x1000 : off 0x0000)
;/*
;/* Bugs:
;/*   Warning! Startup code should be lesser than 1 block.
;/*   Setup/Bootcode not tested for more than 1 block.
;/************************************************************************/
;/*
;/* Update 31.05.1999 (Szeleney Robert)
;/*   - Added support to autodetect VESA2.0 card. If no VESA Card is found
;/*       svga_init() in svgadev.c searchs for specified-driver card.
;/*   - Resolution selection in VESA2.0 mode now available.
;/*     TODO: Allow selection of all supported modes.
;/*
;/************************************************************************/


;org 0x00
MEMORY_EXTENDED equ 0x80000
[bits 16]
jmp start16


;Routine to print str at DS:SI to screen using int 0x10
prtstr:	lodsb
	and	al,al
	jz	fin
	call	prtchr
	jmp	prtstr
fin:	ret

; Space printing

prtsp2:	call	prtspc		; Print double space
prtspc:	mov	al,0x20	        ; Print single space (fall-thru!)

prtchr:	push	ax
	push	cx
	xor	bh,bh
	mov	cx,0x01
	mov	ah,0x0e
	int	0x10
	pop	cx
	pop	ax
	ret

beep:	mov	al,0x07
	jmp	prtchr

wait_key:
        xor ax, ax
        int 0x16
        ret



start16:
  cli
;we know cs = 0x9000, set ds and es
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 0xFFF0
  mov bp, 0xFFF0

  mov si, msg_init
  call prtstr

;check if kernel was loaded correct
  mov ax, 0x1000
  mov es, ax

  mov bx, 2
  mov byte al, [es:bx]
  cmp al, 0xAA
  je kernel_ok

  mov ax, ds
  mov es, ax

  mov si, msg_corrupt_kernel
  call prtstr

  stop:
    jmp stop

kernel_ok:
  mov ax, ds
  mov es, ax

  mov si, msg_search_cards
  call prtstr

  mov si, msg_vga16
  call prtstr

;check vesa-mode
  mov ax, 0x8000
  mov es, ax
  mov bx, 0x1000

  mov byte [es:bx],   'V'
  mov byte [es:bx+1], 'B'
  mov byte [es:bx+2], 'E'
  mov byte [es:bx+3], '2'

  mov ax, 0x4f00
  mov di, 0x1000
  int 0x10

  cmp ax, 0x004f
  jne no_vesa_found

; check Protected Mode info (VESA 2.0)
  mov ax,0x4f0a
  mov bx,0
  int 0x10

  cmp ax, 0x004f
  je vesa_found

no_vesa_found:
  mov ax, 0x8000
  mov es, ax
  mov word [es:0x496], 0        ; VESA2.0 not supported

  mov si, msg_no_vesa
  call prtstr

  mov si, msg_select
  call prtstr

no_v:
  call wait_key

  cmp al, 0x31
  je mode12

  cmp al, 0x32
  je mode6a

  jmp no_v


mode6a:
  mov ax, 0x8000
  mov es, ax
  mov word [es:0x496], 0x6a
  mov ah, 0
  mov al, 0x6a
  int 0x10
  jmp vesa_end

mode12:
  mov ax, 0x8000
  mov es, ax
  mov word [es:0x496], 0x12
  mov ah, 0
  mov al, 0x12
  int 0x10
  jmp vesa_end

vesa_found:
  mov si, msg_vesa
  call prtstr

  mov bx, es
  mov cx, di

  mov ax, 0x8000
  mov es, ax
  mov word [es:0x500], cx
  mov word [es:0x502], bx


  mov si, msg_select
  call prtstr

res:
  call wait_key

  cmp al, 0x31
  je mode12
  cmp al, 0x32
  je mode6a
  cmp al, 0x33
  je res640
  cmp al, 0x34
  je res800
  cmp al, 0x35
  je res1024
  cmp al, 0x36
  jne res

  mov ax, 0x8000
  mov es, ax
  mov word [es:0x496], 0x107        
  mov bx, 0x107
  jmp resok

res1024:
  mov ax, 0x8000
  mov es, ax
  mov word [es:0x496], 0x105      
  mov bx, 0x105
  jmp resok

res800:
  mov ax, 0x8000
  mov es, ax
  mov word [es:0x496], 0x103       
  mov bx, 0x103
  jmp resok

res640:
  mov ax, 0x8000
  mov es, ax
  mov word [es:0x496], 0x101       
  mov bx, 0x101
  jmp resok

resok:

; VESA mode informations
  mov ax, 0x4f01
  mov cx, bx
  mov bx, 0x1500
  mov di, bx
  int 0x10

; 102h or 6Ah      800x600x16
; 103h     -       800x600x256
; 105h     -       1024x768x256
; 107h     -       1280x1024x256

; Init VESA Mode
  mov ax, 0x4f02
  mov bx, cx
  add bx, 0x4000
  int 0x10

vesa_end:
;reset the disk-controller
  mov	ax,0x0000
  mov	dl,0x80
  int	0x13

  mov ax, ds
  mov es, ax

;now we want to move to protected mode
  cli			              ; no interrupts allowed !

; now we enable A20
  call	empty_8042
  mov	al,0xD1		              ; command write
  out	0x64,al
  call	empty_8042
  mov	al,0xDF		              ; A20 on
  out	0x60,al
  call	empty_8042

; reset coprocessor if present
  xor	ax,ax
  out	0xf0,al
  call	delay
  out	0xf1,al
  call	delay

; reprogramm interrupts
; all interrupts are moved behind exception and reserved interrupts.
; int 0 == int 0x20
; int 1 == int 0x21
; .
; .
; .
  mov     al,0x11
  out     0x20,al
  call    delay
  out     0xA0,al
  call    delay
  mov  al,0x20
  out     0x21,al
  call    delay
  mov  al,0x28
  out     0xA1,al
  call    delay
  mov  al,0x04
  out     0x21,al
  call    delay
  mov  al,0x02
  out     0xA1,al
  call    delay
  mov  al,0x01
  out     0x21,al
  call    delay
  out  0xA1,al
  call    delay
  mov  al,0xFF
  out     0xA1,al
  call    delay
  mov  al,0xFB
  out     0x21,al

; setup descipter tables
  o32 lidt [idt_48]
  o32 lgdt [gdt_48]

  xor   ax, ax
  mov	ax,1		        ; set PE
  lmsw	ax		        ; initialize

  jmp	flush_instr
align 4
flush_instr:
; now we are in 16-Bit protected mode, segment register not set, we have to do
; this now
  mov ax, 0x0018
  mov ds, ax

  mov ax, 0x0020
  mov es, ax

  db 0x66, 0xea         ; jmp 0x1000:0x0000
  dd 0x10000
  dw 0x10               ; jump into the kernel at adress 0x10000.

; This routine checks that the keyboard command queue is empty
; (after emptying the output buffers)
;
; No timeout is used - if this hangs there is something wrong with
; the machine, and we probably couldn't proceed anyway.
empty_8042:
	call	delay
	in	al,0x64	                ; 8042 status port
	test	al,1		        ; output buffer?
	jz	no_output
	call	delay
	in	al,0x60	                ; read it
	jmp	empty_8042
no_output:
	test	al,2    		; is input buffer full?
	jnz	empty_8042	        ; yes - loop
	ret

msg_init         db "SKY Operating System pre-initialization. (startup release 2.1.1a)",13,10
msg_kernel_check db "- Searching Kernel in RAM...",13,10,0
msg_search_cards db "- Searching VESA2.0 compatibility...",13,10,13,10,0

msg_vga16        db "   - Standard VGA 4-Bit",13,10
                 db "     Please select resolution: ",13,10
                 db "     <1>   640x480x16 ",13,10
                 db "     <2>   800x600x16",13,10,0

msg_no_vesa      db 13,10,"   - No VESA 2.0 compatibility detected",13,10,0
msg_vesa         db 13,10,"   - Using VESA2.0 video mode... ",13,10
                 db "     Linear Frame Buffering deactivated.",13,10
                 db "     Please select resolution: ",13,10
                 db "     <3>   640x480x256 (not sure this mode works, email me)",13,10
                 db "     <4>   800x600x256",13,10
                 db "     <5>  1024x768x256",13,10
                 db "     <6>  1280x1024x256",13,10,13,10,0

;msg_select       db 13,10,13,10,"     Please mail me if your card works with SkyOS or not. Thanks.",13,10
;                 db "     E-Mail:bertlman@gmx.at",13,10
;                 db "     If you have no VESA2.0 and no Trident SVGA Card,",13,10
;                 db "     SKY is not able to start. <press a key>",10,13,0*/
msg_select       db 13,10,13,10,"Select a graphic mode... ",0

msg_corrupt_kernel
                 db "Warning!! Corrupt kernel detected. This means, startup code is not able to",13,10
	         db "load the kernel, or kernel checksum is incorrect.",13,10
                 db "Please contact bertlman@gmx.at",13,10,0

align 4
delay:
	dw	0x00eb			; jmp $+2
	ret

align 4
gdt:
	dw	0,0,0,0		; dummy

	dw	0,0,0,0		; unused

	dw	0xFFFF		; 4Gb - (0x100000*0x1000 = 4Gb)
	dw	0x0000		; base address=0
	dw	0x9A00		; code read/exec
	dw	0x00CF		; granularity=4096, 386 (+5th nibble of limit)

	dw	0xFFFF		; 4Gb - (0x100000*0x1000 = 4Gb)
	dw	0x0000		; base address=0
	dw	0x9200		; data read/write
	dw	0x00CF		; granularity=4096, 386 (+5th nibble of limit)

	dw	0xFFFF		; 4Gb - (0x100000*0x1000 = 4Gb)
	dw	0x8000		; base address=0
	dw	0x920B		; data read/write
	dw	0x0000		; granularity=4096, 386 (+5th nibble of limit)

align 4
idt_48:
	dw	0		; idt limit=0
	dw	0,0		; idt base=0L

align 4
gdt_48:
	dw	0x28		; gdt limit=2048, 256 GDT entries
	dw	gdt,0x9	        ; gdt base = 0X9xxxx



