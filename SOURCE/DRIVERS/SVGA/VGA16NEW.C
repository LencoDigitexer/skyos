/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\svga\vga16.c
/* Last Update: 24.07.1999
/* Version    : beta
/* Coded by   : Gregory Hayrapetian
/* Docus      : Internet
/************************************************************************/
/* Definition:
/*   A Standard VGA graphic device driver.
/************************************************************************/
#include "svga\svgadev.h"
#include "newgui.h"

#define NULL (void*)0

#define out_GfxController(index,value)  { outportb(0x3ce,index); outportb(0x3cf,value); }
#define in_InputStatus1                   inportb(0x3da);
#define out_AttrController(index,value) { in_InputStatus1;\
                                          outportb(0x3c0,index);\
                                          outportb(0x3c0,value);\
                                          outportb(0x3c0,0x20); }

#define in_AttrController(index,data)   { in_InputStatus1;\
                                          outportb(0x3c0,index);\
                                          data=inportb(0x3c1);\
                                          outportb(0x3c0,data);\
                                          outportb(0x3c0,0x20); }

extern unsigned int MAXX, MAXY;

unsigned char *screen = (unsigned char*) 0xa0000;

unsigned char  VGA16getBitMask[8] =
{
  0xff, 0x7f, 0x3f, 0x1f, 0x0f, 7, 3, 1
};

unsigned char  VGA16getBitMask2[8] =
{
  0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
};

void vga16_init();
void set_draw_table();


display_device_driver display_driver_vga16 =
{
    "Standard VGA",
    "4-Bit",
    vga16_detect,
    vga16_init,
    set_draw_table,
    NULL,
    NULL,
    vga16_setmode,
    NULL,

    0,
    0,
	0,
	0
};


/* BIOS mode 12h - 640x480x16 */
static const unsigned char g640x480x16_regs[60] =
{
  0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E, 0x00, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xEA, 0x8C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xE3
};

static unsigned char g800x600x16_regs[60] =
{
  0x80, 0x63, 0x64, 0x03, 0x67, 0x1C, 0x77, 0xF0, 0x00, 0x60, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x60, 0x82, 0x57, 0x32, 0x00, 0x5B, 0x75, 0xC3,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x02, 0x01, 0x0F, 0x00, 0x06,
    0xEB
};


unsigned int vga16_detect(void)
{
   return 1;           // always present
}

int vga16_setmode(int mode)
{
    const unsigned char *regs;

    if (mode == 0x12)
    {
      regs = (const unsigned char *)  g640x480x16_regs;
      MAXX = 640;
      MAXY = 480;
    }
    else if (mode == 0x6a)
    {
      regs = (const unsigned char *)  g800x600x16_regs;
      MAXX = 800;
      MAXY = 600;
    }

    vga_screenoff();
    svgadev_setregs(regs);
    vga_screenon();
}

void vga16_init()
{

}
/*
void set_pixel16_1(int x,int y,int color)
{
  unsigned int ScrOffs;
  unsigned char PointMask,Latch;

  ScrOffs=(y* MAXX + x) / 8;
  PointMask= 0x80 >> (x & 7);
  out_GfxController(5,2);
  out_GfxController(8,PointMask);
  Latch=screen[ScrOffs];
  screen[ScrOffs]=(unsigned char)color;
  out_GfxController(5,0);
  out_GfxController(8,0xff);
}
*/

void set_pixel16(int x,int y,unsigned char color)
{

  out_GfxController(0,color);
  out_GfxController(1,0xf);

  out_GfxController(8,0x80 >> (x & 7));
  screen[(y * MAXX + x) >> 3]--;
  out_GfxController(1,0);
//  out_GfxController(8,0xff);

}

int get_pixel16(int x, int y)
{
  unsigned int ScrOffs,Color=0;
  unsigned char PointMask;
  int i;

  ScrOffs = (y * MAXX + x) >> 3;
  PointMask= 0x80 >> (x & 7);

  for(i=3;i>=0;i--)
  {
    Color <<= 1;
    out_GfxController(4,i);
    if (screen[ScrOffs] & PointMask) Color++;
  }
  return Color;
}

void draw_hline16(int x1, int x2, int y,int color)
{
  FillArea(x1,y,x2,y,color,0);
}

void draw_fill_rect16(int x1, int y1, int x2, int y2, int color)
{
  FillArea(x1,y1,x2,y2,color,0);
}

void copy_screen_rect16(int x1, int y1, int x2, int y2, int dx, int dy)
{
  int i, j, xlen, k;
  rect_t rect = {x1,y1,x2,y2};
  char *ptr1, *ptr2, *buf;

  if (fit_rect(&rect,&screen_rect))
    return;

  if (dx < 0)
  {
    if ((rect.x1 + dx) < 0) rect.x1 += -(dx + rect.x1);
  }
  else
  {
    if ((rect.x2 + dx) > screen_rect.x2) rect.x2 -= dx - (screen_rect.x2-rect.x2);
  }

  if (dy < 0)
  {
    if ((rect.y1 + dy) < 0) rect.y1 += -(dy + rect.y1);
  }
  else
  {
    if ((rect.y2 + dy) > screen_rect.y2) rect.y2 -= dy - (screen_rect.y2-rect.y2);
  }

  xlen = ((rect.x2+dx)>>3) - ((rect.x1+dx)>>3) + 1;

  buf = valloc((xlen+10)*(rect.y2-rect.y1+10)*4);

  Vcapt16(buf, xlen+1, rect.y2-rect.y1+1,0,0,rect.x1,rect.y1,xlen+1, rect.y2-rect.y1+1);
  Vdisp16(rect.x1, rect.x2+dx,rect.x1+dx,rect.y1+dy,buf,xlen+1,rect.y2-rect.y1+1,0,0,xlen,rect.y2-rect.y1+1);

  vfree(buf);
}

void Select_WPlane(unsigned char p)
{
  int plane = (int) p<<8 + 2;

  outportb(0x3c4, 2);
  outportb(0x3c5, p);
}

void Select_RPlane(unsigned char p)
{
  int plane = (int) (p&3)<<8 + 4;

  outportb(0x3ce, 4);
  outportb(0x3cf, p);
}

int Vdisp16(int srcx, int ex, int dx, unsigned dy,
             char *s, unsigned sw, unsigned sl,
             unsigned sx, unsigned sy, unsigned sdx, unsigned sdy)
{
   unsigned char *dest;
   unsigned char *src, tmp, tmp2;
   unsigned yo;
   int i,shift;

   dest=(char *) (0xa0000 + (dy*MAXX + dx)/8);
   src=(char *) (s + sy*(sw*4) + sx);

   out_GfxController(5,0);

   shift = abs((srcx%8) - (dx%8));

   if (shift != 0)
   {
     if ((srcx%8) < (dx%8))
     {

       for (yo=0; yo < sl*4; yo++)
       {
         tmp = ~VGA16getBitMask[dx%8];
         for (i=0;i<sw;i++)
         {
           tmp2 = src[yo*sw+i] << (8-shift);
           src[yo*sw+i] >>= shift;
           src[yo*sw+i] |= tmp;
           tmp = tmp2;
         }
       }
     }
     else
     {

       for (yo=0; yo < sl*4; yo++)
       {
         tmp = 0;

         for (i=sw-1;i>=0;i--)
         {
           tmp2 = src[yo*sw+i] >> (8-shift);
           src[yo*sw+i] <<= shift;
           src[yo*sw+i] |= tmp;
           tmp = tmp2;
         }
       }
     }
   }

   out_GfxController(8,0xff);

   for (yo=0; yo < sdy; yo++)
   {
       Select_WPlane(1);
//       for (i=1;i<sdx-1;i++)
//         dest[i] = src[i];
       memcpy(dest+1,src+1,sdx-2);
       src=(char *) (src + sw);

       Select_WPlane(2);
//       for (i=1;i<sdx-1;i++)
//         dest[i] = src[i];
       memcpy(dest+1,src+1,sdx-2);
       src=(char *) (src + sw);

       Select_WPlane(4);
//       for (i=1;i<sdx-1;i++)
//         dest[i] = src[i];
       memcpy(dest+1,src+1,sdx-2);
       src=(char *) (src + sw);

       Select_WPlane(8);
//       for (i=1;i<sdx-1;i++)
//         dest[i] = src[i];
       memcpy(dest+1,src+1,sdx-2);
       dest=(char *) (dest + (MAXX/8));
       src=(char *) (src + sw);
   }

   dest=(char *) (0xa0000 + (dy*MAXX + dx)/8);
   src=(char *) (s + sy*(sw*4) + sx);

   out_GfxController(8,VGA16getBitMask[dx%8]);

   for (yo=0; yo < sdy; yo++)
   {
       Select_WPlane(1);
       tmp = dest[0];
       dest[0] = src[0];
       src=(char *) (src + sw);

       Select_WPlane(2);
       tmp = dest[0];
       dest[0] = src[0];
       src=(char *) (src + sw);

       Select_WPlane(4);
       tmp = dest[0];
       dest[0] = src[0];
       src=(char *) (src + sw);

       Select_WPlane(8);
       tmp = dest[0];
       dest[0] = src[0];
       dest=(char *) (dest + (MAXX/8));
       src=(char *) (src + sw);
   }

   dest=(char *) (0xa0000 + (dy*MAXX + dx)/8);
   src=(char *) (s + sy*(sw*4) + sx);

   out_GfxController(8,VGA16getBitMask2[ex%8]);

   for (yo=0; yo < sdy; yo++)
   {
       Select_WPlane(1);
       tmp = dest[sdx-1];
       dest[sdx-1] = src[sdx-1];
       src=(char *) (src + sw);

       Select_WPlane(2);
       tmp = dest[sdx-1];
       dest[sdx-1] = src[sdx-1];
       src=(char *) (src + sw);

       Select_WPlane(4);
       tmp = dest[sdx-1];
       dest[sdx-1] = src[sdx-1];
       src=(char *) (src + sw);

       Select_WPlane(8);
       tmp = dest[sdx-1];
       dest[sdx-1] = src[sdx-1];
       dest=(char *) (dest + (MAXX/8));
       src=(char *) (src + sw);

   }

   Select_WPlane(0x0f);

   return(1);
}

int Vcapt16( char *d, unsigned dw, unsigned dl,
                          unsigned dx, unsigned dy,
                      unsigned sx, unsigned sy, unsigned sdx, unsigned sdy)
{
   char *dest;
   char *src;
   unsigned yo, i;

   src=(char *)  (0xa0000 + (sy*MAXX + sx)/8);
   dest=(char *) (d + dy*dw*4 + dx);

   for (yo=0; yo < sdy; yo++)
   {
       Select_RPlane(0);
//       for (i=0;i<sdx;i++)
//         dest[i] = src[i];
       memcpy(dest, src, sdx);
       dest=(char *) (dest + dw);

       Select_RPlane(1);
//       for (i=0;i<sdx;i++)
//         dest[i] = src[i];
       memcpy(dest, src, sdx);
       dest=(char *) (dest + dw);

       Select_RPlane(2);
//       for (i=0;i<sdx;i++)
//         dest[i] = src[i];
       memcpy(dest, src, sdx);
       dest=(char *) (dest + dw);

       Select_RPlane(3);
//       for (i=0;i<sdx;i++)
//         dest[i] = src[i];
       memcpy(dest, src, sdx);
       src=(char *) (src + MAXX/8);
       dest=(char *) (dest + dw);
   }
   return(1);
}

void set_draw_table(display_draw_table *dt)
{
  dt->color_depth = 4;

  dt->draw_pixel			= set_pixel16;
  dt->get_pixel			= get_pixel16;
  dt->draw_hline			= draw_hline16;
  dt->draw_fill_rect		= draw_fill_rect16;
  dt->copy_screen_rect	= copy_screen_rect16;
}

void area(int xa,int ya,int ye,int PointMask)
{
  unsigned int k,ScrOffs;

  ScrOffs=(ya* MAXX + xa)/8;
  out_GfxController(8,PointMask);
  for(k=ya;k<=ye;k++)
  {
    screen[ScrOffs]++;
    ScrOffs+=MAXX/8;
  }
}

void FillArea(int xa,int ya,int xe,int ye,int color,int func)
{
  unsigned int temp;
  unsigned char PointMask,PointMask2;

  out_GfxController(0,color);
  out_GfxController(1,0xf);
  out_GfxController(3,func<<3);
  PointMask= 0xff >> (xa & 7);

  if ((xe-xa)<(8-(xa & 7))) 
  {
    PointMask2=0xff >> ((xe&7)+1);
    PointMask^=PointMask2;
    area(xa,ya,ye,PointMask);
  }
  else
  {
    area(xa,ya,ye,PointMask);
    xa+=8-(xa & 7); 
    for(;xa+8<=xe;xa+=8) area(xa,ya,ye,0xff); 
    PointMask=0xff << 7-(xe & 7); 
    area(xa,ya,ye,PointMask);
  }
  out_GfxController(8,0xff);
  out_GfxController(1,0);
  out_GfxController(3,0); 
}
void Set_PalRange(unsigned int start,unsigned int end,unsigned char *palette)
{
  unsigned int i;

  outportb(0x03c8,start);
  start*=3;
  end++; end*=3;
  for (i=0;i<end-start;i++) outportb(0x03c9,palette[i]);
}

unsigned char ChangePalReg(unsigned char Addr,unsigned char NewColReg)
{
  unsigned char OldColReg;

  in_AttrController(Addr,OldColReg);
  out_AttrController(Addr,NewColReg);
  return(OldColReg);
}

void ChangeColorReg(unsigned char Addr,unsigned char Red,unsigned char Green,unsigned char Blue)
{
  unsigned char ColorReg;
  unsigned char Pal[3];

  in_AttrController(Addr,ColorReg);
  Pal[0]=Red;
  Pal[1]=Green;
  Pal[2]=Blue;
  Set_PalRange(ColorReg,ColorReg,Pal);
}


void std_pal()
{
  ChangeColorReg(0,0,0,0);     /* COLOR_BLACK        0  */
  ChangeColorReg(1,0,0,48);    /* COLOR_BLUE         1  */
  ChangeColorReg(2,0,32,0);    /* COLOR_GREEN        2  */
  ChangeColorReg(3,0,32,32);   /* COLOR_CYAN         3  */
  ChangeColorReg(4,32,0,0);    /* COLOR_RED          4  */
  ChangeColorReg(5,32,32,0);   /* COLOR_MAGENTA      5  */
  ChangeColorReg(6,0,0,0);     /* COLOR_BROWN        6  */
  ChangeColorReg(7,48,48,48);  /* COLOR_LIGHTGRAY    7  */
  ChangeColorReg(8,32,32,32);  /* COLOR_GRAY         8  */
  ChangeColorReg(9,5,32,60);   /* COLOR_LIGHTBLUE    9  */
  ChangeColorReg(10,0,63,0);   /* COLOR_LIGHTGREEN   10 */
  ChangeColorReg(11,0,63,63);  /* COLOR_LIGHTCYAN    11 */
  ChangeColorReg(12,60,32,5);  /* COLOR_LIGHTRED     12 */
  ChangeColorReg(13,63,63,0);  /* COLOR_LIGHTMAGENTA 13 */
  ChangeColorReg(14,63,0,63);  /* COLOR_YELLOW       14 */
  ChangeColorReg(15,63,63,63); /* COLOR_WHITE        15 */
}

