/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\svga\sis.c
/* Last Update: 23.07.1999
/* Version    : 1.0 alpha
/* Coded by   : Filip Navara
/* Docus      : Internet / VGADOC4B
/************************************************************************/
/* Definition:
/*   A SiS SG86c201 graphic device driver.
/************************************************************************/
#include "svga\svgadev.h"

#define NULL (void*)0

int sis_total_memory;
unsigned char palette[768];

int sis_detect();
int sis_init();
void sis_set_bank_read(int bank);
void sis_set_bank_write(int bank);
int sis_mode_available(int mode);

// Device block
display_device_driver display_driver_sis =
{
    "SiS SG86c201",
    "Version 1.0 alpha",
    sis_detect,
    sis_init,
    NULL,
    sis_set_bank_read,
    sis_set_bank_write,
    NULL,
    sis_mode_available,

    65535,
    2,
    0,
    0
};

unsigned int rdinx(unsigned int inx)
{
    outportb(SEQ_I,inx);
    return inportb(SEQ_I+1);
}

void wrinx(unsigned int inx, unsigned int val)
{
    outportb(SEQ_I,inx);
    outportb(SEQ_I+1, val);
}

// Card detection
int sis_detect()
{
    unsigned int old, ret=0;

    old = rdinx(5);
    wrinx(5, 0);
    if (rdinx(5)==0x21)
    {
        wrinx(5, 0x86);
        if (rdinx(5)==0xA1)
        {
           ret=1;
        }
    }
    wrinx(5, old);

    return ret;
}

// Initialize the memory size variable
int sis_init()
{
    unsigned int old, ret=0;

    old = rdinx(5);
    wrinx(5, 0);
    if (rdinx(5)==0x21)
    {
        wrinx(5, 0x86);
        if (rdinx(5)==0xA1)
        {
            ret=1;
            switch (rdinx(0xF) & 3) {
                case 0: sis_total_memory=1; break;
                case 1: sis_total_memory=2; break;
                case 2: sis_total_memory=4; break;
            }
        }
    }
    wrinx(5, old);
    return ret;
}

// Set video page for reading
void sis_set_bank_read(int bank)
{
    outportb(0x3CB, bank);
}

// Set video page for writing
void sis_set_bank_write(int bank)
{
    outportb(0x3CD, bank); outportb(0x3CB, bank);
}

// Check if mode is available
int sis_mode_available(int mode)
{
    if (mode==0x2D) return 1;
    if (mode==0x2E) return 1;
    if (mode==0x2F) return 1;
    if (mode==0x30) return 1;
    if (mode==0x38) return 1;
    return 0;
}

// Write information about video card
void sis_info()
{
    show_msgf("SiS SG86c201 Card Informations:");
    show_msgf("  Memory: %d KB",sis_total_memory * 1024);
    show_msgf("");

    show_msgf("Video modes:");

    show_msgf(" 0x2D  640x350, 256 colors");
    show_msgf(" 0x2E  640x480, 256 colors");
    show_msgf(" 0x2F  640x400, 256 colors");
    show_msgf(" 0x30  800x600, 256 colors");
    show_msgf(" 0x38  1024x768, 256 colors");
}

/*
// Get video palette (NOT TESTED)
void get_palette(unsigned start,unsigned end,unsigned char *palette)
{
  unsigned i;

  outportb(0x03c7,start);
  start*=3; end++; end*=3;

  for (i=start;i<end;i++)
   palette[i] = inportb(0x03c9);
}

// Set background palette (NOT TESTED)
void set_backpal(int r1, int g1, int b1, int r2, int g2, int b2)
{
  int i;

  int rs, gs, bs, rv, gv, bv;
  char str[256];

  outportb(0x3c8,32);

  rs = ((r2 - r1)*100)/63; gs = ((g2 - g1)*100)/63; bs = ((b2 - b1)*100)/63;
  rv = r1*100; gv = g1*100; bv = b1*100;

  for (i=0;i<64;i++)
  {
   outportb(0x3c9, rv/100); outportb(0x3c9, gv/100); outportb(0x3c9, bv/100);
   rv += rs; gv += gs; bv += bs;
  }
}

// Set standard palette (NOT TESTED)
void std_pal()
{
  int i;

  outportb(0x3c8,0);

// COLOR_BLACK        0 

  outportb(0x3c9, 0); outportb(0x3c9, 0); outportb(0x3c9, 0);

// COLOR_BLUE         1

  outportb(0x3c9, 0); outportb(0x3c9, 0); outportb(0x3c9, 48);

// COLOR_GREEN        2

  outportb(0x3c9, 0); outportb(0x3c9, 32); outportb(0x3c9, 0);

// COLOR_CYAN         3

  outportb(0x3c9, 0); outportb(0x3c9, 32); outportb(0x3c9, 32);

// COLOR_RED          4

  outportb(0x3c9, 32); outportb(0x3c9, 0); outportb(0x3c9, 0);

// COLOR_MAGENTA      5

  outportb(0x3c9, 32); outportb(0x3c9, 32); outportb(0x3c9, 0);

// COLOR_BROWN        6

  outportb(0x3c9, 0); outportb(0x3c9, 0); outportb(0x3c9, 0);

// COLOR_LIGHTGRAY    7

  outportb(0x3c9, 48); outportb(0x3c9, 48); outportb(0x3c9, 48);

// COLOR_GRAY         8

  outportb(0x3c9, 32); outportb(0x3c9, 32); outportb(0x3c9, 32);

// COLOR_LIGHTBLUE    9

  outportb(0x3c9, 5); outportb(0x3c9, 32); outportb(0x3c9, 60);

// COLOR_LIGHTGREEN   10

  outportb(0x3c9, 0); outportb(0x3c9, 63); outportb(0x3c9, 0);

// COLOR_LIGHTCYAN    11

  outportb(0x3c9, 0); outportb(0x3c9, 63); outportb(0x3c9, 0);

// COLOR_LIGHTRED     12

  outportb(0x3c9, 60); outportb(0x3c9, 32); outportb(0x3c9, 5);

// COLOR_LIGHTMAGENTA 13

  outportb(0x3c9, 0); outportb(0x3c9, 63); outportb(0x3c9, 0);

// COLOR_YELLOW       14

  outportb(0x3c9, 0); outportb(0x3c9, 63); outportb(0x3c9, 0);

// COLOR_WHITE        15

  outportb(0x3c9, 63); outportb(0x3c9, 63); outportb(0x3c9, 63);

  get_palette(0,255,palette);
}

// Set palette (NOT TESTED)
void set_palette()
{
  int i;

  outportb(0x3c8,32);

  for (i=0;i<64;i++)
  {
   outportb(0x3c9,0);   
   outportb(0x3c9,0);   
   outportb(0x3c9,i);   
  }
}
*/
