	ifndef	??version
?debug	macro
	endm
publicdll macro	name
	public	name
	endm
	endif
	?debug	V 300h
	?debug	S "DISKSYS.CPP"
	?debug	C E9EC65C1220B4449534B5359532E435050
	?debug	C E94019CB1817433A5C424333315C494E434C5544455C535444494F+
	?debug	C 2E48
	?debug	C E94019CB1817433A5C424333315C494E434C5544455C5F44454653+
	?debug	C 2E48
	?debug	C E94019CB1818433A5C424333315C494E434C5544455C5F4E46494C+
	?debug	C 452E48
	?debug	C E94019CB1817433A5C424333315C494E434C5544455C5F4E554C4C+
	?debug	C 2E48
	?debug	C E94019CB1817433A5C424333315C494E434C5544455C434F4E494F+
	?debug	C 2E48
	?debug	C E94019CB1818433A5C424333315C494E434C5544455C535452494E+
	?debug	C 472E48
	?debug	C E94019CB1819433A5C424333315C494E434C5544455C50524F4345+
	?debug	C 53532E48
	?debug	C E94019CB1817433A5C424333315C494E434C5544455C46434E544C+
	?debug	C 2E48
	?debug	C E94019CB1816433A5C424333315C494E434C5544455C42494F532E+
	?debug	C 48
	?debug	C E94019CB1814433A5C424333315C494E434C5544455C494F2E48
	?debug	C E94019CB1815433A5C424333315C494E434C5544455C444F532E48
_TEXT	segment byte public 'CODE'
_TEXT	ends
DGROUP	group	_DATA,_BSS
	assume	cs:_TEXT,ds:DGROUP
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
_BSS	ends
_DATA	segment word public 'DATA'
_Drive	label	word
	db	0
	db	0
_Head	label	word
	db	0
	db	0
_Track	label	word
	db	0
	db	0
_Sector	label	word
	db	1
	db	0
_Partition	label	word
	db	0
	db	0
_DATA	ends
_BSS	segment word public 'BSS'
_EndHead	label	word
	db	2 dup (?)
_EndTrack	label	word
	db	2 dup (?)
_EndSector	label	word
	db	2 dup (?)
_BSS	ends
_TEXT	segment byte public 'CODE'
	?debug	C E8010B4449534B5359532E435050EC65C122
   ;	
   ;	void Disk(int cmd,int drive,int head,int track,int sector,int nsects, void *buffer)
   ;	
	?debug	L 77
	assume	cs:_TEXT
@Disk$qiiiiiipv	proc	near
	?debug	B
	push	bp
	mov	bp,sp
	push	si
	?debug	C E318000200150104
	?debug	C E606627566666572180A100000066E7365637473+
	?debug	C 040A0E000006736563746F72040A0C0000057472+
	?debug	C 61636B040A0A00000468656164040A0800000564+
	?debug	C 72697665040A06000003636D64040A040000
	?debug	B
   ;	
   ;	{
   ;	 int i;
   ;	 for (i=0;i<MAX_TRY;i++)
   ;	
	?debug	L 80
	xor	si,si
	jmp	short @1@114
@1@58:
   ;	
   ;	 {
   ;	  if (biosdisk(cmd,drive,head,track,sector,nsects,buffer) == 0) return;
   ;	
	?debug	L 82
	push	word ptr [bp+16]
	push	word ptr [bp+14]
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	word ptr [bp+4]
	call	near ptr _biosdisk
	add	sp,14
	or	ax,ax
	je	short @1@170
	?debug	L 80
	inc	si
@1@114:
	cmp	si,5
	jl	short @1@58
   ;	
   ;	 }
   ;	 printf("\n\nERROR: Not able to read/write disksector.\n");
   ;	
	?debug	L 84
	mov	ax,offset DGROUP:s@
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	 exit(1);
   ;	
	?debug	L 85
	mov	ax,1
	push	ax
	call	near ptr _exit
	pop	cx
@1@170:
   ;	
   ;	}
   ;	
	?debug	L 86
	pop	si
	pop	bp
	ret	
	?debug	C E6016904080106002F0004060003636D64040A04+
	?debug	C 0000056472697665040A0600000468656164040A+
	?debug	C 08000005747261636B040A0A000006736563746F+
	?debug	C 72040A0C0000066E7365637473040A0E00000662+
	?debug	C 7566666572180A100000
	?debug	E
	?debug	E
@Disk$qiiiiiipv	endp
   ;	
   ;	void Layout()
   ;	
	?debug	L 88
	assume	cs:_TEXT
@Layout$qv	proc	near
	?debug	B
	?debug	B
   ;	
   ;	{
   ;	 window(1,1,80,25);
   ;	
	?debug	L 90
	mov	ax,25
	push	ax
	mov	ax,80
	push	ax
	mov	ax,1
	push	ax
	push	ax
	call	near ptr _window
	add	sp,8
   ;	
   ;	 textcolor(7);
   ;	
	?debug	L 91
	mov	ax,7
	push	ax
	call	near ptr _textcolor
	pop	cx
   ;	
   ;	 textbackground(1);
   ;	
	?debug	L 92
	mov	ax,1
	push	ax
	call	near ptr _textbackground
	pop	cx
   ;	
   ;	 clrscr();
   ;	
	?debug	L 93
	call	near ptr _clrscr
   ;	
   ;	 gotoxy(28,1);
   ;	
	?debug	L 94
	mov	ax,1
	push	ax
	mov	ax,28
	push	ax
	call	near ptr _gotoxy
	pop	cx
	pop	cx
   ;	
   ;	 printf("SKY Systems Setup 1.0a");
   ;	
	?debug	L 95
	mov	ax,offset DGROUP:s@+45
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	 window(1,2,80,25);
   ;	
	?debug	L 96
	mov	ax,25
	push	ax
	mov	ax,80
	push	ax
	mov	ax,2
	push	ax
	mov	ax,1
	push	ax
	call	near ptr _window
	add	sp,8
   ;	
   ;	 textcolor(7);
   ;	
	?debug	L 97
	mov	ax,7
	push	ax
	call	near ptr _textcolor
	pop	cx
   ;	
   ;	 textbackground(0);
   ;	
	?debug	L 98
	xor	ax,ax
	push	ax
	call	near ptr _textbackground
	pop	cx
   ;	
   ;	 clrscr();
   ;	
	?debug	L 99
	call	near ptr _clrscr
   ;	
   ;	 window(1,3,80,25);
   ;	
	?debug	L 100
	mov	ax,25
	push	ax
	mov	ax,80
	push	ax
	mov	ax,3
	push	ax
	mov	ax,1
	push	ax
	call	near ptr _window
	add	sp,8
   ;	
   ;	}
   ;	
	?debug	L 101
	ret	
	?debug	E
	?debug	E
@Layout$qv	endp
_TEXT	ends
_DATA	segment word public 'DATA'
	db	0
	db	0
	db	510 dup (0)
_DATA	ends
_TEXT	segment byte public 'CODE'
   ;	
   ;	void DriveSelection()
   ;	
	?debug	L 103
	assume	cs:_TEXT
@DriveSelection$qv	proc	near
	?debug	B
	push	bp
	mov	bp,sp
	sub	sp,532
	push	si
	?debug	B
   ;	
   ;	{
   ;	 int i,j=0,Number,BIOSNr[256]={0};
   ;	
	?debug	L 105
	xor	si,si
	lea	ax,word ptr [bp-516]
	push	ss
	push	ax
	mov	ax,offset DGROUP:d@w+10
	push	ds
	push	ax
	mov	cx,512
	call	near ptr N_SCOPY@
   ;	
   ;	 union REGS regs;
   ;	
   ;	 for (i=0;i<256;i++)
   ;	
	?debug	L 108
	mov	word ptr [bp-2],0
	jmp	@3@478
@3@58:
   ;	
   ;	 {
   ;	  regs.h.ah=8;
   ;	
	?debug	L 110
	mov	byte ptr [bp-531],8
   ;	
   ;	  regs.h.dl=i;
   ;	
	?debug	L 111
	mov	al,byte ptr [bp-2]
	mov	byte ptr [bp-526],al
   ;	
   ;	  int86(0x13,&regs,&regs);
   ;	
	?debug	L 112
	lea	ax,word ptr [bp-532]
	push	ax
	push	ax
	mov	ax,19
	push	ax
	call	near ptr _int86
	add	sp,6
   ;	
   ;	  if (!regs.x.cx==0)
   ;	
	?debug	L 113
	mov	ax,word ptr [bp-528]
	neg	ax
	sbb	ax,ax
	inc	ax
	or	ax,ax
	je	@@0
	jmp	@3@450
@@0:
   ;	
   ;	  {
   ;	   BIOSNr[j++]=i;
   ;	
	?debug	L 115
	mov	bx,si
	shl	bx,1
	lea	ax,word ptr [bp-516]
	add	bx,ax
	mov	ax,word ptr [bp-2]
	mov	word ptr [bx],ax
	inc	si
   ;	
   ;	   switch(regs.h.bl)
   ;	
	?debug	L 116
	mov	al,byte ptr [bp-530]
	mov	ah,0
	mov	bx,ax
	cmp	bx,5
	ja	short @3@366
	shl	bx,1
	jmp	word ptr cs:@3@C66[bx]
@3@198:
   ;	
   ;	   {
   ;	    case 0: regs.h.ah=0x15;
   ;	
	?debug	L 118
	mov	byte ptr [bp-531],21
   ;	
   ;		    regs.h.dl=i;
   ;	
	?debug	L 119
	mov	al,byte ptr [bp-2]
	mov	byte ptr [bp-526],al
   ;	
   ;	            int86(0x13,&regs,&regs);
   ;	
	?debug	L 120
	lea	ax,word ptr [bp-532]
	push	ax
	push	ax
	mov	ax,19
	push	ax
	call	near ptr _int86
	add	sp,6
   ;	
   ;	            printf("%d) Harddisk    %5lu MB\n",j,((regs.x.cx*65536)+regs.x.dx)/2048);break;
   ;	
	?debug	L 121
	xor	ax,ax
	mov	dx,2048
	push	ax
	push	dx
	mov	ax,word ptr [bp-528]
	xor	dx,dx
	add	dx,word ptr [bp-526]
	adc	ax,0
	push	ax
	push	dx
	call	near ptr N_LDIV@
	push	dx
	push	ax
	push	si
	mov	ax,offset DGROUP:s@+68
	push	ax
	call	near ptr _printf
	add	sp,8
	jmp	short @3@422
@3@226:
   ;	
   ;	    case 1: printf("%d) Floppydisk    320 KB\n",j);break;
   ;	
	?debug	L 122
	push	si
	mov	ax,offset DGROUP:s@+93
	jmp	short @3@394
@3@254:
   ;	
   ;	    case 2: printf("%d) Floppydisk    1,2 MB\n",j);break;
   ;	
	?debug	L 123
	push	si
	mov	ax,offset DGROUP:s@+119
	jmp	short @3@394
@3@282:
   ;	
   ;	    case 3: printf("%d) Floppydisk    620 KB\n",j);break;
   ;	
	?debug	L 124
	push	si
	mov	ax,offset DGROUP:s@+145
	jmp	short @3@394
@3@310:
   ;	
   ;	    case 4: printf("%d) Floppydisk   1,44 MB\n",j);break;
   ;	
	?debug	L 125
	push	si
	mov	ax,offset DGROUP:s@+171
	jmp	short @3@394
@3@338:
   ;	
   ;	    case 5: printf("%d) Floppydisk   2,88 MB\n",j);break;
   ;	
	?debug	L 126
	push	si
	mov	ax,offset DGROUP:s@+197
	jmp	short @3@394
@3@366:
   ;	
   ;	    default:printf("%d) Unknown Disktyp\n",j);
   ;	
	?debug	L 127
	push	si
	mov	ax,offset DGROUP:s@+223
@3@394:
	push	ax
	call	near ptr _printf
	pop	cx
	pop	cx
@3@422:
   ;	
   ;	   }
   ;	   regs.h.ah=8;
   ;	
	?debug	L 129
	mov	byte ptr [bp-531],8
   ;	
   ;	   regs.h.dl=Drive;
   ;	
	?debug	L 130
	mov	al,byte ptr DGROUP:_Drive
	mov	byte ptr [bp-526],al
   ;	
   ;	   int86(0x13,&regs,&regs);
   ;	
	?debug	L 131
	lea	ax,word ptr [bp-532]
	push	ax
	push	ax
	mov	ax,19
	push	ax
	call	near ptr _int86
	add	sp,6
   ;	
   ;	   (char)EndTrack=regs.h.ch;
   ;	
	?debug	L 132
	mov	al,byte ptr [bp-527]
	mov	byte ptr DGROUP:_EndTrack,al
   ;	
   ;	   (char)EndHead=regs.h.dh;
   ;	
	?debug	L 133
	mov	al,byte ptr [bp-525]
	mov	byte ptr DGROUP:_EndHead,al
   ;	
   ;	   (char)EndSector=regs.h.cl;
   ;	
	?debug	L 134
	mov	al,byte ptr [bp-528]
	mov	byte ptr DGROUP:_EndSector,al
@3@450:
	?debug	L 108
	inc	word ptr [bp-2]
@3@478:
	cmp	word ptr [bp-2],256
	jge	@@1
	jmp	@3@58
@@1:
   ;	
   ;	  }
   ;	 }
   ;	 if (j==0)
   ;	
	?debug	L 137
	or	si,si
	jne	short @3@562
   ;	
   ;	 {
   ;	  printf("\nERROR: No diskdrives found.\n");
   ;	
	?debug	L 139
	mov	ax,offset DGROUP:s@+244
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	  exit(1);
   ;	
	?debug	L 140
	mov	ax,1
	push	ax
	call	near ptr _exit
	pop	cx
@3@562:
   ;	
   ;	 }
   ;	 printf("\nSelect Drive: ");
   ;	
	?debug	L 142
	mov	ax,offset DGROUP:s@+274
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	 scanf("%d",&Number);
   ;	
	?debug	L 143
	lea	ax,word ptr [bp-4]
	push	ax
	mov	ax,offset DGROUP:s@+290
	push	ax
	call	near ptr _scanf
	pop	cx
	pop	cx
   ;	
   ;	 Drive=BIOSNr[Number-1];
   ;	
	?debug	L 144
	mov	bx,word ptr [bp-4]
	shl	bx,1
	lea	ax,word ptr [bp-518]
	add	bx,ax
	mov	ax,word ptr [bx]
	mov	word ptr DGROUP:_Drive,ax
   ;	
   ;	}
   ;	
	?debug	L 145
	pop	si
	mov	sp,bp
	pop	bp
	ret	
	?debug	C E319045245475310001F01
	?debug	C E31A08574F52445245475310001E04
	?debug	C E31B08425954455245475308001E0D
	?debug	C E31C0000021A04
	?debug	C E604726567731902ECFD000642494F534E721C02+
	?debug	C FCFD00064E756D6265720402FCFF00016A040801+
	?debug	C 0A003F0104060001690402FEFF00
	?debug	E
	?debug	E
@DriveSelection$qv	endp
@3@C66	label	word
	dw	@3@198
	dw	@3@226
	dw	@3@254
	dw	@3@282
	dw	@3@310
	dw	@3@338
   ;	
   ;	void PartitionSelection()
   ;	
	?debug	L 147
	assume	cs:_TEXT
@PartitionSelection$qv	proc	near
	?debug	B
	push	bp
	mov	bp,sp
	sub	sp,512
	push	si
	push	di
	?debug	B
   ;	
   ;	{
   ;	 int i,j=0;
   ;	
	?debug	L 149
	xor	di,di
   ;	
   ;	 char Buffer[512];
   ;	 unsigned long *Size;
   ;	 union REGS regs;
   ;	
   ;	 if (Drive>=0x80)
   ;	
	?debug	L 154
	cmp	word ptr DGROUP:_Drive,128
	jge	@@2
	jmp	@4@282
@@2:
   ;	
   ;	 {
   ;	  printf("\n");
   ;	
	?debug	L 156
	mov	ax,offset DGROUP:s@+293
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	  Disk(2,Drive,0,0,1,1,&Buffer);
   ;	
	?debug	L 157
	lea	ax,word ptr [bp-512]
	push	ax
	mov	ax,1
	push	ax
	push	ax
	xor	ax,ax
	push	ax
	push	ax
	push	word ptr DGROUP:_Drive
	mov	ax,2
	push	ax
	call	near ptr @Disk$qiiiiiipv
	add	sp,14
   ;	
   ;	  for (i=0;i<4;i++)
   ;	
	?debug	L 158
	xor	si,si
	jmp	short @4@226
@4@86:
   ;	
   ;	  {
   ;	   Size=(unsigned long*)&Buffer[0x1CA+i*0x10];
   ;	
	?debug	L 160
	mov	ax,si
	mov	cl,4
	shl	ax,cl
	lea	dx,word ptr [bp-54]
	add	ax,dx
	mov	cx,ax
   ;	
   ;	   if (*Size>0) printf("%d) Partition %i %5lu MB\n",++j,i+1,*Size/2048);
   ;	
	?debug	L 161
	mov	bx,cx
	cmp	word ptr [bx+2],0
	jb	short @4@198
	ja	short @4@170
	cmp	word ptr [bx],0
	jbe	short @4@198
@4@170:
	mov	bx,cx
	mov	dx,word ptr [bx+2]
	mov	ax,word ptr [bx]
	mov	cl,11
	call	near ptr N_LXURSH@
	push	dx
	push	ax
	mov	ax,si
	inc	ax
	push	ax
	inc	di
	mov	ax,di
	push	ax
	mov	ax,offset DGROUP:s@+295
	push	ax
	call	near ptr _printf
	add	sp,10
@4@198:
	?debug	L 158
	inc	si
@4@226:
	cmp	si,4
	jl	short @4@86
   ;	
   ;	  }
   ;	  printf("\nSelect Partition: ");
   ;	
	?debug	L 163
	mov	ax,offset DGROUP:s@+321
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	  scanf("%d",&Partition);
   ;	
	?debug	L 164
	mov	ax,offset DGROUP:_Partition
	push	ax
	mov	ax,offset DGROUP:s@+341
	push	ax
	call	near ptr _scanf
	pop	cx
	pop	cx
   ;	
   ;	  (char)Head=Buffer[0x1AF+Partition*0x10];
   ;	
	?debug	L 165
	mov	bx,word ptr DGROUP:_Partition
	mov	cl,4
	shl	bx,cl
	lea	ax,word ptr [bp-81]
	add	bx,ax
	mov	al,byte ptr [bx]
	mov	byte ptr DGROUP:_Head,al
   ;	
   ;	  (char)Track=Buffer[0x1B1+Partition*0x10];
   ;	
	?debug	L 166
	mov	bx,word ptr DGROUP:_Partition
	shl	bx,cl
	lea	ax,word ptr [bp-79]
	add	bx,ax
	mov	al,byte ptr [bx]
	mov	byte ptr DGROUP:_Track,al
   ;	
   ;	  (char)Sector=Buffer[0x1B0+Partition*0x10];
   ;	
	?debug	L 167
	mov	bx,word ptr DGROUP:_Partition
	shl	bx,cl
	lea	ax,word ptr [bp-80]
	add	bx,ax
	mov	al,byte ptr [bx]
	mov	byte ptr DGROUP:_Sector,al
   ;	
   ;	  (char)EndHead=Buffer[0x1B3+Partition*0x10];
   ;	
	?debug	L 168
	mov	bx,word ptr DGROUP:_Partition
	shl	bx,cl
	lea	ax,word ptr [bp-77]
	add	bx,ax
	mov	al,byte ptr [bx]
	mov	byte ptr DGROUP:_EndHead,al
   ;	
   ;	  (char)EndTrack=Buffer[0x1B5+Partition*0x10];
   ;	
	?debug	L 169
	mov	bx,word ptr DGROUP:_Partition
	shl	bx,cl
	lea	ax,word ptr [bp-75]
	add	bx,ax
	mov	al,byte ptr [bx]
	mov	byte ptr DGROUP:_EndTrack,al
   ;	
   ;	  (char)EndSector=Buffer[0x1B4+Partition*0x10];
   ;	
	?debug	L 170
	mov	bx,word ptr DGROUP:_Partition
	shl	bx,cl
	lea	ax,word ptr [bp-76]
	add	bx,ax
	mov	al,byte ptr [bx]
	mov	byte ptr DGROUP:_EndSector,al
@4@282:
   ;	
   ;	 }
   ;	}
   ;	
	?debug	L 172
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
	?debug	C E31D000200150C04
	?debug	C E31E0000021A02
	?debug	C E60453697A651D08014F00760004010006427566+
	?debug	C 6665721E0200FE00016A0408010B007E00040700+
	?debug	C 01690408013D008E00040600
	?debug	E
	?debug	E
@PartitionSelection$qv	endp
   ;	
   ;	void NextSector()
   ;	
	?debug	L 174
	assume	cs:_TEXT
@NextSector$qv	proc	near
	?debug	B
	push	bp
	mov	bp,sp
	sub	sp,22
	?debug	B
   ;	
   ;	{
   ;	 int MaxHead=0,MaxTrack=0,MaxSector=0;
   ;	
	?debug	L 176
   ;	
   ;	 union REGS regs;
   ;	
   ;	 Track=Track+(((Sector&0x00C0)<<2)|((Head&0x00C0)<<4));
   ;	
	?debug	L 179
	mov	ax,word ptr DGROUP:_Sector
	and	ax,192
	mov	cl,2
	shl	ax,cl
	mov	dx,word ptr DGROUP:_Head
	and	dx,192
	mov	cl,4
	shl	dx,cl
	or	ax,dx
	mov	dx,word ptr DGROUP:_Track
	add	dx,ax
	mov	word ptr DGROUP:_Track,dx
   ;	
   ;	 Head=Head&0x003F;
   ;	
	?debug	L 180
	mov	ax,word ptr DGROUP:_Head
	and	ax,63
	mov	word ptr DGROUP:_Head,ax
   ;	
   ;	 Sector=Sector&0x003F;
   ;	
	?debug	L 181
	mov	ax,word ptr DGROUP:_Sector
	and	ax,63
	mov	word ptr DGROUP:_Sector,ax
   ;	
   ;	 regs.h.ah=8;
   ;	
	?debug	L 182
	mov	byte ptr [bp-21],8
   ;	
   ;	 regs.h.dl=Drive;
   ;	
	?debug	L 183
	mov	al,byte ptr DGROUP:_Drive
	mov	byte ptr [bp-16],al
   ;	
   ;	 int86(0x13,&regs,&regs);
   ;	
	?debug	L 184
	lea	ax,word ptr [bp-22]
	push	ax
	push	ax
	mov	ax,19
	push	ax
	call	near ptr _int86
	add	sp,6
   ;	
   ;	 MaxTrack=regs.h.ch+(((regs.h.cl&0x00C0)<<2)|((regs.h.dh&0x00C0)<<4));
   ;	
	?debug	L 185
	mov	al,byte ptr [bp-18]
	mov	ah,0
	and	ax,192
	mov	cl,2
	shl	ax,cl
	mov	dl,byte ptr [bp-15]
	mov	dh,0
	and	dx,192
	mov	cl,4
	shl	dx,cl
	or	ax,dx
	mov	dl,byte ptr [bp-17]
	mov	dh,0
	add	dx,ax
	mov	word ptr [bp-4],dx
   ;	
   ;	 MaxHead=regs.h.dh&0x003F;
   ;	
	?debug	L 186
	mov	al,byte ptr [bp-15]
	mov	ah,0
	and	ax,63
	mov	word ptr [bp-2],ax
   ;	
   ;	 MaxSector=regs.h.cl&0x003F;
   ;	
	?debug	L 187
	mov	al,byte ptr [bp-18]
	mov	ah,0
	and	ax,63
	mov	word ptr [bp-6],ax
   ;	
   ;	 Sector++;
   ;	
	?debug	L 188
	inc	word ptr DGROUP:_Sector
   ;	
   ;	 if (Sector>MaxSector)
   ;	
	?debug	L 189
	mov	ax,word ptr DGROUP:_Sector
	cmp	ax,word ptr [bp-6]
	jle	short @5@142
   ;	
   ;	 {
   ;	  Sector=1;
   ;	
	?debug	L 191
	mov	word ptr DGROUP:_Sector,1
   ;	
   ;	  Head++;
   ;	
	?debug	L 192
	inc	word ptr DGROUP:_Head
   ;	
   ;	  if (Head>MaxHead)
   ;	
	?debug	L 193
	mov	ax,word ptr DGROUP:_Head
	cmp	ax,word ptr [bp-2]
	jle	short @5@142
   ;	
   ;	  {
   ;	   Head=0;
   ;	
	?debug	L 195
	mov	word ptr DGROUP:_Head,0
   ;	
   ;	   Track++;
   ;	
	?debug	L 196
	inc	word ptr DGROUP:_Track
   ;	
   ;	   if (Track>MaxTrack)
   ;	
	?debug	L 197
	mov	ax,word ptr DGROUP:_Track
	cmp	ax,word ptr [bp-4]
	jle	short @5@142
   ;	
   ;	   {
   ;	    printf("\nERROR: Not enough diskspace\n");
   ;	
	?debug	L 199
	mov	ax,offset DGROUP:s@+344
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	    exit(1);
   ;	
	?debug	L 200
	mov	ax,1
	push	ax
	call	near ptr _exit
	pop	cx
@5@142:
   ;	
   ;	   }
   ;	  }
   ;	 }
   ;	 Head=Head+((Track>>4)&0x00C0);
   ;	
	?debug	L 204
	mov	ax,word ptr DGROUP:_Track
	mov	cl,4
	sar	ax,cl
	and	ax,192
	mov	dx,word ptr DGROUP:_Head
	add	dx,ax
	mov	word ptr DGROUP:_Head,dx
   ;	
   ;	 Sector=Sector+((Track>>2)&0x00C0);
   ;	
	?debug	L 205
	mov	ax,word ptr DGROUP:_Track
	mov	cl,2
	sar	ax,cl
	and	ax,192
	mov	dx,word ptr DGROUP:_Sector
	add	dx,ax
	mov	word ptr DGROUP:_Sector,dx
   ;	
   ;	 Track=Track&0x00FF;
   ;	
	?debug	L 206
	mov	ax,word ptr DGROUP:_Track
	and	ax,255
	mov	word ptr DGROUP:_Track,ax
   ;	
   ;	}
   ;	
	?debug	L 207
	mov	sp,bp
	pop	bp
	ret	
	?debug	C E604726567731902EAFF00094D6178536563746F+
	?debug	C 720402FAFF00084D6178547261636B0402FCFF00+
	?debug	C 074D6178486561640402FEFF00
	?debug	E
	?debug	E
@NextSector$qv	endp
_TEXT	ends
_DATA	segment word public 'DATA'
	db	0
	db	2 dup (0)
	db	509 dup (0)
_DATA	ends
_TEXT	segment byte public 'CODE'
   ;	
   ;	void WriteBootsector()
   ;	
	?debug	L 209
	assume	cs:_TEXT
@WriteBootsector$qv	proc	near
	?debug	B
	push	bp
	mov	bp,sp
	sub	sp,528
	push	si
	?debug	B
   ;	
   ;	{
   ;	 int FileHandle;
   ;	 BootsectorTyp BSBuffer[1] = {0};
   ;	
	?debug	L 212
	lea	ax,word ptr [bp-512]
	push	ss
	push	ax
	mov	ax,offset DGROUP:d@w+522
	push	ds
	push	ax
	mov	cx,512
	call	near ptr N_SCOPY@
   ;	
   ;	 union REGS regs;
   ;	
   ;	 printf("\nWriting bootsector...");
   ;	
	?debug	L 215
	mov	ax,offset DGROUP:s@+374
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	 if ((FileHandle = open("BOOTCODE.DAT",O_RDONLY|O_BINARY)) == -1)
   ;	
	?debug	L 216
	mov	ax,-32767
	push	ax
	mov	ax,offset DGROUP:s@+397
	push	ax
	call	near ptr _open
	pop	cx
	pop	cx
	mov	si,ax
	cmp	ax,-1
	jne	short @6@86
   ;	
   ;	 {
   ;	  printf("\n\nERROR: Missing BOOTCODE.DAT\n");
   ;	
	?debug	L 218
	mov	ax,offset DGROUP:s@+410
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	  exit(1);
   ;	
	?debug	L 219
	mov	ax,1
	push	ax
	call	near ptr _exit
	pop	cx
@6@86:
   ;	
   ;	 }
   ;	 read(FileHandle,&BSBuffer,512);
   ;	
	?debug	L 221
	mov	ax,512
	push	ax
	lea	ax,word ptr [bp-512]
	push	ax
	push	si
	call	near ptr _read
	add	sp,6
   ;	
   ;	 strcpy(BSBuffer->FileSystem,FILE_SYSTEM);
   ;	
	?debug	L 222
	mov	ax,offset DGROUP:s@+441
	push	ax
	lea	ax,word ptr [bp-509]
	push	ax
	call	near ptr _strcpy
	pop	cx
	pop	cx
   ;	
   ;	 BSBuffer->MediaTyp=Drive;
   ;	
	?debug	L 223
	mov	al,byte ptr DGROUP:_Drive
	mov	byte ptr [bp-501],al
   ;	
   ;	 BSBuffer->SectorSize=SECTOR_SIZE;
   ;	
	?debug	L 224
	mov	byte ptr [bp-500],2
   ;	
   ;	 regs.h.ah=8;
   ;	
	?debug	L 225
	mov	byte ptr [bp-527],8
   ;	
   ;	 regs.h.dl=Drive;
   ;	
	?debug	L 226
	mov	byte ptr [bp-522],al
   ;	
   ;	 int86(0x13,&regs,&regs);
   ;	
	?debug	L 227
	lea	ax,word ptr [bp-528]
	push	ax
	push	ax
	mov	ax,19
	push	ax
	call	near ptr _int86
	add	sp,6
   ;	
   ;	 BSBuffer->MaxTrack=regs.h.ch;
   ;	
	?debug	L 228
	mov	al,byte ptr [bp-523]
	mov	byte ptr [bp-499],al
   ;	
   ;	 BSBuffer->MaxHead=regs.h.dh;
   ;	
	?debug	L 229
	mov	al,byte ptr [bp-521]
	mov	byte ptr [bp-498],al
   ;	
   ;	 BSBuffer->MaxSector=regs.h.cl;
   ;	
	?debug	L 230
	mov	al,byte ptr [bp-524]
	mov	byte ptr [bp-497],al
   ;	
   ;	 BSBuffer->BeginTrack=Track;
   ;	
	?debug	L 231
	mov	al,byte ptr DGROUP:_Track
	mov	byte ptr [bp-496],al
   ;	
   ;	 BSBuffer->BeginHead=Head;
   ;	
	?debug	L 232
	mov	al,byte ptr DGROUP:_Head
	mov	byte ptr [bp-495],al
   ;	
   ;	 BSBuffer->BeginSector=Sector+1;
   ;	
	?debug	L 233
	mov	al,byte ptr DGROUP:_Sector
	inc	al
	mov	byte ptr [bp-494],al
   ;	
   ;	 BSBuffer->EndTrack=EndTrack;
   ;	
	?debug	L 234
	mov	al,byte ptr DGROUP:_EndTrack
	mov	byte ptr [bp-493],al
   ;	
   ;	 BSBuffer->EndHead=EndHead;
   ;	
	?debug	L 235
	mov	al,byte ptr DGROUP:_EndHead
	mov	byte ptr [bp-492],al
   ;	
   ;	 BSBuffer->EndSector=EndSector;
   ;	
	?debug	L 236
	mov	al,byte ptr DGROUP:_EndSector
	mov	byte ptr [bp-491],al
   ;	
   ;	 BSBuffer->BlockSize=BLOCK_SIZE;
   ;	
	?debug	L 237
	mov	byte ptr [bp-490],1
   ;	
   ;	 Disk(3,Drive,Head,Track,Sector,1,&BSBuffer);
   ;	
	?debug	L 238
	lea	ax,word ptr [bp-512]
	push	ax
	mov	ax,1
	push	ax
	push	word ptr DGROUP:_Sector
	push	word ptr DGROUP:_Track
	push	word ptr DGROUP:_Head
	push	word ptr DGROUP:_Drive
	mov	ax,3
	push	ax
	call	near ptr @Disk$qiiiiiipv
	add	sp,14
   ;	
   ;	 NextSector();
   ;	
	?debug	L 239
	call	near ptr @NextSector$qv
   ;	
   ;	 close(FileHandle);
   ;	
	?debug	L 240
	push	si
	call	near ptr _close
	pop	cx
   ;	
   ;	 printf("success\n");
   ;	
	?debug	L 241
	mov	ax,offset DGROUP:s@+450
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	}
   ;	
	?debug	L 242
	pop	si
	mov	sp,bp
	pop	bp
	ret	
	?debug	C E3200D426F6F74736563746F7254797000021E16
	?debug	C E3210003001A02
	?debug	C E3220008001A02
	?debug	C E32300E9011A02
	?debug	C E31F0000021A20
	?debug	C E604726567731902F0FD00084253427566666572+
	?debug	C 1F0200FE000A46696C6548616E646C6504080134+
	?debug	C 000A01040600
	?debug	E
	?debug	E
@WriteBootsector$qv	endp
_TEXT	ends
_DATA	segment word public 'DATA'
	db	0
	db	49 dup (0)
	db	78 dup (0)
	db	384 dup (0)
_DATA	ends
_TEXT	segment byte public 'CODE'
   ;	
   ;	void WriteiNodes()
   ;	
	?debug	L 244
	assume	cs:_TEXT
@WriteiNodes$qv	proc	near
	?debug	B
	push	bp
	mov	bp,sp
	sub	sp,512
	?debug	B
   ;	
   ;	{
   ;	 iNodeTyp iNodeBuffer[4] = {0};
   ;	
	?debug	L 246
	lea	ax,word ptr [bp-512]
	push	ss
	push	ax
	mov	ax,offset DGROUP:d@w+1034
	push	ds
	push	ax
	mov	cx,512
	call	near ptr N_SCOPY@
   ;	
   ;	
   ;	 printf("Writing iNodes.......");
   ;	
	?debug	L 248
	mov	ax,offset DGROUP:s@+459
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	 strcpy(iNodeBuffer[0].Name,"SKY Bootable");
   ;	
	?debug	L 249
	mov	ax,offset DGROUP:s@+481
	push	ax
	lea	ax,word ptr [bp-512]
	push	ax
	call	near ptr _strcpy
	pop	cx
	pop	cx
   ;	
   ;	 iNodeBuffer[0].LowerNode=0x01000001;
   ;	
	?debug	L 250
	mov	word ptr [bp-407],256
	mov	word ptr [bp-409],1
   ;	
   ;	 strcpy(iNodeBuffer[1].Name,"SKYOS");
   ;	
	?debug	L 251
	mov	ax,offset DGROUP:s@+494
	push	ax
	lea	ax,word ptr [bp-384]
	push	ax
	call	near ptr _strcpy
	pop	cx
	pop	cx
   ;	
   ;	 strcpy(iNodeBuffer[1].Type,"SYS");
   ;	
	?debug	L 252
	mov	ax,offset DGROUP:s@+500
	push	ax
	lea	ax,word ptr [bp-334]
	push	ax
	call	near ptr _strcpy
	pop	cx
	pop	cx
   ;	
   ;	 iNodeBuffer[1].HigherNode=0x00000001;
   ;	
	?debug	L 253
	mov	word ptr [bp-287],0
	mov	word ptr [bp-289],1
   ;	
   ;	 iNodeBuffer[1].Pointer1=0x00000002;
   ;	
	?debug	L 254
	mov	word ptr [bp-299],0
	mov	word ptr [bp-301],2
   ;	
   ;	 Disk(3,Drive,Head,Track,Sector,1,&iNodeBuffer);
   ;	
	?debug	L 255
	lea	ax,word ptr [bp-512]
	push	ax
	mov	ax,1
	push	ax
	push	word ptr DGROUP:_Sector
	push	word ptr DGROUP:_Track
	push	word ptr DGROUP:_Head
	push	word ptr DGROUP:_Drive
	mov	ax,3
	push	ax
	call	near ptr @Disk$qiiiiiipv
	add	sp,14
   ;	
   ;	 NextSector();
   ;	
	?debug	L 256
	call	near ptr @NextSector$qv
   ;	
   ;	 printf("success\n");
   ;	
	?debug	L 257
	mov	ax,offset DGROUP:s@+504
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	}
   ;	
	?debug	L 258
	mov	sp,bp
	pop	bp
	ret	
	?debug	C E32508694E6F646554797080001E26
	?debug	C E3260032001A02
	?debug	C E3270003001A02
	?debug	C E3280007001A02
	?debug	C E3290007001A02
	?debug	C E32A0007001A02
	?debug	C E32B0015001A02
	?debug	C E3240000021A25
	?debug	C E60B694E6F6465427566666572240200FE00
	?debug	E
	?debug	E
@WriteiNodes$qv	endp
_TEXT	ends
_DATA	segment word public 'DATA'
	db	0
	db	511 dup (0)
	db	0
	db	0
	db	0
	db	0
	db	508 dup (0)
_DATA	ends
_TEXT	segment byte public 'CODE'
   ;	
   ;	void WriteKernel()
   ;	
	?debug	L 260
	assume	cs:_TEXT
@WriteKernel$qv	proc	near
	?debug	B
	push	bp
	mov	bp,sp
	sub	sp,1024
	push	si
	push	di
	?debug	B
   ;	
   ;	{
   ;	 int i,FileHandle;
   ;	 char DataBuffer[512] = {0};
   ;	
	?debug	L 263
	lea	ax,word ptr [bp-512]
	push	ss
	push	ax
	mov	ax,offset DGROUP:d@w+1546
	push	ds
	push	ax
	mov	cx,512
	call	near ptr N_SCOPY@
   ;	
   ;	 long PointerBuffer[128] = {0};
   ;	
	?debug	L 264
	lea	ax,word ptr [bp-1024]
	push	ss
	push	ax
	mov	ax,offset DGROUP:d@w+2058
	push	ds
	push	ax
	mov	cx,512
	call	near ptr N_SCOPY@
   ;	
   ;	
   ;	 printf("Writing kernel.......");
   ;	
	?debug	L 266
	mov	ax,offset DGROUP:s@+513
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	 if ((FileHandle = open("SKYOS.SYS",O_RDONLY|O_BINARY)) == -1)
   ;	
	?debug	L 267
	mov	ax,-32767
	push	ax
	mov	ax,offset DGROUP:s@+535
	push	ax
	call	near ptr _open
	pop	cx
	pop	cx
	mov	di,ax
	cmp	ax,-1
	jne	short @8@86
   ;	
   ;	 {
   ;	  printf("\n\nERROR: Missing SKYOS.SYS\n");
   ;	
	?debug	L 269
	mov	ax,offset DGROUP:s@+545
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	  exit(1);
   ;	
	?debug	L 270
	mov	ax,1
	push	ax
	call	near ptr _exit
	pop	cx
@8@86:
   ;	
   ;	 }
   ;	 for (i=0;i<=(filelength(FileHandle)/512);i++)
   ;	
	?debug	L 272
	xor	si,si
	jmp	short @8@170
@8@114:
   ;	
   ;	 {
   ;	  PointerBuffer[i]=i+3;
   ;	
	?debug	L 274
	mov	ax,si
	add	ax,3
	cwd	
	mov	bx,si
	mov	cl,2
	shl	bx,cl
	lea	cx,word ptr [bp-1024]
	add	bx,cx
	mov	word ptr [bx+2],dx
	mov	word ptr [bx],ax
	?debug	L 272
	inc	si
@8@170:
	mov	ax,si
	cwd	
	push	ax
	push	dx
	xor	ax,ax
	mov	dx,512
	push	ax
	push	dx
	push	di
	call	near ptr _filelength
	pop	cx
	push	dx
	push	ax
	call	near ptr N_LDIV@
	pop	bx
	cmp	bx,dx
	pop	dx
	jl	short @8@114
	jg	short @8@254
	cmp	dx,ax
	jbe	short @8@114
@8@254:
   ;	
   ;	 }
   ;	 PointerBuffer[i]=0;
   ;	
	?debug	L 276
	mov	bx,si
	mov	cl,2
	shl	bx,cl
	lea	ax,word ptr [bp-1024]
	add	bx,ax
	mov	word ptr [bx+2],0
	mov	word ptr [bx],0
   ;	
   ;	 Disk(3,Drive,Head,Track,Sector,1,&PointerBuffer);
   ;	
	?debug	L 277
	push	ax
	mov	ax,1
	push	ax
	push	word ptr DGROUP:_Sector
	push	word ptr DGROUP:_Track
	push	word ptr DGROUP:_Head
	push	word ptr DGROUP:_Drive
	mov	ax,3
	push	ax
	call	near ptr @Disk$qiiiiiipv
	add	sp,14
   ;	
   ;	 NextSector();
   ;	
	?debug	L 278
	call	near ptr @NextSector$qv
	jmp	short @8@394
@8@282:
   ;	
   ;	 while (!eof(FileHandle))
   ;	 {
   ;	  read(FileHandle,DataBuffer,512);
   ;	
	?debug	L 281
	mov	ax,512
	push	ax
	lea	ax,word ptr [bp-512]
	push	ax
	push	di
	call	near ptr _read
	add	sp,6
   ;	
   ;	  Disk(3,Drive,Head,Track,Sector,1,&DataBuffer);
   ;	
	?debug	L 282
	lea	ax,word ptr [bp-512]
	push	ax
	mov	ax,1
	push	ax
	push	word ptr DGROUP:_Sector
	push	word ptr DGROUP:_Track
	push	word ptr DGROUP:_Head
	push	word ptr DGROUP:_Drive
	mov	ax,3
	push	ax
	call	near ptr @Disk$qiiiiiipv
	add	sp,14
   ;	
   ;	  NextSector();
   ;	
	?debug	L 283
	call	near ptr @NextSector$qv
   ;	
   ;	  for (i=0;i<512;i++) DataBuffer[i]=0;
   ;	
	?debug	L 284
	xor	si,si
	jmp	short @8@366
@8@310:
	mov	byte ptr [bp+si-512],0
	inc	si
@8@366:
	cmp	si,512
	jl	short @8@310
@8@394:
	?debug	L 279
	push	di
	call	near ptr _eof
	pop	cx
	or	ax,ax
	je	short @8@282
   ;	
   ;	 }
   ;	 close(FileHandle);
   ;	
	?debug	L 286
	push	di
	call	near ptr _close
	pop	cx
   ;	
   ;	 printf("success\n");
   ;	
	?debug	L 287
	mov	ax,offset DGROUP:s@+573
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	}
   ;	
	?debug	L 288
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
	?debug	C E32C0000021A06
	?debug	C E32D0000021A02
	?debug	C E60D506F696E7465724275666665722C0200FC00+
	?debug	C 0A446174614275666665722D0200FE000A46696C+
	?debug	C 6548616E646C6504080148004001040700016904+
	?debug	C 080163002E01040600
	?debug	E
	?debug	E
@WriteKernel$qv	endp
   ;	
   ;	void WritePartitionsector()
   ;	
	?debug	L 290
	assume	cs:_TEXT
@WritePartitionsector$qv	proc	near
	?debug	B
	push	bp
	mov	bp,sp
	sub	sp,512
	?debug	B
   ;	
   ;	{
   ;	 int i;
   ;	 char Buffer[512];
   ;	
   ;	 Disk(2,Drive,0,0,1,1,&Buffer);
   ;	
	?debug	L 295
	lea	ax,word ptr [bp-512]
	push	ax
	mov	ax,1
	push	ax
	push	ax
	xor	ax,ax
	push	ax
	push	ax
	push	word ptr DGROUP:_Drive
	mov	ax,2
	push	ax
	call	near ptr @Disk$qiiiiiipv
	add	sp,14
   ;	
   ;	 for (i=0;i<4;i++) Buffer[0x1BE+i*0x10]=0;
   ;	
	?debug	L 296
	xor	dx,dx
	jmp	short @9@114
@9@58:
	mov	bx,dx
	mov	cl,4
	shl	bx,cl
	lea	ax,word ptr [bp-66]
	add	bx,ax
	mov	byte ptr [bx],0
	inc	dx
@9@114:
	cmp	dx,4
	jl	short @9@58
   ;	
   ;	 Buffer[0x1AE+Partition*0x10]=Drive;
   ;	
	?debug	L 297
	mov	bx,word ptr DGROUP:_Partition
	mov	cl,4
	shl	bx,cl
	lea	ax,word ptr [bp-82]
	add	bx,ax
	mov	al,byte ptr DGROUP:_Drive
	mov	byte ptr [bx],al
   ;	
   ;	 Buffer[0x1B2+Partition*0x10]=PARTITION_TYP;
   ;	
	?debug	L 298
	mov	bx,word ptr DGROUP:_Partition
	shl	bx,cl
	lea	ax,word ptr [bp-78]
	add	bx,ax
	mov	byte ptr [bx],16
   ;	
   ;	 Disk(3,Drive,0,0,1,1,&Buffer);
   ;	
	?debug	L 299
	lea	ax,word ptr [bp-512]
	push	ax
	mov	ax,1
	push	ax
	push	ax
	xor	ax,ax
	push	ax
	push	ax
	push	word ptr DGROUP:_Drive
	mov	ax,3
	push	ax
	call	near ptr @Disk$qiiiiiipv
	add	sp,14
   ;	
   ;	}
   ;	
	?debug	L 300
	mov	sp,bp
	pop	bp
	ret	
	?debug	C E32E0000021A02
	?debug	C E6064275666665722E0200FE0001690408012500+
	?debug	C 3D00040200
	?debug	E
	?debug	E
@WritePartitionsector$qv	endp
   ;	
   ;	void main()
   ;	
	?debug	L 302
	assume	cs:_TEXT
_main	proc	near
	?debug	B
	push	bp
	mov	bp,sp
	sub	sp,2
	?debug	B
   ;	
   ;	{
   ;	 char ch;
   ;	 Layout();
   ;	
	?debug	L 305
	call	near ptr @Layout$qv
   ;	
   ;	 DriveSelection();
   ;	
	?debug	L 306
	call	near ptr @DriveSelection$qv
   ;	
   ;	 PartitionSelection();
   ;	
	?debug	L 307
	call	near ptr @PartitionSelection$qv
   ;	
   ;	 if (Drive>=0x80) printf("\nWARNING: All data on partition %d of the harddisk (%.2Xh) will be lost!\n",Partition,Drive);
   ;	
	?debug	L 308
	cmp	word ptr DGROUP:_Drive,128
	jl	short @10@86
	push	word ptr DGROUP:_Drive
	push	word ptr DGROUP:_Partition
	mov	ax,offset DGROUP:s@+582
	push	ax
	call	near ptr _printf
	add	sp,6
	jmp	short @10@114
@10@86:
   ;	
   ;	 else printf("\nWARNING: All data on the floppydisk (%.2Xh) will be lost!\n",Drive);
   ;	
	?debug	L 309
	push	word ptr DGROUP:_Drive
	mov	ax,offset DGROUP:s@+656
	push	ax
	call	near ptr _printf
	pop	cx
	pop	cx
@10@114:
   ;	
   ;	 printf("\nContinue? (y/n) ");
   ;	
	?debug	L 310
	mov	ax,offset DGROUP:s@+716
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	 scanf("%s",&ch);
   ;	
	?debug	L 311
	lea	ax,word ptr [bp-1]
	push	ax
	mov	ax,offset DGROUP:s@+734
	push	ax
	call	near ptr _scanf
	pop	cx
	pop	cx
   ;	
   ;	 if ((ch!='y')&&(ch!='Y'))
   ;	
	?debug	L 312
	cmp	byte ptr [bp-1],121
	je	short @10@198
	cmp	byte ptr [bp-1],89
	je	short @10@198
   ;	
   ;	 {
   ;	  printf("\nSetup aborted!\n");
   ;	
	?debug	L 314
	mov	ax,offset DGROUP:s@+737
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	  exit(0);
   ;	
	?debug	L 315
	xor	ax,ax
	push	ax
	call	near ptr _exit
	pop	cx
@10@198:
   ;	
   ;	 }
   ;	 WriteBootsector();
   ;	
	?debug	L 317
	call	near ptr @WriteBootsector$qv
   ;	
   ;	 WriteiNodes();
   ;	
	?debug	L 318
	call	near ptr @WriteiNodes$qv
   ;	
   ;	 WriteKernel();
   ;	
	?debug	L 319
	call	near ptr @WriteKernel$qv
   ;	
   ;	 if (Drive>=0x80) WritePartitionsector();
   ;	
	?debug	L 320
	cmp	word ptr DGROUP:_Drive,128
	jl	short @10@254
	call	near ptr @WritePartitionsector$qv
@10@254:
   ;	
   ;	}
   ;	
	?debug	L 321
	mov	sp,bp
	pop	bp
	ret	
	?debug	C E60263680202FFFF00
	?debug	E
	?debug	E
_main	endp
	?debug	C E9
	?debug	C FA10010000
_TEXT	ends
_DATA	segment word public 'DATA'
s@	label	byte
	db	10
	db	10
	db	'ERROR: Not able to read/write disksector.'
	db	10
	db	0
	db	'SKY Systems Setup 1.0a'
	db	0
	db	'%d) Harddisk    %5lu MB'
	db	10
	db	0
	db	'%d) Floppydisk    320 KB'
	db	10
	db	0
	db	'%d) Floppydisk    1,2 MB'
	db	10
	db	0
	db	'%d) Floppydisk    620 KB'
	db	10
	db	0
	db	'%d) Floppydisk   1,44 MB'
	db	10
	db	0
	db	'%d) Floppydisk   2,88 MB'
	db	10
	db	0
	db	'%d) Unknown Disktyp'
	db	10
	db	0
	db	10
	db	'ERROR: No diskdrives found.'
	db	10
	db	0
	db	10
	db	'Select Drive: '
	db	0
	db	'%d'
	db	0
	db	10
	db	0
	db	'%d) Partition %i %5lu MB'
	db	10
	db	0
	db	10
	db	'Select Partition: '
	db	0
	db	'%d'
	db	0
	db	10
	db	'ERROR: Not enough diskspace'
	db	10
	db	0
	db	10
	db	'Writing bootsector...'
	db	0
	db	'BOOTCODE.DAT'
	db	0
	db	10
	db	10
	db	'ERROR: Missing BOOTCODE.DAT'
	db	10
	db	0
	db	'SKYFS1.0'
	db	0
	db	'success'
	db	10
	db	0
	db	'Writing iNodes.......'
	db	0
	db	'SKY Bootable'
	db	0
	db	'SKYOS'
	db	0
	db	'SYS'
	db	0
	db	'success'
	db	10
	db	0
	db	'Writing kernel.......'
	db	0
	db	'SKYOS.SYS'
	db	0
	db	10
	db	10
	db	'ERROR: Missing SKYOS.SYS'
	db	10
	db	0
	db	'success'
	db	10
	db	0
	db	10
	db	'WARNING: All data on partition %d of the harddisk (%.2Xh) wil'
	db	'l be lost!'
	db	10
	db	0
	db	10
	db	'WARNING: All data on the floppydisk (%.2Xh) will be lost!'
	db	10
	db	0
	db	10
	db	'Continue? (y/n) '
	db	0
	db	'%s'
	db	0
	db	10
	db	'Setup aborted!'
	db	10
	db	0
_DATA	ends
_TEXT	segment byte public 'CODE'
_TEXT	ends
	public	_main
	public	@WritePartitionsector$qv
	public	@WriteKernel$qv
	public	@WriteiNodes$qv
	public	@WriteBootsector$qv
	public	@NextSector$qv
	extrn	N_LXURSH@:far
	public	@PartitionSelection$qv
	extrn	N_LDIV@:far
	extrn	N_SCOPY@:far
	public	@DriveSelection$qv
	public	@Layout$qv
	public	@Disk$qiiiiiipv
	public	_EndSector
	public	_EndTrack
	public	_EndHead
	public	_Partition
	public	_Sector
	public	_Track
	public	_Head
	public	_Drive
_pokeb	equ	pokeb
_poke	equ	poke
_peekb	equ	peekb
_peek	equ	peek
	extrn	_read:near
	extrn	_open:near
	extrn	_filelength:near
	extrn	_eof:near
	extrn	_close:near
	extrn	_int86:near
	extrn	_biosdisk:near
	extrn	_exit:near
	extrn	_strcpy:near
	extrn	_window:near
	extrn	_textcolor:near
	extrn	_textbackground:near
	extrn	_gotoxy:near
	extrn	_clrscr:near
	extrn	_scanf:near
	extrn	_printf:near
_s@	equ	s@
	?debug	C EA0508
	?debug	C E32F00000023010000
	?debug	C EC055F6D61696E2F1800
	?debug	C E33000000023010000
	?debug	C EC18405772697465506172746974696F6E736563+
	?debug	C 746F72247176301800
	?debug	C E33100000023010000
	?debug	C EC0F4057726974654B65726E656C247176311800
	?debug	C E33200000023010000
	?debug	C EC0F405772697465694E6F646573247176321800
	?debug	C E33300000023010000
	?debug	C EC13405772697465426F6F74736563746F722471+
	?debug	C 76331800
	?debug	C E33400000023010000
	?debug	C EC0E404E657874536563746F72247176341800
	?debug	C E33500000023040500
	?debug	C EB094E5F4C5855525348403500
	?debug	C E33600000023010000
	?debug	C EC1640506172746974696F6E53656C656374696F+
	?debug	C 6E247176361800
	?debug	C EB074E5F4C444956403500
	?debug	C EB084E5F53434F5059403500
	?debug	C E33700000023010000
	?debug	C EC1240447269766553656C656374696F6E247176+
	?debug	C 371800
	?debug	C E33800000023010000
	?debug	C EC0A404C61796F7574247176381800
	?debug	C E33900000023010000
	?debug	C EC0F404469736B24716969696969697076391800
	?debug	C EC0A5F456E64536563746F72040000
	?debug	C EC095F456E64547261636B040000
	?debug	C EC085F456E6448656164040000
	?debug	C EC0A5F506172746974696F6E040000
	?debug	C EC075F536563746F72040000
	?debug	C EC065F547261636B040000
	?debug	C EC055F48656164040000
	?debug	C EC065F4472697665040000
	?debug	C E33A00000023010000
	?debug	C E33B00000023010000
	?debug	C E33C00000023020000
	?debug	C E33D00000023040000
	?debug	C E33E0666696E645F742B001E37
	?debug	C E33F0015001A02
	?debug	C E340000D001A02
	?debug	C E341056666626C6B2B001E3E
	?debug	C E3420015001A02
	?debug	C E343000D001A02
	?debug	C E34409646F73646174655F7405001E45
	?debug	C E345046461746504001E4A
	?debug	C E34609646F7374696D655F7404001E4E
	?debug	C E3470474696D6504001E53
	?debug	C E3480664657668647212001E58
	?debug	C E3490008001A02
	?debug	C E34A07666174696E666F06001E5E
	?debug	C E34B0A6469736B667265655F7408001E63
	?debug	C E34C05646672656508001E68
	?debug	C E34D08444F534552524F5205001E6D
	?debug	C E34E07434F554E54525922001E72
	?debug	C E34F0005001A02
	?debug	C E3500002001A02
	?debug	C E3510002001A02
	?debug	C E3520002001A02
	?debug	C E3530002001A02
	?debug	C E3540002001A02
	?debug	C E355000A001A02
	?debug	C E35604786663622C001E7F
	?debug	C E3570005001A02
	?debug	C E3580366636225001E8084
	?debug	C E3590008001A02
	?debug	C E35A0003001A02
	?debug	C E35B000A001A02
	?debug	C E35C00000023040000
	?debug	C EB055F726561645C00
	?debug	C E35D00000023040001
	?debug	C EB055F6F70656E5D00
	?debug	C E35E00000023060000
	?debug	C EB0B5F66696C656C656E6774685E00
	?debug	C E35F00000023040000
	?debug	C EB045F656F665F00
	?debug	C E36000000023040000
	?debug	C EB065F636C6F73656000
	?debug	C E361056674696D6504001E808F
	?debug	C E36200000023040000
	?debug	C EB065F696E7438366200
	?debug	C E36300000023040000
	?debug	C EB095F62696F736469736B6300
	?debug	C E364075245475041434B14001E8096
	?debug	C E36505535245475308001E80A1
	?debug	C E3660A6469736B696E666F5F740E001E80A6
	?debug	C E367000400160100
	?debug	C E36800000023010000
	?debug	C EB055F657869746800
	?debug	C E36A000200150204
	?debug	C E369000000236A0000
	?debug	C EB075F7374726370796900
	?debug	C E36B00000023010000
	?debug	C EB075F77696E646F776B00
	?debug	C E36C00000023010000
	?debug	C EB0A5F74657874636F6C6F726C00
	?debug	C E36D00000023010000
	?debug	C EB0F5F746578746261636B67726F756E646D00
	?debug	C E36E00000023010000
	?debug	C EB075F676F746F78796E00
	?debug	C E36F00000023010000
	?debug	C EB075F636C727363726F00
	?debug	C E37009746578745F696E666F0B001E80AD
	?debug	C E37100000023040001
	?debug	C EB065F7363616E667100
	?debug	C E37200000023040001
	?debug	C EB075F7072696E74667200
	?debug	C E60D426F6F74736563746F725479702006000869+
	?debug	C 4E6F64655479702506000666706F735F74060600+
	?debug	C 0673697A655F740A06000D426F6F74736563746F+
	?debug	C 7254797020070008694E6F646554797025070006+
	?debug	C 66696E645F743E0700056666626C6B4107000964+
	?debug	C 6F73646174655F74440700046461746545070009+
	?debug	C 646F7374696D655F744607000474696D65470700+
	?debug	C 0664657668647248070007666174696E666F4A07+
	?debug	C 000A6469736B667265655F744B07000564667265+
	?debug	C 654C070008444F534552524F524D070007434F55+
	?debug	C 4E5452594E070004786663625607000366636258+
	?debug	C 0700056674696D65610700075245475041434B64+
	?debug	C 0700055352454753650700045245475319070008+
	?debug	C 42595445524547531B070008574F524452454753+
	?debug	C 1A07000A6469736B696E666F5F74660700097465+
	?debug	C 78745F696E666F700700
	?debug	F pokeb 58 24 0
	?debug	F poke 59 24 0
	?debug	F peekb 60 24 0
	?debug	F peek 61 24 0
	?debug	C E20001781A0001681BC010000000
	?debug	C E2000261780A000262780A000263780A00026478+
	?debug	C 0A000273690A000264690A000563666C61670A00+
	?debug	C 05666C6167730AC010000000
	?debug	C E20002616C0800026168080002626C0800026268+
	?debug	C 080002636C0800026368080002646C0800026468+
	?debug	C 08C008000000
	?debug	C E200044A756D7021000A46696C6553797374656D+
	?debug	C 2200084D6564696154797002000A536563746F72+
	?debug	C 53697A650200084D6178547261636B0200074D61+
	?debug	C 78486561640200094D6178536563746F7202000A+
	?debug	C 426567696E547261636B020009426567696E4865+
	?debug	C 616402000B426567696E536563746F7202000845+
	?debug	C 6E64547261636B020007456E6448656164020009+
	?debug	C 456E64536563746F72020009426C6F636B53697A+
	?debug	C 650200045265737423C000020000
	?debug	C E200044E616D65260004547970652700054F776E+
	?debug	C 6572040006526967687473040009417474726962+
	?debug	C 7574730200054461746531280005446174653229+
	?debug	C 000544617465332A000846696C6573697A650600+
	?debug	C 08506F696E74657231060008506F696E74657232+
	?debug	C 060008506F696E7465723306000A486967686572+
	?debug	C 4E6F64650600084E6578744E6F64650600094C6F+
	?debug	C 7765724E6F6465060005456D7074792BC0800000+
	?debug	C 00
	?debug	C E2000872657365727665643F0006617474726962+
	?debug	C 02000777725F74696D650A000777725F64617465+
	?debug	C 0A000473697A650600046E616D6540C02B000000
	?debug	C E2000B66665F726573657276656442000966665F+
	?debug	C 61747472696202000866665F6674696D650A0008+
	?debug	C 66665F66646174650A000866665F6673697A6506+
	?debug	C 000766665F6E616D6543C02B000000
	?debug	C E200036461790800056D6F6E7468080004796561+
	?debug	C 720A00096461796F667765656B08C005000000
	?debug	C E2000764615F7965617204000664615F64617902+
	?debug	C 000664615F6D6F6E02C004000000
	?debug	C E20004686F75720800066D696E75746508000673+
	?debug	C 65636F6E64080007687365636F6E6408C0040000+
	?debug	C 00
	?debug	C E2000674695F6D696E08000774695F686F757208+
	?debug	C 000774695F68756E6408000674695F73656308C0+
	?debug	C 04000000
	?debug	C E2000764685F6E65787406000764685F61747472+
	?debug	C 04000864685F73747261740A000864685F696E74+
	?debug	C 65720A000764685F6E616D6549C012000000
	?debug	C E2000866695F73636C757302000866695F666174+
	?debug	C 696402000866695F6E636C75730A000866695F62+
	?debug	C 7973656304C006000000
	?debug	C E2000E746F74616C5F636C7573746572730A000E+
	?debug	C 617661696C5F636C7573746572730A0013736563+
	?debug	C 746F72735F7065725F636C75737465720A001062+
	?debug	C 797465735F7065725F736563746F720AC0080000+
	?debug	C 00
	?debug	C E2000864665F617661696C0A000864665F746F74+
	?debug	C 616C0A000764665F627365630A000864665F7363+
	?debug	C 6C75730AC008000000
	?debug	C E2000B64655F6578746572726F7204000864655F+
	?debug	C 636C61737302000964655F616374696F6E020008+
	?debug	C 64655F6C6F63757302C005000000
	?debug	C E20007636F5F64617465040007636F5F63757272+
	?debug	C 4F0008636F5F7468736570500008636F5F646573+
	?debug	C 6570510008636F5F6474736570520008636F5F74+
	?debug	C 6D73657053000C636F5F637572727374796C6502+
	?debug	C 0009636F5F646967697473020007636F5F74696D+
	?debug	C 65020007636F5F63617365060008636F5F646173+
	?debug	C 6570540007636F5F66696C6C55C022000000
	?debug	C E20009786663625F666C6167020009786663625F+
	?debug	C 72657376570009786663625F6174747202000878+
	?debug	C 6663625F66636258C02C000000
	?debug	C E200096663625F64726976650200086663625F6E+
	?debug	C 616D655900076663625F6578745A000A6663625F+
	?debug	C 637572626C6B04000B6663625F72656373697A65+
	?debug	C 04000B6663625F66696C73697A65060008666362+
	?debug	C 5F646174650400086663625F726573765B000A66+
	?debug	C 63625F63757272656302000A6663625F72616E64+
	?debug	C 6F6D06C025000000
	?debug	C E2050766745F747365630A060666745F6D696E0A+
	?debug	C 050766745F686F75720A050666745F6461790A04+
	?debug	C 0866745F6D6F6E74680A070766745F796561720A+
	?debug	C C004000000
	?debug	C E20004725F61780A0004725F62780A0004725F63+
	?debug	C 780A0004725F64780A0004725F62700A0004725F+
	?debug	C 73690A0004725F64690A0004725F64730A000472+
	?debug	C 5F65730A0007725F666C6167730AC014000000
	?debug	C E2000265730A000263730A000273730A00026473+
	?debug	C 0AC008000000
	?debug	C E2000564726976650A0004686561640A00057472+
	?debug	C 61636B0A0006736563746F720A00086E73656374+
	?debug	C 6F72730A000662756666657267C00E000000
	?debug	C E2000777696E6C65667408000677696E746F7008+
	?debug	C 000877696E726967687408000977696E626F7474+
	?debug	C 6F6D0800096174747269627574650800086E6F72+
	?debug	C 6D61747472080008637572726D6F646508000C73+
	?debug	C 637265656E68656967687408000B73637265656E+
	?debug	C 7769647468080004637572780800046375727908+
	?debug	C C00B000000
	end
