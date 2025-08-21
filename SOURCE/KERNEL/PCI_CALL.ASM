%define d dword
%define w word

[bits 32]
[section .text]

global _PCIBIOS_call
extern _pci_farcall
extern _pci_farcall_data

align 4
_PCIBIOS_call:
	push	ebp
	mov	ebp,esp

	pushad

	;; 0 = ebp
	;; 4 = ret
	;; 8 = inregs
	;; 12 = outregs
	;; 16 = ds_data

	push	ebp
	push	es
	push	ds

	mov	esi,d[ebp+8]
	mov	edi,.k01

	inc	edi
	movsd
	inc	edi
	movsd
	inc	edi
	movsd
	inc	edi
	movsd
	inc	edi
	movsd
	inc	edi
	movsd
	inc	edi
	movsd

	mov	ds,w[cs: _pci_farcall_data]
	mov	es,w[cs: _pci_farcall_data]

.k01:
	db	0xb8, 0,0,0,0
	db	0xbb, 0,0,0,0
	db	0xb9, 0,0,0,0
	db	0xba, 0,0,0,0
	db	0xbe, 0,0,0,0
	db	0xbf, 0,0,0,0
	db	0xbd, 0,0,0,0

	push	cs
	push	long .l01
	push	d[cs: _pci_farcall+4]		;; pci_cs
	push	d[cs: _pci_farcall+0]		;; pci_offset
	retf

.l01:
	pop	ds
	pop	es
	pop	ebp

	push	ebx
	mov	ebx,d[ebp+12]
	mov	d[ebx+0],eax
	pop	eax
	mov	d[ebx+4],eax

	mov	d[ebx+8],ecx
	mov	d[ebx+12],edx
	mov	d[ebx+16],esi
	mov	d[ebx+20],edi
	mov	d[ebx+24],ebp

	pushfd
	pop	eax
	mov	d[ebx+28],eax

	popad
	pop	ebp
	ret
