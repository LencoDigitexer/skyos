/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\svga\svgadev.c
/* Last Update: 08.08.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus from : Internet / Svgalib
/************************************************************************/
/* Definition:
/*   The main svga device driver file. Implements transparent
/*   SVGA device driver handling and some VGA driver functions.
/*   Main function: svga_init_device
/************************************************************************/
#include "sched.h"
#include "newgui.h"
#include "svga\svgadev.h"
#include "svga\modes.h"

#define NULL (void*)0
#define outb outportb
#define outw outportw
#define inb inportb
#define inw inportw

#define port_out(value, port) outportb(port, value)
#define port_in(port) inportb(port)

const int MiscOutputReg = 0x3c2;
const int DataReg = 0x3c0;
const int AddressReg = 0x3c0;


unsigned int MAXX = 1024;
unsigned int MAXY = 768;
unsigned int BPP = 8;

int active_bank_read;                        /* active bank of active svga card */
int active_bank_write;                       /* active bank of active svga card */

int svgadev_CRT_I = CRT_IC;		/* current CRT index register address */
int svgadev_CRT_D = CRT_DC;		/* current CRT data register address */
int svgadev_IS1_R = IS1_RC;		/* current input status register address */


/* default palette values */
static const unsigned char default_red[256]
=
{0, 0, 0, 0, 42, 42, 42, 42, 21, 21, 21, 21, 63, 63, 63, 63,
 0, 5, 8, 11, 14, 17, 20, 24, 28, 32, 36, 40, 45, 50, 56, 63,
 0, 16, 31, 47, 63, 63, 63, 63, 63, 63, 63, 63, 63, 47, 31, 16,
 0, 0, 0, 0, 0, 0, 0, 0, 31, 39, 47, 55, 63, 63, 63, 63,
 63, 63, 63, 63, 63, 55, 47, 39, 31, 31, 31, 31, 31, 31, 31, 31,
 45, 49, 54, 58, 63, 63, 63, 63, 63, 63, 63, 63, 63, 58, 54, 49,
 45, 45, 45, 45, 45, 45, 45, 45, 0, 7, 14, 21, 28, 28, 28, 28,
 28, 28, 28, 28, 28, 21, 14, 7, 0, 0, 0, 0, 0, 0, 0, 0,
 14, 17, 21, 24, 28, 28, 28, 28, 28, 28, 28, 28, 28, 24, 21, 17,
 14, 14, 14, 14, 14, 14, 14, 14, 20, 22, 24, 26, 28, 28, 28, 28,
 28, 28, 28, 28, 28, 26, 24, 22, 20, 20, 20, 20, 20, 20, 20, 20,
 0, 4, 8, 12, 16, 16, 16, 16, 16, 16, 16, 16, 16, 12, 8, 4,
 0, 0, 0, 0, 0, 0, 0, 0, 8, 10, 12, 14, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 14, 12, 10, 8, 8, 8, 8, 8, 8, 8, 8,
 11, 12, 13, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 13, 12,
 11, 11, 11, 11, 11, 11, 11, 11, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned char default_green[256]
=
{0, 0, 42, 42, 0, 0, 21, 42, 21, 21, 63, 63, 21, 21, 63, 63,
 0, 5, 8, 11, 14, 17, 20, 24, 28, 32, 36, 40, 45, 50, 56, 63,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 31, 47, 63, 63, 63, 63,
 63, 63, 63, 63, 63, 47, 31, 16, 31, 31, 31, 31, 31, 31, 31, 31,
 31, 39, 47, 55, 63, 63, 63, 63, 63, 63, 63, 63, 63, 55, 47, 39,
 45, 45, 45, 45, 45, 45, 45, 45, 45, 49, 54, 58, 63, 63, 63, 63,
 63, 63, 63, 63, 63, 58, 54, 49, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 7, 14, 21, 29, 28, 28, 28, 28, 28, 28, 28, 28, 21, 14, 7,
 14, 14, 14, 14, 14, 14, 14, 14, 14, 17, 21, 24, 28, 28, 28, 28,
 28, 28, 28, 28, 28, 24, 21, 17, 20, 20, 20, 20, 20, 20, 20, 20,
 20, 22, 24, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 26, 24, 22,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 8, 12, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 12, 8, 4, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 10, 12, 14, 16, 16, 16, 16, 16, 16, 16, 16, 16, 14, 12, 10,
 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 13, 15, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 15, 13, 12, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned char default_blue[256]
=
{0, 42, 0, 42, 0, 42, 0, 42, 21, 63, 21, 63, 21, 63, 21, 63,
 0, 5, 8, 11, 14, 17, 20, 24, 28, 32, 36, 40, 45, 50, 56, 63,
 63, 63, 63, 63, 63, 47, 31, 16, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 16, 31, 47, 63, 63, 63, 63, 63, 63, 63, 63, 63, 55, 47, 39,
 31, 31, 31, 31, 31, 31, 31, 31, 31, 39, 47, 55, 63, 63, 63, 63,
 63, 63, 63, 63, 63, 58, 54, 49, 45, 45, 45, 45, 45, 45, 45, 45,
 45, 49, 54, 58, 63, 63, 63, 63, 28, 28, 28, 28, 28, 21, 14, 7,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 14, 21, 28, 28, 28, 28,
 28, 28, 28, 28, 28, 24, 21, 17, 14, 14, 14, 14, 14, 14, 14, 14,
 14, 17, 21, 24, 28, 28, 28, 28, 28, 28, 28, 28, 28, 26, 24, 22,
 20, 20, 20, 20, 20, 20, 20, 20, 20, 22, 24, 26, 28, 28, 28, 28,
 16, 16, 16, 16, 16, 12, 8, 4, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 4, 8, 12, 16, 16, 16, 16, 16, 16, 16, 16, 16, 14, 12, 10,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 10, 12, 14, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 15, 13, 12, 11, 11, 11, 11, 11, 11, 11, 11,
 11, 12, 13, 15, 16, 16, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0};


/************************************************************************
/* Devices structures for each display device driver
/************************************************************************/

extern display_device_driver display_driver_vga16, 
							 display_driver_vesa2b, 
							 display_driver_vesa2l,
							 display_driver_tvga;

display_device_driver *registered_drivers[MAX_SVGA_DEVICES] = {0};

/* Pointer to active display driver */
display_device_driver *active_display_driver = NULL;

display_draw_table active_draw_table;

/* virtual address of linear frame buffer */
unsigned int lfb_virtual = 0x10000000;


void svgadev_delay(void)
{
    int i;
    for (i = 0; i < 10; i++);
}

int vga_screenoff(void)
{
    int tmp = 0;

    port_out(0x01, SEQ_I);
    port_out(port_in(SEQ_D) | 0x20, SEQ_D);
    port_in(svgadev_IS1_R);
    svgadev_delay();
    port_out(0x00, ATT_IW);
}

int vga_screenon(void)
{
    port_out(0x01, SEQ_I);
    port_out(port_in(SEQ_D) & 0xDF, SEQ_D);
    port_in(svgadev_IS1_R);
    svgadev_delay();
    port_out(0x20, ATT_IW);
}

int svgadev_setregs(const unsigned char *regs)
{
    int i;

    /* update misc output register */
    port_out(regs[MIS], MIS_W);

    /* synchronous reset on */
    port_out(0x00, SEQ_I);
    port_out(0x01, SEQ_D);

    /* write sequencer registers */
    port_out(1, SEQ_I);
    port_out(regs[SEQ + 1] | 0x20, SEQ_D);
    for (i = 2; i < SEQ_C; i++)
    {
	port_out(i, SEQ_I);
	port_out(regs[SEQ + i], SEQ_D);
    }

    /* synchronous reset off */
    port_out(0x00, SEQ_I);
    port_out(0x03, SEQ_D);

	/* deprotect CRT registers 0-7 */
	port_out(0x11, svgadev_CRT_I);
	port_out(port_in(svgadev_CRT_D) & 0x7F, svgadev_CRT_D);

    /* write CRT registers */
    for (i = 0; i < CRT_C; i++)
    {
	port_out(i, svgadev_CRT_I);
	port_out(regs[CRT + i], svgadev_CRT_D);
    }

    /* write graphics controller registers */
    for (i = 0; i < GRA_C; i++)
    {
	port_out(i, GRA_I);
	port_out(regs[GRA + i], GRA_D);
    }

    /* write attribute controller registers */
    for (i = 0; i < ATT_C; i++)
    {
	port_in(svgadev_IS1_R);		/* reset flip-flop */
	svgadev_delay();
	port_out(i, ATT_IW);
	svgadev_delay();
	port_out(regs[ATT + i], ATT_IW);
	svgadev_delay();
    }

    return 0;
}

void restorepalette(const unsigned char *red, const unsigned char *green, const unsigned char *blue)
{
    int i;

    /* restore saved palette */
    port_out(0, PEL_IW);

    /* read RGB components - index is autoincremented */
    for (i = 0; i < 256; i++) {
	svgadev_delay();
	port_out(*(red++), PEL_D);
	svgadev_delay();
	port_out(*(green++), PEL_D);
	svgadev_delay();
	port_out(*(blue++), PEL_D);
    }
}

void vga_set_standard_palette(void)
{
  restorepalette(default_red, default_green, default_blue);
}

int display_setmode(mode_info_t *mode)
{
  int ret;

  ret = active_display_driver->setmode(mode);

  if (ret == SVGAOK)
  {
    MAXX = mode->xdim;
    MAXY = mode->ydim;

    std_pal();
    newgui_update();
  }

  return ret;
}

int display_driver_init(display_device_driver *driver)
{
  int ret;

  ret = driver->init();

//  if (ret == SVGAOK)
  {
    active_bank_read = -1;
    active_bank_write = -1;

    active_display_driver = driver;
    init_draw_table();
  }

  return ret;
}

int display_device_init(int disp_drv, int mode)
{
  active_bank_read = -1;
  active_bank_write = -1;

  switch (disp_drv)
  {
    case DISP_VGA16: active_display_driver = &display_driver_vga16;
	             active_display_driver->init();

                     switch (mode)
		     {
                       case 0x12: MAXX = 640; MAXY = 480; break;
                       case 0x6a: MAXX = 800; MAXY = 600; break;
		     }
           BPP = 4;
                     break;
    case DISP_VESA2: active_display_driver = &display_driver_vesa2l;
	             active_display_driver->init();

                     switch (mode)
   		     {
                       case 0x101: MAXX = 640; MAXY = 480; break;
                       case 0x103: MAXX = 800; MAXY = 600; break;
                       case 0x105: MAXX = 1024; MAXY = 768; break;
                       case 0x107: MAXX = 1280; MAXY = 1024; break;
		     }
           BPP = 8;
		     break;
    default:    if (tvga_probe())
                       {
                         active_display_driver = &display_driver_tvga;

                         if (!active_display_driver->setmode(mode))
                           while(1);
                         else if (active_display_driver == NULL)
                           while(1);
                       }
                       BPP = 8;
                       break;
  }

  /* build draw table*/

  init_draw_table();
}

void std_pal()
{

  if (active_draw_table.color_depth == 4)
  {
	vga16_stdpal();
  }
  else if (active_draw_table.color_depth != 4)
  {
	vesa_stdpal();
  }
}

void setpix(int x, int y, int color)
{
  active_draw_table.draw_pixel(x,y,color);
}


void fdraw_hline(int x1, int x2, int y, int color)
{
  active_draw_table.draw_hline(x1,x2,y,color);
}

void fdraw_vline(int x, int y1, int y2, int color)
{
  int i;

  for (i=y1;i<=y2;i++)
    active_draw_table.draw_pixel(x,i,color);
}


void copy_screen_rect(rect_t rect, int dx, int dy)
{
  active_draw_table.copy_screen_rect(rect.x1,rect.y1,rect.x2,rect.y2,dx,dy);
}

int getpix(int x, int y)
{
  return active_draw_table.get_pixel(x,y);
}

void draw_pixel_b(int x, int y, int color)
{
  int addr = (int)y * MAXX + x;
  char *ptr = (char*) (0xa0000 + (addr & 0xFFFF));

  if (active_bank_write != (int)(addr >> 16))
  {
    active_bank_write = (int)(addr >> 16);
    active_display_driver->setpage_write(active_bank_write);
  }

  *ptr = (char)color;
}

void draw_pixel_l(int x, int y, int color)
{
  char *ptr = (char*) (lfb_virtual + y * MAXX + x);

  *ptr = (char)color;
}

int get_pixel_b(int x,int y)
{
  int addr = (int)y * MAXX + x;
  char *ptr = (char*) (0xa0000 + (addr & 0xFFFF));

  if (active_bank_write != (int)(addr >> 16))
  {
    active_bank_write = (int)(addr >> 16);
    active_display_driver->setpage_read(active_bank_write);
  }

  return (unsigned char) *ptr;
}

int get_pixel_bd(int x,int y)
{
  int addr = (int)y * MAXX + x;
  char *ptr = (char*) (0xa0000 + (addr & 0xFFFF));

  if (active_bank_read != (int)(addr >> 16))
  {
    active_bank_read = (int)(addr >> 16);
    active_display_driver->setpage_read(active_bank_read);
  }

  return (unsigned char) *ptr;
}

int get_pixel_l(int x,int y)
{
  char *ptr = (char*) (lfb_virtual + y * MAXX + x);

  return (unsigned char) (*ptr);
}

void draw_hline_b(int x1, int x2, int y,unsigned char color)
{
/*
  int addr = (int)y * MAXX + x1;
  unsigned int offset = (0xa0000 + (addr & 0xFFFF));

  if (active_bank_write != (int)(addr >> 16))
  {
    active_bank_write = (int)(addr >> 16);
    active_display_driver->setpage_write(active_bank_write);
  }

  memset(offset,color,x2-x1+1);
*/
  int i;

  for (i=x1;i<=x2;i++)
    active_draw_table.draw_pixel(i,y,color);
}

void draw_hline_l(int x1, int x2, int y,int color)
{
  memset(lfb_virtual + y * MAXX + x1,color,x2-x1+1);
}

void draw_fill_rect_l(int x1, int y1, int x2, int y2, int color)
{
  unsigned int i, ofs, xlen = x2-x1+1;

  ofs = lfb_virtual + y1 * MAXX + x1;

  for (i=y1;i<=y2;i++)
  {
    memset(ofs,color,xlen);
    ofs += MAXX;
  }
}

void draw_fill_rect_b(int x1, int y1, int x2, int y2, int color)
{
  int i,j;

  for (j=y1;j<=y2;j++)
	for (i=x1;i<=x2;i++)
	  draw_pixel_b(i,j,color);
}

void copy_screen_rect_b(int x1, int y1, int x2, int y2, int dx, int dy)
{
  int i, addr, xlen;
  char *ptr, *buffer;
  rect_t rect = {x1, y1, x2, y2};

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

  xlen = rect.x2 - rect.x1 + 1;
  buffer = (char*) valloc(xlen);

  if (dy < 0)
  {
    for (i=rect.y1;i<=rect.y2;i++)
    {
      addr = (int)i * MAXX + rect.x1;
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_write != (int)(addr >> 16))
      {
        active_bank_write = (int)(addr >> 16);
        active_display_driver->setpage_write(active_bank_write);
      }
      memcpy(buffer,ptr,xlen);

      addr = (int)(i+dy) * MAXX + (rect.x1+dx);
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_write != (int)(addr >> 16))
      {
        active_bank_write = (int)(addr >> 16);
        active_display_driver->setpage_write(active_bank_write);
      }
      memcpy(ptr,buffer,xlen);
    }
  }
  else
  {
    for (i=rect.y2;i>=rect.y1;i--)
    {
      addr = (int)i * MAXX + rect.x1;
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_write != (int)(addr >> 16))
      {
        active_bank_write = (int)(addr >> 16);
        active_display_driver->setpage_write(active_bank_write);
      }
      memcpy(buffer,ptr,xlen);

      addr = (int)(i+dy) * MAXX + (rect.x1+dx);
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_write != (int)(addr >> 16))
      {
        active_bank_write = (int)(addr >> 16);
        active_display_driver->setpage_write(active_bank_write);
      }
      memcpy(ptr,buffer,xlen);
    }
  }

  vfree(buffer);
}

/*
void copy_screen_rect_b(int x1, int y1, int x2, int y2, int dx, int dy)
{
  int i, addr, xlen;
  char *ptr, *buffer;
  rect_t rect = {x1, y1, x2, y2};

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

  xlen = rect.x2 - rect.x1 + 1;
  buffer = (char*) valloc(xlen);

  if (dy < 0)
  {
    for (i=rect.y1;i<=rect.y2;i++)
    {
      addr = (int)i * MAXX + rect.x1;
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_write != (int)(addr >> 16))
      {
        active_bank_write = (int)(addr >> 16);
        active_display_driver->setpage_write(active_bank_write);
      }
      memcpy(buffer,ptr,xlen);

      addr = (int)(i+dy) * MAXX + (rect.x1+dx);
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_write != (int)(addr >> 16))
      {
        active_bank_write = (int)(addr >> 16);
        active_display_driver->setpage_write(active_bank_write);
      }
      memcpy(ptr,buffer,xlen);
    }
  }
  else
  {
    for (i=rect.y2;i>=rect.y1;i--)
    {
      addr = (int)i * MAXX + rect.x1;
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_write != (int)(addr >> 16))
      {
        active_bank_write = (int)(addr >> 16);
        active_display_driver->setpage_write(active_bank_write);
      }
      memcpy(buffer,ptr,xlen);

      addr = (int)(i+dy) * MAXX + (rect.x1+dx);
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_write != (int)(addr >> 16))
      {
        active_bank_write = (int)(addr >> 16);
        active_display_driver->setpage_write(active_bank_write);
      }
      memcpy(ptr,buffer,xlen);
    }
  }

  vfree(buffer);
}
*/

void copy_screen_rect_bd(int x1, int y1, int x2, int y2, int dx, int dy)
{
  int i, addr, xlen;
  char *ptr, *buffer;
  rect_t rect = {x1, y1, x2, y2};

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

  xlen = rect.x2 - rect.x1 + 1;
  buffer = (char*) valloc(xlen);

  if (dy < 0)
  {
    for (i=rect.y1;i<=rect.y2;i++)
    {
      addr = (int)i * MAXX + rect.x1;
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_read != (int)(addr >> 16))
      {
        active_bank_read = (int)(addr >> 16);
        active_display_driver->setpage_read(active_bank_read);
      }
      memcpy(buffer,ptr,xlen);

      addr = (int)(i+dy) * MAXX + (rect.x1+dx);
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_write != (int)(addr >> 16))
      {
        active_bank_write = (int)(addr >> 16);
        active_display_driver->setpage_write(active_bank_write);
      }
      memcpy(ptr,buffer,xlen);
    }
  }
  else
  {
    for (i=rect.y2;i>=rect.y1;i--)
    {
      addr = (int)i * MAXX + rect.x1;
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_read != (int)(addr >> 16))
      {
        active_bank_read = (int)(addr >> 16);
        active_display_driver->setpage_read(active_bank_read);
      }
      memcpy(buffer,ptr,xlen);

      addr = (int)(i+dy) * MAXX + (rect.x1+dx);
      ptr = (char*) (0xa0000 + (addr & 0xFFFF));

      if (active_bank_write != (int)(addr >> 16))
      {
        active_bank_write = (int)(addr >> 16);
        active_display_driver->setpage_write(active_bank_write);
      }
      memcpy(ptr,buffer,xlen);
    }
  }

  vfree(buffer);
}


void copy_screen_rect_l(int x1, int y1, int x2, int y2, int dx, int dy)
{
  int i, j, xlen;
  char *ptr1, *ptr2, *buffer;
  rect_t rect = {x1,y1,x2,y2};

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

  xlen = rect.x2 - rect.x1 + 1;
  buffer = (char*) valloc(xlen);

  if (dy < 0)
  {
    ptr1 = (char*) (lfb_virtual + rect.y1 * MAXX + rect.x1);
    ptr2 = (char*) (lfb_virtual + (rect.y1+dy) * MAXX + (rect.x1+dx));

    for (i=rect.y1;i<=rect.y2;i++)
    {
      memcpy(buffer,ptr1,xlen);
      memcpy(ptr2,buffer,xlen);

      ptr1 += MAXX;
      ptr2 += MAXX;
    }
  }
  else
  {
    ptr1 = (char*) (lfb_virtual + rect.y2 * MAXX + rect.x1);
    ptr2 = (char*) (lfb_virtual + (rect.y2+dy) * MAXX + (rect.x1+dx));

    for (i=rect.y2;i>=rect.y1;i--)
    {
      memcpy(buffer,ptr1,xlen);
      memcpy(ptr2,buffer,xlen);

      ptr1 -= MAXX;
      ptr2 -= MAXX;
    }
  }

  vfree(buffer);
}


void init_draw_table()
{

  if (active_display_driver->use_lfb)
  {
	  active_draw_table.color_depth = 8;

	  active_draw_table.draw_pixel			= draw_pixel_l;
	  active_draw_table.get_pixel			= get_pixel_l;
	  active_draw_table.draw_hline			= draw_hline_l;
	  active_draw_table.draw_vline			= fdraw_vline;
	  active_draw_table.draw_fill_rect		= draw_fill_rect_l;
	  active_draw_table.copy_screen_rect	= copy_screen_rect_l;
  }
  else if (active_display_driver->bank_mode == 1)
  {
	  active_draw_table.color_depth = 8;

	  active_draw_table.draw_pixel			= draw_pixel_b;
	  active_draw_table.get_pixel			= get_pixel_b;
	  active_draw_table.draw_hline			= draw_hline_b;
	  active_draw_table.draw_vline			= fdraw_vline;
	  active_draw_table.draw_fill_rect		= draw_fill_rect_b;
	  active_draw_table.copy_screen_rect	= copy_screen_rect_b;
  }
  else if (active_display_driver->bank_mode == 2)
  {
	  active_draw_table.color_depth = 8;

	  active_draw_table.draw_pixel			= draw_pixel_b;
	  active_draw_table.get_pixel			= get_pixel_bd;
	  active_draw_table.draw_hline			= draw_hline_b;
	  active_draw_table.draw_vline			= fdraw_vline;
	  active_draw_table.draw_fill_rect		= draw_fill_rect_b;
	  active_draw_table.copy_screen_rect	= copy_screen_rect_bd;
  }

  if (active_display_driver->set_draw_table != NULL)
    active_display_driver->set_draw_table(&active_draw_table);

}


void svgadev_add_driver(struct display_device_driver* device)
{
  int i;

  for (i=0;i<MAX_SVGA_DEVICES;i++)
  {
    if (registered_drivers[i] == 0)
    {
      registered_drivers[i] = device;
      return;
    }
  }
}


void svgadev_init(void)
{
  // Here you must add all svga device drivers.
  // Note: prototype them with extern above.

  // Note: vga16 must be the first driver always
  svgadev_add_driver(&display_driver_vga16);

  // Note: vesa2b must be the second driver always
  svgadev_add_driver(&display_driver_vesa2b);

  // Note: vesa2l must be the third driver always
  svgadev_add_driver(&display_driver_vesa2l);

  svgadev_add_driver(&display_driver_tvga);
}



/*
void copy_screen_rect2(rect_t rect, int dx, int dy)
{
  int i, j, xlen;
  unsigned int ofs1, ofs2;

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

  xlen = rect.x2 - rect.x1 + 1;
  xlen /= 4;

  if (dy < 0)
  {

    ofs1 = (lfb_virtual + rect.y1 * MAXX + rect.x1);
    ofs2 = (lfb_virtual + (rect.y1+dy) * MAXX + (rect.x1+dx));

    for (i=rect.y1;i<=rect.y2;i++)
    {

     asm("cld\n\t"
         "movl %%edx, %%ecx\n\t"
         "rep ; movsl\n\t"
         :
         :"d" (xlen),"D" ((long) ofs2),"S" ((long) ofs1)
         : "cx","di","si","memory");

      ofs1 += MAXX;
      ofs2 += MAXX;
    }
  }
  else
  {
    ofs1 = (lfb_virtual + rect.y2 * MAXX + rect.x1);
    ofs2 = (lfb_virtual + (rect.y2+dy) * MAXX + (rect.x1+dx));

    for (i=rect.y2;i>=rect.y1;i--)
    {
     asm("cld\n\t"
         "movl %%edx, %%ecx\n\t"
         "rep ; movsl\n\t"
         :
         :"d" (xlen),"D" ((long) ofs2),"S" ((long) ofs1)
         : "cx","di","si","memory");

      ofs1 -= MAXX;
      ofs2 -= MAXX;
    }
  }

}
*/



/*
void setpix(int x, int y, int color)
{
  int addr = (int)y * MAXX + x;
  char *ptr = (char*) (0xa0000 + (addr & 0xFFFF));

  if (*active_bank_write != (int)(addr >> 16))
  {
    *active_bank_write = (int)(addr >> 16);
    active_display_driver->setpage_write(*active_bank_write);
  }

  *ptr = (char)color;
}

unsigned char getpix_norm(int x,int y)
{
  int addr = (int)y * MAXX + x;
  char *ptr = (char*) (0xa0000 + (addr & 0xFFFF));

  if (*active_bank_write != (int)(addr >> 16))
  {
    *active_bank_write = (int)(addr >> 16);
    active_display_driver->setpage_write(*active_bank_write);
  }

  return (unsigned char) *ptr;
}

unsigned char getpix(int x,int y)
{
  int addr = (int)y * MAXX + x;
  char *ptr = (char*) (0xa0000 + (addr & 0xFFFF));

  if (*active_bank_read != (int)(addr >> 16))
  {
    *active_bank_read = (int)(addr >> 16);
    active_display_driver->setpage_read(*active_bank_read);
  }

  return (unsigned char) *ptr;
}

int wsetpix(window_t *win, int x, int y, int color)
{
  int addr = (int)y * MAXX + x;
  char *ptr = (char*) (0xa0000 + (addr & 0xFFFF));
  int flags;

  if (*active_bank_write != (int)(addr >> 16))
  {
    *active_bank_write = (int)(addr >> 16);
    active_display_driver->setpage_write(*active_bank_write);
  }

  if (get_wflag(win, WF_STOP_OUTPUT))
    return 1;

  if (get_bitmask(win, win_offset(win,x,y)))
  {
    if (get_wflag(win, WF_STOP_OUTPUT))
      return 1;

    save_flags(flags);
    cli();

    *ptr = (char)color;

    restore_flags(flags);
  }

  return 0;
}

void fdraw_hline(int x1, int x2, int y,unsigned char color)
{
  int addr = (int)y * MAXX + x1;
  unsigned int offset = (0xa0000 + (addr & 0xFFFF));

  if (*active_bank_write != (int)(addr >> 16))
  {
    *active_bank_write = (int)(addr >> 16);
    active_display_driver->setpage_write(*active_bank_write);
  }

  memset(offset,color,x2-x1+1);
}

*/
