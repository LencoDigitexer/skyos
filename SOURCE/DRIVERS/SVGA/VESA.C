/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\svga\vesa.c
/* Last Update: 04.12.1998
/* Version    : beta
/* Coded by   : Gregory Hayrapetian
/* Docus      : Internet
/************************************************************************/
/* Definition:
/*   A VESA 2.0 (VBE2) graphic device driver.
/************************************************************************/
#include "svga\svgadev.h"

#define PACKED __attribute__((packed))

#define real2pm(realptr, pmptr) pmptr = (realptr&0xffff)+((realptr&0xffff0000)>>12)

#define NULL (void*)0



struct VbeInfoBlock
{
 char   VESASignature[4]    PACKED;    // 0x81000
 short  VESAVersion         PACKED;         // 0x81004
 char   *OEMStringPtr       PACKED;       // 0x81006
 long   Capabilities        PACKED;        // 0x8100a
 unsigned short *VideoModePtr       PACKED;       // 0x8100e
 short  TotalMemory         PACKED;         // 0x81012

 //VBE 2.0
 short  OemSoftwareRev      PACKED;    // 0x81014
 char   *OemVendorNamePtr   PACKED;    // 0x81016
 char   *OemProductNamePtr  PACKED;    // 0x8101a
 char   *OemProductRevPtr   PACKED;    // 0x8101e
 char   reserved[222]       PACKED;
 char   OemData[256]        PACKED;

};

struct PM_Int
{
 unsigned short SetWindow PACKED;
 unsigned short SetDisplayStart PACKED;
 unsigned short SetPalette PACKED;
 unsigned short IOPrivInfo PACKED;
};

struct VbeModeInfo
{
  short   ModeAttributes PACKED;         /* Mode attributes                  */
  char    WinAAttributes PACKED;         /* Window A attributes              */
  char    WinBAttributes PACKED;         /* Window B attributes              */
  short   WinGranularity PACKED;         /* Window granularity in k          */
  short   WinSize PACKED;                /* Window size in k                 */
  short   WinASegment PACKED;            /* Window A segment                 */
  short   WinBSegment PACKED;            /* Window B segment                 */
  void    *WinFuncPtr PACKED;            /* Pointer to window function       */
  short   BytesPerScanLine PACKED;       /* Bytes per scanline               */
  short   XResolution PACKED;            /* Horizontal resolution            */
  short   YResolution PACKED;            /* Vertical resolution              */
  char    XCharSize PACKED;              /* Character cell width             */
  char    YCharSize PACKED;              /* Character cell height            */
  char    NumberOfPlanes PACKED;         /* Number of memory planes          */
  char    BitsPerPixel PACKED;           /* Bits per pixel                   */
  char    NumberOfBanks PACKED;          /* Number of CGA style banks        */
  char    MemoryModel PACKED;            /* Memory model type                */
  char    BankSize PACKED;               /* Size of CGA style banks          */
  char    NumberOfImagePages PACKED;     /* Number of images pages           */
  char    res1 PACKED;                   /* Reserved                         */
  char    RedMaskSize PACKED;            /* Size of direct color red mask    */
  char    RedFieldPosition PACKED;       /* Bit posn of lsb of red mask      */
  char    GreenMaskSize PACKED;          /* Size of direct color green mask  */
  char    GreenFieldPosition PACKED;     /* Bit posn of lsb of green mask    */
  char    BlueMaskSize PACKED;           /* Size of direct color blue mask   */
  char    BlueFieldPosition PACKED;      /* Bit posn of lsb of blue mask     */
  char    RsvdMaskSize PACKED;           /* Size of direct color res mask    */
  char    RsvdFieldPosition PACKED;      /* Bit posn of lsb of res mask      */
  char    DirectColorModeInfo PACKED;    /* Direct color mode attributes     */

  /* VESA 2.0 variables */
  unsigned int PhysBasePtr;           /* physical address for flat frame buffer */
  long    OffScreenMemOffset;            /* pointer to start of off screen memory */
  short   OffScreenMemSize;      /* amount of off screen memory in 1k units */
  char    res2[206] PACKED;              /* Pad to 256 byte block size       */
};


void (*pm_bank)(char);
void (*pm_setstart)(char);
void (*pm_setpalette)(char);

struct VbeInfoBlock *vbe_info;
struct PM_Int *pm_info;
struct VbeModeInfo *vbe_mode;

unsigned int active_bank = 0;

unsigned char palette[768];


void vesa_setpage_read(int bank);
void vesa_setpage_write(int bank);
int vesa_init();
int vesa_detect();

display_device_driver display_driver_vesa2b =
{
    "VESA2.0 Banked",
	"",
    vesa_detect,
    vesa_init,
    NULL,
    vesa_setpage_read,
    vesa_setpage_write,
    NULL,
    NULL,

	NULL,

    65535,
    1,
	0,
	0
};

void set_draw_table_vesa(display_draw_table *dt)
{
  dt->color_depth = 8;

}

display_device_driver display_driver_vesa2l =
{
    "VESA2.0 Linear Frame Buffer",
	"",
    vesa_detect,
    vesa_init,
    set_draw_table_vesa,
    NULL,
    NULL,
    NULL,
    NULL,

	NULL,

    0,
    0,
	1,
	0
};

void vesa_setpage_write(int bank)
{
 bank <<= 64;

 asm( "call *%0"
    :
    :"r" (pm_bank),"b"((unsigned short)(0)),"d"(bank)
    : "%eax","%ebx","%ecx","%edx","%esi","%edi");
}

void vesa_setpage_read(int bank)
{
 bank <<= 64;

 asm( "call *%0"
    :
    :"r" (pm_bank),"b"((unsigned short)(1)),"d"(bank)
    : "%eax","%ebx","%ecx","%edx","%esi","%edi");
}

void SetDisplayStart(unsigned int start)
{
 asm( "call *%0"
    :
    :"r" (pm_setstart),"b" (0),"c"((unsigned short)(start&0xfffff)), "d"((unsigned short)(start>>16))
    : "%eax","%ebx","%ecx","%edx","%esi","%edi");
}

void get_palette(unsigned start,unsigned end,unsigned char *palette)
{
  unsigned i;

  outportb(0x03c7,start);
  start*=3;
  end++;
  end*=3;

  for (i=start;i<end;i++)
   palette[i] = inportb(0x03c9);
}

void set_backpal(int r1, int g1, int b1, int r2, int g2, int b2)
{
  int i;

  int rs, gs, bs, rv, gv, bv;
  char str[256];

  outportb(0x3c8,32);

  rs = ((r2 - r1)*100)/63;
  gs = ((g2 - g1)*100)/63;
  bs = ((b2 - b1)*100)/63;

  rv = r1*100;
  gv = g1*100;
  bv = b1*100;

  for (i=0;i<64;i++)
  {
   outportb(0x3c9, rv/100);
   outportb(0x3c9, gv/100);
   outportb(0x3c9, bv/100);

   rv += rs;
   gv += gs;
   bv += bs;
  }
}

void vesa_stdpal()
{
  outportb(0x3c8,0);

/* COLOR_BLACK         0 */
   outportb(0x3c9, 0);
   outportb(0x3c9, 0);
   outportb(0x3c9, 0);

/* COLOR_BLUE         1 */
   outportb(0x3c9, 0);
   outportb(0x3c9, 0);
   outportb(0x3c9, 48);

/* COLOR_GREEN         2 */
   outportb(0x3c9, 0);
   outportb(0x3c9, 32);
   outportb(0x3c9, 0);

// #define COLOR_CYAN         3
   outportb(0x3c9, 0);
   outportb(0x3c9, 32);
   outportb(0x3c9, 32);

//#define COLOR_RED          4
   outportb(0x3c9, 32);
   outportb(0x3c9, 0);
   outportb(0x3c9, 0);

//#define COLOR_MAGENTA      5
   outportb(0x3c9, 32);
   outportb(0x3c9, 32);
   outportb(0x3c9, 0);

// #define COLOR_BROWN        6
   outportb(0x3c9, 0);
   outportb(0x3c9, 0);
   outportb(0x3c9, 0);

// #define COLOR_LIGHTGRAY    7
   outportb(0x3c9, 48);
   outportb(0x3c9, 48);
   outportb(0x3c9, 48);

// #define COLOR_GRAY         8
   outportb(0x3c9, 32);
   outportb(0x3c9, 32);
   outportb(0x3c9, 32);

// #define COLOR_LIGHTBLUE    9
   outportb(0x3c9, 5);
   outportb(0x3c9, 32);
   outportb(0x3c9, 60);

// #define COLOR_LIGHTGREEN   10
   outportb(0x3c9, 0);
   outportb(0x3c9, 63);
   outportb(0x3c9, 0);

// #define COLOR_LIGHTCYAN    11
   outportb(0x3c9, 0);
   outportb(0x3c9, 63);
   outportb(0x3c9, 63);

// #define COLOR_LIGHTRED     12
   outportb(0x3c9, 60); 
   outportb(0x3c9, 32);
   outportb(0x3c9, 5);

// #define COLOR_LIGHTMAGENTA 13
   outportb(0x3c9, 63);
   outportb(0x3c9, 63);
   outportb(0x3c9, 0);

// #define COLOR_YELLOW       14
   outportb(0x3c9, 63);
   outportb(0x3c9, 0);
   outportb(0x3c9, 63);

// #define COLOR_WHITE        15
   outportb(0x3c9, 63);
   outportb(0x3c9, 63);
   outportb(0x3c9, 63);

  get_palette(0,255,palette);

  set_backpal(0,0,0,0,0,63);

  get_palette(0,255,palette);
}


int vesa_detect(void)
{
  return 255;             // never present
}

int vesa_init()
{
    unsigned int *ptr_pm = (unsigned int*) 0x80500;

    ptr_pm = (unsigned int*) *ptr_pm;
    real2pm((int)ptr_pm,(int)ptr_pm);

    // VESA 2.0 Information
    vbe_info = (struct VbeInfoBlock*) 0x81000;
    vbe_mode = (struct VbeModeInfo*) 0x81500;

    real2pm((int)vbe_info->OEMStringPtr,(int)vbe_info->OEMStringPtr);
    real2pm((int)vbe_info->OemVendorNamePtr,(int)vbe_info->OemVendorNamePtr);
    real2pm((int)vbe_info->OemProductNamePtr,(int)vbe_info->OemProductNamePtr);
    real2pm((int)vbe_info->OemProductRevPtr,(int)vbe_info->OemProductRevPtr);
    real2pm((int)vbe_info->VideoModePtr,(int)vbe_info->VideoModePtr);

    // Protected Mode Interface
    pm_info = (struct PM_Int*) ptr_pm;

    pm_bank     = (void*)((char*)pm_info+pm_info->SetWindow);
    pm_setstart = (void*)((char*)pm_info+pm_info->SetDisplayStart);
    pm_setpalette=(void*)((char*)pm_info+pm_info->SetPalette);

    if (vbe_mode->NumberOfBanks == 2)
    {
      active_display_driver->bank_mode = 2;
      active_display_driver->setpage_read = vesa_setpage_read;
    }

  return SVGAOK;
}

unsigned int get_lfb_adr()
{
  return vbe_mode->PhysBasePtr;
}

void vesa_info()
{
    int i;
    char str[100], mstr[10];
    unsigned short *ModePtr;
    char str2[256];

    show_msgf("VESA Card Informations:");
    show_msgf("  VBE Version: %d.%d",vbe_info->VESAVersion>>8,vbe_info->VESAVersion & 0xff);
    show_msgf("  Memory: %d KB",vbe_info->TotalMemory*64);
    show_msgf("");
    show_msgf("  Rev.: %d.%d",vbe_info->OemSoftwareRev>>8, vbe_info->OemSoftwareRev & 0xff);
    show_msgf("  Manufacturer: %s",vbe_info->OEMStringPtr);
    show_msgf("  Vendor name:  %s",vbe_info->OemVendorNamePtr);
    show_msgf("  Product name: %s",vbe_info->OemProductNamePtr);
    show_msgf("  Product rev:  %s",vbe_info->OemProductRevPtr);
    show_msgf("");


    ModePtr = vbe_info->VideoModePtr;
    i = 0;
    str[0] = '\0';
    show_msgf("Video modes:");

    while (1)
    {
      sprintf(mstr," 0x%3x",*ModePtr);
      strcat(str,mstr);
      ModePtr++;
      if (*ModePtr == 0xffff) break;
      if (++i == 12)
      {
        show_msgf(str);
        str[0] = '\0';
        i = 0;
      }
    }
    show_msg(str);

}

void set_palette()
{
  int i;

  outportb(0x3c8,32);

  for (i=0;i<64;i++)
  {
   outportb(0x3c9,0);   /* RED   */
   outportb(0x3c9,0);   /* GREEN */
   outportb(0x3c9,i);   /* BLUE  */
  }
}

void out_sequencer(int index,int value)
{
  outportb(0x3c4,index);
  outportb(0x3c5,value);
}

unsigned char in_sequencer(int index)
{
  outportb(0x3c4,index);
  return inportb(0x3c5);
}

void screen_off()
{
  unsigned char old;

  old = in_sequencer(1);
  out_sequencer(1,old | 0x20);
}

void screen_on()
{
  unsigned char old;

  old = in_sequencer(1);
  out_sequencer(1,old & 0xDF);
}

void set_palette2(unsigned start,unsigned end,unsigned char *pal,unsigned char blend)
{
  unsigned i;

  outportb(0x03c8,start);
  start*=3;
  end++;
  end*=3;

  for (i=start;i<=end;i++)
    outportb(0x03c9,(pal[i]*blend) >> 7);
}

void blend_in()
{
  int bc, i;

  for (bc=0;bc<128;bc++)
  {
//    for (i=0;i<10000;i++) i = i;
    sleep(1);
    set_palette2(0,255,palette,bc);
  }
}

void blend_out()
{
  int bc, i;

  for (bc=128;bc>=0;bc--)
  {
//    for (i=0;i<10000;i++) i = i;
    sleep(1);
    set_palette2(0,255,palette,bc);
  }
}

