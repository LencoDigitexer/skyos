/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\svga\s3.c
/* Last Update: 27.07.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Allegro, svgalib
/************************************************************************/
/* Definition:
/*   A S3 device driver.
/************************************************************************/
#include "sched.h"
#include "svga\svgadev.h"
#include "svga\modes.h"

/* Indentify chipset; return non-zero if detected */
#define NULL (void*)0
#define outb outportb
#define outw outportw
#define inb inportb
#define inw inportw

#define port_out(value, port) outportb(port, value)
#define port_in(port) inportb(port)


//int svgadev_CRT_I = CRT_IC;             /* current CRT index register address */
//int svgadev_CRT_D = CRT_DC;                /* current CRT data register address */
//int svgadev_IS1_R = IS1_RC;                   /* current input status register address */

//extern int svgadev_CRT_I;		/* current CRT index register address */
//extern int svgadev_CRT_D;		/* current CRT data register address */
//extern int svgadev_IS1_R;               /* current input status register address */

/* Mode table */
#define ModeEntry94(res) { G##res, g##res##_regs94 }
#define ModeEntry96(res) { G##res, g##res##_regs96 }
#define LOOKUPMODE svgadev_mode_in_table

#define S3_LOCALBUS		0x01


/* S3*/

int s3_init(void);
int s3_detect(void);
static void s3_setpage_read(int page);
static void s3_setpage_write(int page);
static int s3_setmode(int mode);

static int s3_flags;
static int s3_chiptype;
static int s3_memory;

display_device_driver display_driver_s3 =
{
    "S3",
    "Alpha Version",
    s3_detect,
    s3_init,
    NULL,
    s3_setpage_read,
    s3_setpage_write,
    s3_setmode,
    NULL,

    65535,
    1,
    0,
    0
};

enum {
    S3_911, S3_924, S3_801, S3_805, S3_928, S3_864, S3_964, S3_TRIO32,
    S3_TRIO64, S3_866, S3_868, S3_968, S3_765
};

static const char *s3_chipname[] =
{"911", "924", "801", "805", "928",
 "864", "964", "Trio32", "Trio64", "866", "868", "968", "Trio64V+"};

/* Some port I/O functions: */
static unsigned char rdinx(int port, unsigned char index)
{
    outb(port, index);
    return inb(port + 1);
}

unsigned char svgadev_inCR(int index)
{
     outb(CRT_IC, index);
         return inb(CRT_DC);
     }

void svgadev_outCR(int index, unsigned char val)
{
     int v;
         v = ((int) val << 8) + index;
             outw(CRT_IC, v);
             }


static void wrinx(int port, unsigned char index, unsigned char val)
{
    outb(port, index);
    outb(port + 1, val);
}

/*
 * Returns true iff the bits in 'mask' of register 'port', index 'index'
 * are read/write.
 */
static int testinx2(int port, unsigned char index, unsigned char mask)
{
    unsigned char old, new1, new2;

    old = rdinx(port, index);
    wrinx(port, index, (old & ~mask));
    new1 = rdinx(port, index) & mask;
    wrinx(port, index, (old | mask));
    new2 = rdinx(port, index) & mask;
    wrinx(port, index, old);
    return (new1 == 0) && (new2 == mask);
}


static void s3_unlock(void)
{
    svgadev_outCR(0x38, 0x48);		/* Unlock special regs. */
    svgadev_outCR(0x39, 0xA5);		/* Unlock system control regs. */
}


int s3_detect(void)
{
    int vgaIOBase, vgaCRIndex, vgaCRReg;

    vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
    vgaCRIndex = vgaIOBase + 4;
    vgaCRReg = vgaIOBase + 5;

    outb(vgaCRIndex, 0x11);	/* for register CR11, (Vertical Retrace End) */
    outb(vgaCRReg, 0x00);	/* set to 0 */

    outb(vgaCRIndex, 0x38);	/* check if we have an S3 */
    outb(vgaCRReg, 0x00);

    /* Make sure we can't write when locked */

    if (testinx2(vgaCRIndex, 0x35, 0x0f))
	return 0;

    outb(vgaCRIndex, 0x38);	/* for register CR38, (REG_LOCK1) */
    outb(vgaCRReg, 0x48);	/* unlock S3 register set for read/write */

    /* Make sure we can write when unlocked */

    if (!testinx2(vgaCRIndex, 0x35, 0x0f))
	return 0;

    return 1;
}


int s3_init(void)
{
    int id, rev, config;
    char str[255];

    s3_unlock();

    s3_flags = 0;		/* initialize */
    id = svgadev_inCR(0x30);		/* Get chip id. */
    rev = id & 0x0F;
    if (id >= 0xE0) {
	id |= svgadev_inCR(0x2E) << 8;
	rev |= svgadev_inCR(0x2F) << 4;
    }

    s3_chiptype = -1;
    config = svgadev_inCR(0x36);	/* get configuration info */
    switch (id & 0xf0)
    {
       case 0x80:
	    if (rev == 1)
            {
		s3_chiptype = S3_911;
		break;
	    }
	    if (rev == 2)
            {
		s3_chiptype = S3_924;
		break;
	    }
	    break;
	case 0xa0:
	    switch (config & 0x03)
            {
	     case 0x00:
	     case 0x01:
		/* EISA or VLB - 805 */
		s3_chiptype = S3_805;
//		if ((rev & 0x0F) < 2)
//		    s3_flags |= S3_OLD_STEPPING;	/* can't handle 1152 width */
		break;
	    case 0x03:
		/* ISA - 801 */
		s3_chiptype = S3_801;
		/* Stepping same as 805, just ISA */
//		if ((rev & 0x0F) < 2)
//		    s3_flags |= S3_OLD_STEPPING;	/* can't handle 1152 width */
		break;
	    }
	    break;
	case 0x90:
	    s3_chiptype = S3_928;
//	    if ((rev & 0x0F) < 4)	/* ARI: Stepping D or below */
//		s3_flags |= S3_OLD_STEPPING;	/* can't handle 1152 width */
	    break;
	case 0xB0:
	    /* 928P */
	    s3_chiptype = S3_928;
	    break;
	case 0xC0:
	    s3_chiptype = S3_864;
	    break;
	case 0xD0:
	    s3_chiptype = S3_964;
	    break;
	case 0xE0:
	    switch (id & 0xFFF0) {
	    case 0x10E0:
		s3_chiptype = S3_TRIO32;
		break;
	    case 0x3DE0: /* ViRGE/VX ID */
	    case 0x31E0: /* ViRGE ID */
	    case 0x01E0: /* S3Trio64V2/DX ... any others? */
	    case 0x11E0:
		if (rev & 0x0400)
		    s3_chiptype = S3_765;
		else
		    s3_chiptype = S3_TRIO64;
		break;
	    case 0x80E0:
		s3_chiptype = S3_866;
		break;
	    case 0x90E0:
		s3_chiptype = S3_868;
		break;
	    case 0xF0E0:	/* XXXX From data book; XF86 says 0xB0E0? */
		s3_chiptype = S3_968;
		break;
	    }
	}
	if (s3_chiptype == -1)
        {
            return 2;
	}
	if (s3_chiptype <= S3_924)
        {
	    if ((config & 0x20) != 0)
		s3_memory = 512;
	    else
		s3_memory = 1024;
	} else {
	    /* look at bits 5, 6 and 7 */
	    switch ((config & 0xE0) >> 5) {
	    case 0:
		s3_memory = 4096;
		break;
	    case 2:
		s3_memory = 3072;
		break;
	    case 3:
		s3_memory = 8192;
		break;
	    case 4:
		s3_memory = 2048;
		break;
	    case 5:
		s3_memory = 6144;
		break;
	    case 6:
		s3_memory = 1024;
		break;
	    case 7:
		s3_memory = 512;
		break;		/* Trio32 */
	    }
	}

	if ((config & 0x03) < 3)	/* XXXX 928P is ignored */
	    s3_flags |= S3_LOCALBUS;

        show_msgf("s3.c: Using S3 driver (%s, %dK).", s3_chipname[s3_chiptype],
	       s3_memory);
//	if (s3_flags & S3_OLD_STEPPING)
//	    printf("svgalib: Chip revision cannot handle modes with width 1152.\n");
	if (s3_chiptype > S3_864)
          show_msgf("s3.c: chipsets newer than S3-864 is not supported well yet.");


        sprintf(str,"S3 Chipset %s. RAM: %dK",s3_chipname[s3_chiptype],
	       s3_memory);

        strcpy(display_driver_s3.desc, str);

/* begin: Initialize cardspecs. */
/*    cardspecs = malloc(sizeof(CardSpecs));
    cardspecs->videoMemory = s3_memory;
    cardspecs->nClocks = 0;
    /* cardspecs->maxHorizontalCrtc = 2040; SL: kills 800x600x32k and above */
/*    cardspecs->maxHorizontalCrtc = 4088;
    cardspecs->flags = INTERLACE_DIVIDE_VERT;

    /* Process S3-specific config file options. */
/*    __svgalib_read_options(s3_config_options, s3_process_option);

#ifdef INCLUDE_S3_TRIO64_DAC
    if ((s3_chiptype == S3_TRIO64 || s3_chiptype == S3_765) && dac_used == NULL)
	dac_used = &__svgalib_Trio64_methods;
#endif

    if (dac_used == NULL)
	dac_used = __svgalib_probeDacs(dacs_to_probe);
    else
	dac_used->initialize();


    if (dac_used == NULL) {
	/* Not supported. */
/*	printf("svgalib: s3: Assuming normal VGA DAC.\n");
#ifdef INCLUDE_NORMAL_DAC
	dac_used = &__svgalib_normal_dac_methods;
	dac_used->initialize();
#else
	printf("svgalib: Alas, normal VGA DAC support is not compiled in, goodbye.\n");
	return 1;
#endif
    }
    if (clk_used)
	clk_used->initialize(cardspecs, dac_used);

    dac_used->qualifyCardSpecs(cardspecs, dac_speed);

    /* Initialize standard clocks for unknown DAC. */
/*    if ((!(cardspecs->flags & CLOCK_PROGRAMMABLE))
	&& cardspecs->nClocks == 0) {
	/*
	 * Almost all cards have 25 and 28 MHz on VGA clocks 0 and 1,
	 * so use these for an unknown DAC, yielding 640x480x256.
	 */
/*	cardspecs->nClocks = 2;
	cardspecs->clocks = malloc(sizeof(int) * 2);
	cardspecs->clocks[0] = 25175;
	cardspecs->clocks[1] = 28322;
    }
    /* Limit pixel clocks according to chip specifications. */
//    if (s3_chiptype == S3_864 || s3_chiptype == S3_868) {
	/* Limit max clocks according to 95 MHz DCLK spec. */
	/* SL: might just be 95000 for 4/8bpp since no pixmux'ing */
/*	LIMIT(cardspecs->maxPixelClock4bpp, 95000 * 2);
	LIMIT(cardspecs->maxPixelClock8bpp, 95000 * 2);
	LIMIT(cardspecs->maxPixelClock16bpp, 95000);
	/* see explanation below */
//	LIMIT(cardspecs->maxPixelClock24bpp, 36000);
	/*
	 * The official 32bpp limit is 47500, but we allow
	 * 50 MHz for VESA 800x600 timing (actually the
	 * S3-864 doesn't have the horizontal timing range
	 * to run unmodified VESA 800x600 72 Hz timings).
	 */
/*	LIMIT(cardspecs->maxPixelClock32bpp, 50000);
    }
#ifndef S3_16_COLORS
    cardspecs->maxPixelClock4bpp = 0;	/* 16-color doesn't work. */
//#endif

/* end: Initialize cardspecs. */

//    __svgalib_driverspecs = &__svgalib_s3_driverspecs;


    return 2;
}


static void s3_setpage_read(int page)
{
}

static void s3_setpage_write(int page)
{
}

static int s3_setmode(int mode)
{
}
