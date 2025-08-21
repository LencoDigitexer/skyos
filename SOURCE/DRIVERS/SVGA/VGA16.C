/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\svga\vga16.c
/* Last Update: 03.08.1999
/* Version    : beta
/* Coded by   : Gregory Hayrapetian
/* Docus      : Internet
/************************************************************************/
/* Definition:
/*   A Standard VGA graphic device driver.
/************************************************************************/
#include "svga\svgadev.h"
#include "newgui.h"
#include "sched.h"

#define NULL (void*)0

#define out_GfxController(index,value)  { outportw(0x3ce,(value << 8) | index); }
#define in_InputStatus1                   inportb(0x3da);
#define out_AttrController(index,value) { in_InputStatus1;       \
                                          outportb(0x3c0,index); \
                                          outportb(0x3c0,value); \
                                          outportb(0x3c0,0x20);  }
#define in_AttrController(index,data)   { in_InputStatus1;\
                                          outportb(0x3c0,index); \
                                          data=inportb(0x3c1);   \
                                          outportb(0x3c0,data);  \
                                          outportb(0x3c0,0x20);  }

#define Select_WPlane(plane) { outportw(0x3c4, (plane << 8) | 2); }
#define Select_RPlane(plane) { outportw(0x3ce, (plane << 8) | 4); }


extern unsigned int MAXX, MAXY;

unsigned char *screen = (unsigned char*) 0xa0000;
unsigned char active_color;
int active_offset;
unsigned char bitplane[4];

unsigned char  VGA16getBitMask[8] =
{
  0xff, 0x7f, 0x3f, 0x1f, 0x0f, 7, 3, 1
};

unsigned char  VGA16getBitMask2[8] =
{
  0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
};

unsigned char  VGA16PointMask[8] =
{
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

int vga16_init();
int vga16_detect();
void set_draw_table();
int vga16_setmode(mode_info_t *mode);


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


mode_info_t vga16_modes[3] =
{ 
  MODE_ENTRY(640,480,4),
  MODE_ENTRY(800,600,4),
  MODE_TERMINATE
};

int vga16_detect(void)
{
   display_driver_vga16.mode_table = vga16_modes;

   return SVGAOK;           // always present
}


struct vgamode_t {
	char name[64];
	unsigned char Attribute[21];
	unsigned char CRTC[24];
	unsigned char Graphics[9];
	unsigned char MiscOutReg;
	unsigned char Sequencer[5];
};

struct vgamode_t vga_640x480x4 =
{
  "VGA 640x480x4",
  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, 0x38, 0x39, 0x3A, 0x3B,
   0x3C, 0x3D, 0x3E, 0x3F, 0x01, 0x00, 0x0F, 0x00, 0x00},
  {0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E, 0x00, 0x40, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xEA, 0x8C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3},
  {0x07, 0x00, 0x00, 0x00, 0x03, 0x03, 0x05, 0x0F, 0xFF},
   0xE3,
  {0x03, 0x01, 0x0F, 0x00, 0x06}
};

struct vgamode_t vga_800x600x4 =
{
  "VGA 800x600x4",
  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, 0x38, 0x39, 0x3A, 0x3B,
   0x3C, 0x3D, 0x3E, 0x3F, 0x01, 0x00, 0x0F, 0x00, 0x00},
  {0x7f, 0x63, 0x64, 0x82, 0x6b, 0x1b, 0x72, 0xf0, 0x00, 0x60, 0x00, 0x00,
   0x00, 0x00, 0xff, 0x00, 0x58, 0x8c, 0x57, 0x32, 0x00, 0x57, 0x00, 0xe3},
  {0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x05, 0x0F, 0xFF},
   0x2f,
  {0x03, 0x01, 0x0F, 0x00, 0x06}
};

struct vgamode_t vga_1024x768x8 =
{
  "VGA 640x480x4",
  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
   0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x02, 0x0F, 0x00, 0x00},
  {0xa3, 0x74, 0x74, 0x87, 0x83, 0x93, 0x24, 0xf5, 0x00, 0x60, 0x20, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x02, 0x86, 0xff, 0x40, 0x00, 0xff, 0x25, 0xc3},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF},
   0xEf,
  {0x03, 0x01, 0x0F, 0x00, 0x0e}
};


int vga_mode_set(struct vgamode_t *themode)
{
	int i;

	inportb(0x3DA);
	outportb(0x3C0, 0x00);

	outportb(0x3C2, (themode->MiscOutReg | 1));

	for (i=0; i<5; i++)
		outportw(0x3C4, ((themode->Sequencer[i] << 8) | i));

	outportw(0x3D4, (((themode->CRTC[17] & 0x7F) << 8) | 17));

	for (i=0; i<25; i++)
		outportw(0x3D4, ((themode->CRTC[i] << 8) | i));

	for (i=0; i<9; i++)
		outportw(0x3CE, ((themode->Graphics[i] << 8) | i));

	for (i=0; i<21; i++) {
		inportb(0x3DA);
		outportb(0x3C0, i);
		outportb(0x3C0, themode->Attribute[i]);
	}

	inportb(0x3DA);
	outportb(0x3C0, 0x20);

	return 1;
}


int vga16_setmode(mode_info_t *mode)
{
  const unsigned char *regs;
  int i,j;

  if (MODE_EQUAL(*mode,vga16_modes[0]))
  {
    vga_mode_set(&vga_640x480x4);
    regs = (const unsigned char *)  g640x480x16_regs;
  }
  else if (MODE_EQUAL(*mode,vga16_modes[1]))
  {
    vga_mode_set(&vga_800x600x4);
    regs = (const unsigned char *)  g800x600x16_regs;
  }


/*
  vga_screenoff();
  svgadev_setregs(regs);

  for (i=0;i<1000;i++)
	for (j=0;j<10000;j++)
		i = i;

  vga_screenon();
*/
  out_GfxController(5,3);

  active_color = 0xff;
  active_offset = 0xffffffff;

  return SVGAOK;
}


int vga16_init()
{
  display_driver_vga16.mode_table = vga16_modes;

  out_GfxController(5,3);

  active_color = 0xff;
  active_offset = 0xffffffff;

  return SVGAOK;
}

void set_pixel16(int x,int y,int color)
{
  unsigned int ScrOffs;
  unsigned char PointMask,Latch;
  int flags;

  save_flags(flags);
  cli();

  ScrOffs = (y* MAXX + x)>>3;
  PointMask = VGA16PointMask[x & 7];

  if (active_color != color)
  {
    out_GfxController(0,color);
    active_color = color;
  }

  Latch = screen[ScrOffs];
  screen[ScrOffs] = PointMask;

  restore_flags(flags);
}

void get_planes(int offset)
{
  register int i;

  for (i=0;i<4;i++)
  {
    out_GfxController(4,i);
    bitplane[i] = screen[offset];
  }
}

int get_pixel16(int x, int y)
{
  unsigned int offset,color=0;
  unsigned char point_mask;
  register int i;

  offset = (y * MAXX + x) >> 3;
  point_mask = VGA16PointMask[x & 7];

  /* active_offset should be reset by other function which overwrites the screen */
  if (active_offset != offset)
  {
    get_planes(offset);
    active_offset = offset;
  }

  for (i=3;i>=0;i--)
  {
    color <<= 1;
    if (bitplane[i] & point_mask) color++;
  }

  return color;
}

int get_pixel16a(int x, int y)
{
  unsigned int offset,color=0;
  unsigned char point_mask;
  register int i;

  offset = (y * MAXX + x) >> 3;
  point_mask = VGA16PointMask[x & 7];

  for (i=3;i>=0;i--)
  {
    color <<= 1;
    out_GfxController(4,i);
    if (screen[offset] & point_mask) color++;
  }

  return color;
}

void draw_hline16(int x1, int x2, int y,int color)
{
  FillArea(x1,y,x2,y,color,0);
}

void draw_vline16(int x, int y1, int y2,int color)
{
  FillArea(x,y1,x,y2,color,0);
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
  Vdisp16(rect.x1, rect.x2+dx, rect.x1+dx,rect.y1+dy,buf,xlen+1,rect.y2-rect.y1+1,0,0,xlen,rect.y2-rect.y1+1);

  vfree(buf);

}


void set_draw_table(display_draw_table *dt)
{
  dt->color_depth = 4;

  dt->draw_pixel			= set_pixel16;
  dt->get_pixel			= get_pixel16;
  dt->draw_hline			= draw_hline16;
  dt->draw_vline			= draw_vline16;
  dt->draw_fill_rect		= draw_fill_rect16;
  dt->copy_screen_rect	= copy_screen_rect16;
}

void Vdisp16(int srcx, int ex, int dx, int dy, char *s, int sw, int sl,
             int sx, int sy, int sdx, int sdy)
{
  unsigned char *dest;
  unsigned char *src, tmp, tmp2;
  int i,shift, yo, dest_ofs, src_ofs, maxx = MAXX>>3;

  dest_ofs = 0xa0000 + ((dy*MAXX + dx)>>3);
  src_ofs = s + sy*(sw*4) + sx;

  dest = (char *) dest_ofs;
  src = (char *) src_ofs;

  shift = abs((srcx & 7) - (dx & 7));

  if (shift != 0)
  {
    if ((srcx & 7) < (dx & 7))
    {
      for (yo=0; yo < sl*4; yo++)
      {
        tmp = ~VGA16getBitMask[dx & 7];
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

  out_GfxController(5,0);
  out_GfxController(8,0xff);

  sdx -= 2;

  for (i=0;i<4;i++)
  {
	dest = (char *) (dest_ofs + 1);
    src = (char *) (src_ofs + 1);

    Select_WPlane(1 << i);
    for (yo=0; yo < sdy; yo++)
	{
      src += sw * i;
      memcpy(dest,src,sdx);
      src += sw * (4-i);
      dest += MAXX>>3;
	}
  }

  out_GfxController(8,VGA16getBitMask[dx & 7]);

  for (i=0;i<4;i++)
  {
    dest = (char *) dest_ofs;
    src = (char *) src_ofs;

    Select_WPlane(1 << i);
    for (yo=0; yo < sdy; yo++)
	{
      src += sw * i;
      tmp = *dest;
      *dest = *src;
      src += sw * (4-i);
      dest += maxx;
	}
  }

  out_GfxController(8,VGA16getBitMask2[ex & 7]);

  sdx++;
  dest_ofs += sdx;
  src_ofs += sdx;

  for (i=0;i<4;i++)
  {
    dest = (char *) dest_ofs;
    src = (char *) src_ofs;

    Select_WPlane(1 << i);
    for (yo=0; yo < sdy; yo++)
	{
      src += sw * i;
      tmp = *dest;
      *dest = *src;
      src += sw * (4-i);
      dest += maxx;
	}
  }

  Select_WPlane(0x0f);
  out_GfxController(5,3);
  out_GfxController(8,0xff);
}

void Vcapt16( char *d, int dw, int dl, int dx, int dy,
                       int sx, int sy, int sdx, int sdy)
{
  char *dest;
  char *src;
  unsigned yo, i, src_ofs, dest_ofs, maxx = MAXX>>3;

  src_ofs = 0xa0000 + ((sy*MAXX + sx)>>3);
  dest_ofs = d + dy*dw*4 + dx;

  for (i=0;i<4;i++)
  {
    src = (char *)  src_ofs;
    dest = (char *) dest_ofs;

    Select_RPlane(i);
    for (yo=0; yo < sdy; yo++)
    {
      dest += dw * i;
      memcpy(dest, src, sdx);
      dest += dw * (4-i);
      src += maxx;
    }
  }
}

void area(int xa,int ya,int ye,int PointMask)
{
  unsigned int k,ScrOffs,Dummy;

  ScrOffs = (ya* MAXX + xa)>>3; 

  for(k=ya;k<=ye;k++) 
  {
    Dummy = screen[ScrOffs];
    screen[ScrOffs] = PointMask;
    ScrOffs += MAXX>>3;
  }
}

void FillArea(int xa,int ya,int xe,int ye,int color,int func)
{
  unsigned int temp;
  unsigned char PointMask,PointMask2;

  if (active_color != color)
  {
    out_GfxController(0,color);
    active_color = color;
  }

  if (func)
	out_GfxController(3,func<<3);

  PointMask = 0xff >> (xa & 7);

  if ((xe-xa) < (8-(xa & 7))) 
  {
	PointMask2 = 0xff >> ((xe&7)+1); 
    PointMask ^= PointMask2;
    area(xa, ya, ye, PointMask); 
  }
  else
  {
    area(xa, ya, ye, PointMask); 
    xa += 8-(xa & 7); 
    for(;xa+8<=xe;xa+=8) 
	  area(xa,ya,ye,0xff);
    PointMask = 0xff << 7-(xe & 7);
    area(xa, ya, ye, PointMask); 
  }
  
  if (func)
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


void vga16_stdpal()
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

