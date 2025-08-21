;***************************************************************************;
;* SKY Systems Bootroutine                                                 *;
;***************************************************************************;
;* Project Members: ˛ Christian Resl 12.11.1979                            *;
;*                    Username: Chris                                      *;
;*                  ˛ Robert Szeleney 04.06.1980                           *;
;*                    Username: Bertl                                      *;
;***************************************************************************;
;* Mainfile:    SETUP.CPP (SETUP.EXE)                                      *;
;* File:        BOOTCODE.ASM (BOOTCODE.DAT)                                *;
;* Description: The Bootroutine tries to load the Kernel SKYOS.SYS into    *;
;*              memory at location 1000:0000,                              *;
;*              and the startup code (START.SYS) at location 9000:0000     *;
;*              To generate the file BOOTCODE.DAT remove the EXE-Header    *;
;*              from BOOTCODE.EXE and rename the file to BOOTCODE.DAT.     *;
;***************************************************************************;
;* Created:     23.02.1997                                                 *;
;* Last Update: 11.06.1998                                                 *;
;***************************************************************************;

BOOTCODE_OFFS equ 7C00h
BUFFER_SEG    equ 5000h         ;Zwischenspeicher fÅr Blîcke an 5000:0000
KERNEL_SEG    equ 1000h         ;Kernel wird an Adresse 1000:0000 geladen
START_SEG     equ 9000h         ;Startcode wird an 9000:0000 geladen

iNode struc
        Name       DB 50 dup(?)
        Type       DB  3 dup(?)
        Owner      DB  2 dup(?)
        Rights     DB  2 dup(?)
        Attributes DB  1 dup(?)
        CreateTime DB  7 dup(?)
        AccessTime DB  7 dup(?)
        UpdateTime DB  7 dup(?)
        Filesize   DB  4 dup(?)
        Indirect1  DB  4 dup(?)
        Indirect2  DB  4 dup(?)
        Indirect3  DB  4 dup(?)
        HigherNode DB  4 dup(?)
        NextNode   DB  4 dup(?)
        LowerNode  DB  4 dup(?)
iNode ends

CODE SEGMENT
        assume  CS:CODE
.286

START:  jmp     BOOTROUTINE

FileSystem  DB 8 dup(0)                 ;FilesystemDescriptor
MediaTyp    DB 0                        ;MediaDescriptor
SectorSize  DB 0
MaxTrack    DB 0
MaxHead     DB 0
MaxSector   DB 0
BeginTrack  DB 0                        ;VolumeDescriptor
BeginHead   DB 0
BeginSector DB 0
EndTrack    DB 0
EndHead     DB 0
EndSector   DB 0
BlockSize   DB 0

BOOTROUTINE:

       mov ax, cs
       mov ds, ax
       mov ax, 0b800h
       mov es, ax
       xor di, di
       mov si, offset BootMessage
       cld                                       ;Fehlermeldung ausgeben
       mov     AH,0Eh
put:   lodsb
       cmp     AL,0
       je      putende
       int     10h
       jmp     put
putende:

        cli                                     ;Initialisierung
        xor     AX,AX
        mov     BP,BOOTCODE_OFFS
        mov     SS,AX
        mov     SP,BP
        sti
        push    BUFFER_SEG
        pop     ES
        push    CS
        pop     DS
.386
        mov     EAX,1                           ;Rootkatalog lesen
.286
        push    ES
        call    ReadBlock
        pop     ES
.386
        mov     EAX,dword ptr ES:[iNode.LowerNode]    ;LowerNode-Zeiger in EAX

TRYNEXT:
        cmp     EAX,0
.286
        je      ERROR
.386
        push    EAX
.286
        push    ES
        call    ReadBlock
        pop     ES
.386
        pop     EDI
        shr     EDI,24
.286
        shl     DI,7                            ;ES:DI auf den aktuellen iNode
.386
        mov     EAX,dword ptr ES:[DI+(iNode.NextNode)] ;NextNode-Zeiger in EAX
.286
        mov     SI,offset StartName            ;DS:SI auf den StartupNamen
        add     SI,BP
        mov     CX,6
        repe    cmpsb                           ;Namensvergleich (START)
        jne     TRYNEXT
        add     DI,44                           
        mov     CX,3
        repe    cmpsb                           ;Typvergleich (SYS)
        jne     TRYNEXT                         
.386
        sub     DI,53
        mov     EAX,dword ptr ES:[DI+(iNode.Indirect1)]  ;Kernel laden
        cmp     EAX,0
.286
        je      ERROR
        push    ES
        call    ReadBlock
        pop     DS
        xor     SI,SI
        push    START_SEG
        pop     ES
        cld
.386
LOAD:   lodsd
        cmp     EAX,0
.286
        je      RUN
        push    SI
        call    ReadBlock
        pop     SI
        jmp     LOAD
RUN:    DB 0EAh,000h,000h,000h,090h
       ;jmp     9000:0000                       ;Start-Code ausfÅhren

ERROR:  mov     SI,offset ErrorMessage          ;DS:SI auf Fehlermeldung
        add     SI,BP
        cld                                     ;Fehlermeldung ausgeben
        mov     AH,0Eh
OUTPUT: lodsb
        cmp     AL,0
        je      ENDE
        int     10h
        jmp     OUTPUT
ENDE:   xor     AH,AH                           ;Auf Tastendruck warten
        int     16h
        int     19h                             ;Neustart

ReadBlock proc near                             ;EAX...BN   ES...Pufferzeiger

.386
        and     EAX,00FFFFFFh                   ;(BN-1) * BS in EAX
        dec     EAX
        xor     EBX,EBX
.286
        mov     BL,CS:[BP+offset BlockSize]
.386
        mul     EBX
.286
        push    BX

        mov     BX,word ptr CS:[BP+offset BeginHead]            ;ZN in BX
        shr     BL,6
        ror     BX,6
        mov     BL,CS:[BP+offset BeginTrack]
        mov     CX,word ptr CS:[BP+offset BeginSector]          ;SN in CX
        and     CX,003Fh
        mov     DX,word ptr CS:[BP+offset BeginHead]            ;KN in DX
        and     DX,003Fh
        mov     SI,word ptr CS:[BP+offset MaxSector]            ;SA in SI
        and     SI,003Fh
        mov     DI,word ptr CS:[BP+offset MaxHead]              ;KA in DI
        and     DI,003Fh
LBL1:
.386
        cmp     EAX,0
.286
        je     LBL2
.386
        dec     EAX
.286
        inc     CX                              ;SN, KN, ZN weiterzÑhlen
        cmp     CX,SI
        jle     LBL1
        mov     CX,1
        inc     DX
        cmp     DX,DI
        jle     LBL1
        xor     DX,DX
        inc     BX
        jmp     LBL1
LBL2:   pop     AX
        cmp     AX,0
        je      LBL3
        dec     AX
.386
        pusha                                   ;Alle Register auf Stack
.286
        mov     CH,BH                           ;SN aufteilen
        shl     CL,2
        shr     CX,2
        mov     DH,BH                           ;KN aufteilen
        shl     DL,2
        shr     DH,2
        shl     DX,6
        mov     CH,BL                           ;ZN aufteilen
READ:   mov     AX,0201h                        ;Sektor lesen
        mov     DL,CS:[BP+offset MediaTyp]
        xor     BX,BX
        int     13h
        jnc     READOK                          ;Fehler?
        xor     AH,AH                           ;Controller-Reset
        int     13h
        jmp     READ
READOK: mov     AX,ES                           ;Pufferzeiger weiterschieben
        add     AX,20h
        mov     ES,AX
.386
        popa                                    ;Alle Register vom Stack
.286
        push    AX
.386
        mov     EAX,1
.286
        jmp     LBL1
LBL3:   ret                                     ;ReadBlock beenden

ReadBlock endp

KernelName   DB 'SKYOS',000h,'SYS'
StartName  DB 'START',000h,'SYS'
BootMessage  DB 00Ah,00Dh,'Sky Operating System V2.0a is booting...',00Ah,00Dh,000h
ErrorMessage DB 00Ah,00Dh,'Missing Operating System',00Ah,00Dh
             DB 'Change and press any key...',00Ah,00Dh,000h
org 510
ID_Code      DB 055h,0AAh

CODE ENDS
END START
