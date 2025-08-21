/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\svga\dms64.c
/* Last Update: 23.07.1999
/* Version    : 1.0 alpha
/* Coded by   : Filip Navara
/* Docus      : Internet / VGADOC4B
/************************************************************************/
/* Definition:
/*   A Diamond Stealth 64 Video 2001 series graphic device driver.
/************************************************************************/
#include "svga\svgadev.h"

#define NULL (void*)0

int dms64_total_memory;
unsigned char palette[768];

int dms64_detect();
int dms64_init();
void dms64_set_bank(int bank);
int dms64_mode_available(int mode);

// Device block
display_device_driver display_driver_dms64 =
{
    "Diamond Stealth 64",
    "Version 1.0 alpha",
    dms64_detect,
    dms64_init,
    NULL,
    dms64_set_bank,
    dms64_set_bank,
    NULL,
    dms64_mode_available,

    65535,
    1,
    0,
    0
};

// Card detection
int dms64_detect()
{
    int ID;

    asm(
        "int $0x10"
        : "=b" (ID)
        : "a" (0x1DAA), "b" (0xFDEC)
    );

    if (ID==0xCEDF) return 1; else return 0;
}

// Initialize the memory size variable
int dms64_init()
{
    int ID, INFO;

    asm(
        "int $0x10"
        : "=b" (ID), "=a" (INFO)
        : "a" (0x1DAA), "b" (0xFDEC)
    );

    dms64_total_memory=INFO & 0xFFFFFF00;

    if (ID==0xCEDF) return 1; else return 0;
}

// Set video page
void dms64_set_bank(int bank)
{
    int crtc;

    crtc=(inportb(0x3CC) & 1) ? CRT_IC : CRT_IM;

    outportb(crtc, 0x39); outportb(crtc+1, 0xA5);
    outportb(crtc, 0x6A); outportb(crtc+1, bank);
    outportb(crtc, 0x39); outportb(crtc+1, 0x5A);
}

// Check if mode is available
int dms64_mode_available(int mode)
{
    if (mode==0x68) return 1;
    if (mode==0x69) return 1;
    if (mode==0x6B) return 1;
    if (mode==0x6D) return 1;
    if (dms64_total_memory==2) {
        if (mode==0x6F) return 1;
        if (mode==0x7C) return 1;
    }
    return 0;
}

// Write information about video card
void dms64_info()
{
    show_msgf("Diamond Stealth 64 Card Informations:");
    show_msgf("  Memory: %d KB",dms64_total_memory * 1024);
    show_msgf("");

    show_msgf("Video modes:");

    show_msgf(" 0x68  640x400, 256 colors");
    show_msgf(" 0x69  640x480, 256 colors");
    show_msgf(" 0x6B  800x600, 256 colors");
    show_msgf(" 0x6D  1024x768, 256 colors");

    if (dms64_total_memory>=2) {
      show_msgf(" 0x6F  1280x1024, 256 colors");
      show_msgf(" 0x7C  1600x1200, 256 colors");
    }
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
