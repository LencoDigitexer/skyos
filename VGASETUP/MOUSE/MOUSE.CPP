#include <dos.h>
#include <conio.h>
#include <graphics.h>
#include "v8060\v8060.h"

char maskstr[2][255] = {"c:\\vms\\source\\kernel\\gfx\\cursor\\norm.mms",
			"c:\\vms\\source\\kernel\\gfx\\cursor\\hot.mms"};
int mask = 0;
int Init_Mouse(void)
{
  union REGS regs;

  regs.x.ax = 0x00;
  int86(0x33,&regs,&regs);

  if (regs.x.ax == 0x0000) return 0;
  else return regs.x.bx;
}

void Show_Mouse(void)
{
  asm mov ax, 0x01;
  asm int 0x33;
}

void Hide_Mouse(void)
{
  asm mov ax, 0x02;
  asm int 0x33;
}

void Set_Cursor(int x, int y)
{
  asm mov ax, 0x04;
  asm mov cx, x;
  asm mov dx, y;
  asm int 0x33;
}

void mouse(int *x, int *y, int *b)
{
   union REGS regs;

   regs.x.ax = 0x03;
   int86(0x33,&regs,&regs);

   *b = regs.x.bx;
   *x = regs.x.cx;
   *y = regs.x.dx;
}

void do_mouse(int *x, int *y, int *b, int buttontobepressed)
{
  int buffer[25][25];
  int i,j;
  int c = 0;

  Show_Mouse();
  mouse(x,y,b);
  int ox = *x;
  int oy = *y;

  i = 0;
  j = 0;

  for (i=0;i<20;i++)
   for (j=0;j<20;j++)
    buffer[0+i][0+j] = getpixel(*x+i,*y+j);

  while (!(*b&buttontobepressed))
  {
    if (kbhit())
    {
      *x = 1000;
      *y = 1000;
      *b = gc();
      return;
    }
//    while (ipxRecvChk()) recvPacket();

    int ok = 0;

    while (!ok)
    {
      mouse(x,y,b);
      if (ox != *x) ok =1;
      if (oy != *y) ok =1;
      if (*b) ok = 1;
    }
    for (i=0; i<20;i++)
      for (j=0;j<20;j++ )
	putpixel(ox+i,oy+j,buffer[0+i][0+j]);

    ox = *x;
    oy = *y;


    for (i=0;i<20;i++)
      for (j=0;j<20;j++)
	buffer[0+i][0+j] = getpixel(*x+i,*y+j);


    Picg(maskstr[mask],*x,*y);
    }


    for (i=0; i<20;i++)
      for (j=0;j<20;j++)
	putpixel(*x+i,*y+j,buffer[0+i][0+j]);
}

void sethbounds(int l, int r) /* set left and right boundaries */
{
 union REGS regs;
 regs.x.ax = 7;
 regs.x.cx = l;
 regs.x.dx = r;
 int86(0x33,&regs,&regs);
}


/* mouse function 8 */
void setvbounds(int t, int b) /*set top and bottom bounds */
{
 union REGS regs;
 regs.x.ax = 8;
 regs.x.cx = t;
 regs.x.dx = b;
 int86(0x33,&regs,&regs);
}
