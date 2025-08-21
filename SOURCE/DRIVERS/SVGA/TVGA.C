/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\svga\tvga.c
/* Last Update: 04.12.1998
/* Version    : release
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Allegro, svgalib
/************************************************************************/
/* Definition:
/*   A Trident 8900/9440/9680 device driver.
/*   64K Bank Switch.
/************************************************************************/
#include "devman.h"
#include "sched.h"
#include "svga\svgadev.h"
#include "svga\modes.h"
#include "svga\t8900.reg"
#include "svga\t9440.reg"
#include "svga\t9680.reg"

/* Indentify chipset; return non-zero if detected */
#define NULL (void*)0
#define outb outportb
#define outw outportw
#define inb inportb
#define inw inportw

#define port_out(value, port) outportb(port, value)
#define port_in(port) inportb(port)

extern int svgadev_CRT_I;		/* current CRT index register address */
extern int svgadev_CRT_D;		/* current CRT data register address */
extern int svgadev_IS1_R;               /* current input status register address */

static int tvga_memory;	/* amount of video memory in K */
static int tvga_nonint;	/* nonzero if non-interlaced jumper set */

static int reg_0c = 0xad;	/* value for 256k cards */

static int tvga_model = 8900;	/* set to 8900, 9440, or 9680 based on model */

/* Mode table */
#define ModeEntry94(res) { G##res, g##res##_regs94 }
#define ModeEntry96(res) { G##res, g##res##_regs96 }
#define LOOKUPMODE tvga_mode_in_table

/* TRIDENT 8900/9440/9680*/
int tvga_probe(void);
int tvga_detect(void);
void tvga_setpage_read(int page);
void tvga_setpage_write(int page);
int tvga_setmode(mode_info_t *nmode);
int tvga_mode_available(int mode);

display_device_driver display_driver_tvga =
{
    "Trident",
    "8900/9440/9680",
    tvga_detect,
    tvga_probe,
    NULL,
    tvga_setpage_read,
    tvga_setpage_write,
    tvga_setmode,
    tvga_mode_available,

	NULL,

    65535,
    1,
	0,
	0
};

mode_info_t tvga_mode_table[5] =
{ 
  MODE_ENTRY(640,480,8),
  MODE_ENTRY(800,600,8),
  MODE_ENTRY(1024,768,8),
  MODE_ENTRY(1280,1024,8),
  MODE_TERMINATE
};


static ModeTable tvga_modes_2048_96[] =
{				/* 2M modes for the 9680 */
    ModeEntry96(800x600x16),
    ModeEntry96(1024x768x16),
    ModeEntry96(1280x1024x16),
    ModeEntry96(1600x1200x16),
    ModeEntry96(640x480x256),
    ModeEntry96(800x600x256),
    ModeEntry96(1024x768x256),
    ModeEntry96(1280x1024x256),
    ModeEntry96(1600x1200x256),
    ModeEntry96(320x200x32K),
    ModeEntry96(640x480x32K),
    ModeEntry96(800x600x32K),
    ModeEntry96(1024x768x32K),
    ModeEntry96(320x200x64K),
    ModeEntry96(640x480x64K),
    ModeEntry96(800x600x64K),
    ModeEntry96(1024x768x64K),
    ModeEntry96(320x200x16M),
    ModeEntry96(640x480x16M),
    ModeEntry96(800x600x16M),
    END_OF_MODE_TABLE
};

static ModeTable tvga_modes_1024_96[] =
{				/* 1M modes for the 9680 */
    ModeEntry96(800x600x16),
    ModeEntry96(1024x768x16),
    ModeEntry96(1280x1024x16),
    ModeEntry96(1600x1200x16),
    ModeEntry96(640x480x256),
    ModeEntry96(800x600x256),
    ModeEntry96(1024x768x256),
    ModeEntry96(320x200x32K),
    ModeEntry96(640x480x32K),
    ModeEntry96(800x600x32K),
    ModeEntry96(320x200x64K),
    ModeEntry96(640x480x64K),
    ModeEntry96(800x600x64K),
    ModeEntry96(320x200x16M),
    ModeEntry96(640x480x16M),
    END_OF_MODE_TABLE
};

static ModeTable tvga_modes_512_96[] =
{				/* 512K modes for the 9680 */
    ModeEntry96(800x600x16),
    ModeEntry96(1024x768x16),
    ModeEntry96(640x480x256),
    ModeEntry96(800x600x256),
    ModeEntry96(320x200x32K),
    ModeEntry96(320x200x64K),
    ModeEntry96(320x200x16M),
    END_OF_MODE_TABLE
};

static ModeTable tvga_modes_1024_94[] =
{				/* 1M modes for the 9440 */
    ModeEntry94(800x600x16),
    ModeEntry94(1024x768x16),
    ModeEntry94(1280x1024x16),
    ModeEntry94(1600x1200x16),
    ModeEntry94(640x480x256),
    ModeEntry94(800x600x256),
    ModeEntry94(1024x768x256),
    ModeEntry94(320x200x32K),
    ModeEntry94(640x480x32K),
    ModeEntry94(800x600x32K),
    ModeEntry94(320x200x64K),
    ModeEntry94(640x480x64K),
    ModeEntry94(800x600x64K),
    ModeEntry94(320x200x16M),
    ModeEntry94(640x480x16M),
    END_OF_MODE_TABLE
};

static ModeTable tvga_modes_512_94[] =
{				/* 512K modes for the 9440 */
    ModeEntry94(800x600x16),
    ModeEntry94(1024x768x16),
    ModeEntry94(640x480x256),
    ModeEntry94(800x600x256),
    ModeEntry94(320x200x32K),
    ModeEntry94(320x200x64K),
    ModeEntry94(320x200x16M),
    END_OF_MODE_TABLE
};

static ModeTable tvga_modes_1024[] =
{				/* 1Mb, non-interlace jumper set */
/* *INDENT-OFF* */
    OneModeEntry(640x480x256),
    OneModeEntry(800x600x256),
    OneModeEntry(1024x768x256),
    END_OF_MODE_TABLE
/* *INDENT-ON* */
};

#define INTERL(res,i) { G##res, g##res##i##_regs }

static ModeTable tvga_modes_1024i[] =
{				/* 1Mb, jumper set to interlaced */
/* *INDENT-OFF* */
    INTERL(640x480x256, i),
    INTERL(800x600x256, i),
    INTERL(1024x768x256, i),
    END_OF_MODE_TABLE
/* *INDENT-ON* */
};

static ModeTable tvga_modes_512[] =
{				/* 512K */
/* *INDENT-OFF* */
    INTERL(640x480x256, i),
    INTERL(800x600x256, i1),
    END_OF_MODE_TABLE
/* *INDENT-ON* */
};

static ModeTable *tvga_modes = NULL;

/* Set chipset-specific registers */
static void tvga_setregs(const unsigned char regs[], int mode)
{
    int i;
    int crtc31 = 0;

    /* 7 extended CRT registers */
    /* 4 extended Sequencer registers (old and new) */
    /* CRTC reg 0x1f is apparently dependent */
    /* on the amount of memory installed. */

    switch (tvga_memory >> 8)
    {
    case 1:
	crtc31 = 0x94;
	reg_0c = 0xad;
	break;			/* 256K */
    case 2:
    case 3:
	crtc31 = 0x98;
	reg_0c = 0xcd;
	break;			/* 512/768K */
    case 4:			/* 1024K */
	crtc31 = 0x18;
	reg_0c = 0xcd;
	if (mode == G1024x768x256)
	{
	    reg_0c = 0xed;
	    crtc31 = 0x98;
	} else if (mode == G640x480x256 || mode == G800x600x256)
	    reg_0c = 0xed;
	break;
    }

    if (tvga_model == 9440 || tvga_model == 9680) goto tvga9440_setregs;

    if (mode == TEXT) {
	reg_0c = regs[EXT + 11];
	crtc31 = regs[EXT + 12];
    }

#ifdef REG_DEBUG
    printf("Setting extended registers\n");
#endif

    /* write extended CRT registers */
    for (i = 0; i < 7; i++)
    {
	port_out(0x18 + i, svgadev_CRT_I);
	port_out(regs[EXT + i], svgadev_CRT_D);
    }

    /* update sequencer mode regs */
    port_out(0x0b, SEQ_I);	/* select old regs */
    port_out(port_in(SEQ_D), SEQ_D);
    port_out(0x0d, SEQ_I);	/* old reg 13 */
    port_out(regs[EXT + 7], SEQ_D);
    port_out(0x0e, SEQ_I);	/* old reg 14 */
#if 0
    port_out(regs[EXT + 8], SEQ_D);
#endif
    port_out(((port_in(SEQ_D) & 0x08) | (regs[EXT + 8] & 0xf7)), SEQ_D);


    port_out(0x0b, SEQ_I);
    port_in(SEQ_D);		/* select new regs */

    if (tvga_memory > 512)
    {
	port_out(0x0e, SEQ_I);	/* set bit 7 of reg 14  */
	port_out(0x80, SEQ_D);	/* to enable writing to */
	port_out(0x0c, SEQ_I);	/* reg 12               */
	port_out(reg_0c, SEQ_D);
    }
    /*      outw(0x3c4, 0x820e); *//* unlock conf. reg */
    /*      port_out(0x0c, SEQ_I); *//* reg 12 */

    port_out(0x0d, SEQ_I);	/* new reg 13 */
    port_out(regs[EXT + 9], SEQ_D);
    port_out(0x0e, SEQ_I);	/* new reg 14 */
    port_out(regs[EXT + 10], SEQ_D);

#ifdef REG_DEBUG
    printf("Now setting last two extended registers.\n");
#endif

    /* update CRTC reg 1f */
    port_out(0x1f, svgadev_CRT_I);
    port_out((port_in(svgadev_CRT_D) & 0x03) | crtc31, svgadev_CRT_D);

    return;


    tvga9440_setregs:

    /* update sequencer mode regs */
    port_out(0x0D, SEQ_I);
    port_out(regs[EXT + 1], SEQ_D);
    port_out(0x0E, SEQ_I);
    port_out(regs[EXT + 2], SEQ_D);
    port_out(0x0F, SEQ_I);
    port_out(regs[EXT + 3], SEQ_D);
// you cant write to this anyways... it messes things up...
//    port_out(0x0B, SEQ_I);
//    port_out(regs[EXT + 0], SEQ_D);

    /* write extended CRT registers */
    port_out(0x19, svgadev_CRT_I);
    port_out(regs[EXT + 4], svgadev_CRT_D);
    port_out(0x1E, svgadev_CRT_I);
    port_out(regs[EXT + 5], svgadev_CRT_D);
    port_out(0x1F, svgadev_CRT_I);
    port_out(regs[EXT + 6], svgadev_CRT_D);
    port_out(0x21, svgadev_CRT_I);
    port_out(regs[EXT + 7], svgadev_CRT_D);
    port_out(0x25, svgadev_CRT_I);
    port_out(regs[EXT + 8], svgadev_CRT_D);
    port_out(0x27, svgadev_CRT_I);
    port_out(regs[EXT + 9], svgadev_CRT_D);
    port_out(0x29, svgadev_CRT_I);
    port_out(regs[EXT + 10], svgadev_CRT_D);
    /* Extended regs 11/12 are clobbered by vga.c */
    port_out(0x2A, svgadev_CRT_I);
    port_out(regs[EXT + 13], svgadev_CRT_D);
    port_out(0x2F, svgadev_CRT_I);
    port_out(regs[EXT + 14], svgadev_CRT_D);
    port_out(0x30, svgadev_CRT_I);
    port_out(regs[EXT + 15], svgadev_CRT_D);
    port_out(0x36, svgadev_CRT_I);
    port_out(regs[EXT + 16], svgadev_CRT_D);
    port_out(0x38, svgadev_CRT_I);
    port_out(regs[EXT + 17], svgadev_CRT_D);
    port_out(0x50, svgadev_CRT_I);
    port_out(regs[EXT + 18], svgadev_CRT_D);

    /* grfx controller */
    port_out(0x0F, GRA_I);
    port_out(regs[EXT + 19], GRA_D);
    port_out(0x2F, GRA_I);
    port_out(regs[EXT + 20], GRA_D);

    /* unprotect 3DB */
    port_out(0x0F, GRA_I);
    port_out(port_in(GRA_D) | 0x04, GRA_D);
    /* allow user-defined clock rates */
    port_out((port_in(0x3DB) & 0xFC) | 0x02, 0x3DB);

    /* trident specific ports */
    port_out(regs[EXT + 21], 0x43C8);
    port_out(regs[EXT + 22], 0x43C9);
    port_out(regs[EXT + 23], 0x83C6);
    port_out(regs[EXT + 24], 0x83C8);
    for(i=0;i<5;i++)
	port_in(PEL_MSK);
    port_out(regs[EXT + 25], PEL_MSK);
    port_out(0xFF, PEL_MSK);
}

/* Bank switching function - set 64K bank number */

void tvga_setpage_write(int page)
{
    port_out(0x0b, SEQ_I);
    port_out(port_in(SEQ_D), SEQ_D);
    port_in(SEQ_D);		/* select new mode regs */

    port_out(0x0e, SEQ_I);
    port_out(page ^ 0x02, SEQ_D);	/* select the page */
}

void tvga_setpage_read(int page)
{
    port_out(0x0b, SEQ_I);
    port_out(port_in(SEQ_D), SEQ_D);
    port_in(SEQ_D);		/* select new mode regs */

    port_out(0x0e, SEQ_I);
    port_out(page ^ 0x02, SEQ_D);	/* select the page */
}

const unsigned char *tvga_mode_in_table(const ModeTable * modes, int mode)
{
    while (modes->regs != NULL)
    {
	if (modes->mode_number == mode)
	    return modes->regs;
	modes++;
    }
    return NULL;
}

int tvga_mode_available(int mode)
{
  int i=0;
  const unsigned char *regs;

  regs = (const unsigned char *)tvga_mode_in_table(tvga_modes, mode);

  if (regs == NULL) return SVGAERROR_MODE_NOT_SUPPORTED;
  else return SVGAOK;
}

int tvga_setmode(mode_info_t *nmode)
{
    const unsigned char *regs;
    int i;
    int mode;

    if (nmode->xdim == 640)
      mode = G640x480x256;
    if (nmode->xdim == 800)
      mode = G800x600x256;
    if (nmode->xdim == 1024)
      mode = G1024x768x256;
    if (nmode->xdim == 1280)
      mode = G1280x1024x256;

    regs = (const unsigned char *)LOOKUPMODE(tvga_modes, mode);

    if (regs == NULL)
    {
      return -1;
    }

    vga_screenoff();
    svgadev_setregs(regs);
    tvga_setregs(regs, mode);
    {
      int j;
      for (j=0;j<=100000000;j++);
    }
    vga_set_standard_palette();

    vga_screenon();
    return SVGAOK;
}

/* select the correct register table */
static void setup_registers(void)
{
    if (tvga_modes == NULL)
    {
	if (tvga_model == 9440)
	{
	    if (tvga_memory < 1024)
		tvga_modes = tvga_modes_512_94;
	    else
		tvga_modes = tvga_modes_1024_94;
	}
	else if (tvga_model == 9680)
	{
	    if (tvga_memory < 1024)
		tvga_modes = tvga_modes_512_96;
	    else if (tvga_memory < 2048)
		tvga_modes = tvga_modes_1024_96;
	    else
		tvga_modes = tvga_modes_2048_96;
	}
	else
	{
	    if (tvga_memory < 1024)
		tvga_modes = tvga_modes_512;
	    if (tvga_nonint)
		tvga_modes = tvga_modes_1024;
	    else
		tvga_modes = tvga_modes_1024i;
	}
    }
}

static int tvga_init(int force, int par1, int par2)
{
    int id;

    if (force)
    {
#ifdef DEBUG
	printf("Forcing memory to %dK\n", par1);
#endif
	tvga_memory = par1;
	tvga_nonint = par2 & 1;
/*
par2 specs
bit 31-3:	reserved
bit 2-1:	00 - force 8900
		01 - force 9440
		10 - force 9680
		11 - reserved
bit 0:		force noninterlaced if set
		only affects the 8900
*/
	switch (par2 & 6)
	{
	    case 0: tvga_model=8900;
	    break;
	    case 2: tvga_model=9440;
	    break;
	    case 4: tvga_model=9680;
	    break;
	    case 6: tvga_model=0;
	    break;
	}
    }
    else
    {
	port_out(0x1f, svgadev_CRT_I);

	/* this should detect up to 2M memory now */
	tvga_memory = (port_in(svgadev_CRT_D) & 0x07) * 256 + 256;

	/* Now is the card running in interlace mode? */
	port_out(0x0f, SEQ_I);
	tvga_nonint = port_in(SEQ_D) & 0x04;

	/* check version */
	outw(0x3c4, 0x000b);
	switch (inb(0x3c5))
	{
	  case 0x02:			/* 8800cs */
   	  case 0x03:			/* 8900b */
	  case 0x04:			/* 8900c */
	  case 0x13:
	  case 0x33:			/* 8900cl */
	  case 0x23:			/* 9000 */
	    tvga_model = 8900;
	    break;
          case 0xD3:
/* I know 0xD3 is a 9680, but if your 9680 is detected as a */
/* 9440, EMAIL ME! (ark) and I will change this...          */
	    tvga_model = 9680;
	    break;
	default:
/* Whatever the original 8900 driver thought was */
/* an invalid version, we will use as a 9440.    */
	    tvga_model = 9440;
	}
    }

    /* The 9440 uses ports 43C8 and 43C9... we need more privileges */

    if(tvga_model == 9440)
      sprintf(display_driver_tvga.name,"Trident 9440 driver (%dK)",
		tvga_memory);
    else if(tvga_model == 9680)
      sprintf(display_driver_tvga.name,"Trident Driver with Chipset 9680 and %dKB memory",
		tvga_memory);
   else
      sprintf(display_driver_tvga.name," Trident 8900/9000 driver (%dK, %sinterlaced).",
		tvga_memory, (tvga_nonint) ? "non-" : "");


    id = register_hardware(DEVICE_IDENT_MULTIMEDIA, display_driver_tvga.name, 0);
//    register_memory(id, 0xB0000, 0xBFFFF);
//    register_memory(id, 0xA0000, 0xAFFFF);

      //svgadev_driverspecs = &svgadev_tvga_driverspecs;

    setup_registers();

    return SVGAOK;
}

int tvga_probe(void)
{
    int origVal, newVal;
    int save0b;
    int ret;
    /* 
     * Check first that we have a Trident card.
     */
    outb(0x3c4, 0x0b);
    save0b = inb(0x3c5);
    outw(0x3C4, 0x000B);	/* Switch to Old Mode */
    inb(0x3C5);			/* Now to New Mode */
    outb(0x3C4, 0x0E);
    origVal = inb(0x3C5);
    outb(0x3C5, 0x00);
    newVal = inb(0x3C5) & 0x0F;
    outb(0x3C5, (origVal ^ 0x02));


    if (newVal != 2)
    {
	outb(0x3c5, origVal);
	outb(0x3c4, 0x0b);
	outb(0x3c5, save0b);
	return 0;
    }


    ret = tvga_init(0, 0, 0);
    return ret;
}

int tvga_detect(void)
{
    int origVal, newVal;
    int save0b;
    /* 
     * Check first that we have a Trident card.
     */
    outb(0x3c4, 0x0b);
    save0b = inb(0x3c5);
    outw(0x3C4, 0x000B);	/* Switch to Old Mode */
    inb(0x3C5);			/* Now to New Mode */
    outb(0x3C4, 0x0E);
    origVal = inb(0x3C5);
    outb(0x3C5, 0x00);
    newVal = inb(0x3C5) & 0x0F;
    outb(0x3C5, (origVal ^ 0x02));

    display_driver_tvga.mode_table = tvga_mode_table;

    if (newVal != 2)
    {
	outb(0x3c5, origVal);
	outb(0x3c4, 0x0b);
	outb(0x3c5, save0b);
	return 0;
    }
    return SVGAOK;
}

