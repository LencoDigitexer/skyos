/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\newgui.h
/* Last Update: 08.08.1999
/* Version    : alpha
/* Coded by   : Gregory Hayrapetian
/* Docus      :
/************************************************************************/


typedef struct mode_info
{
  unsigned int xdim;
  unsigned int ydim;
  unsigned int bpp;
} mode_info_t;


#define MODE_ENTRY(xdim,ydim,bpp)	{xdim, ydim, bpp}
#define MODE_TERMINATE				{0, 0, 0}
#define MODE_ENTRY_VALID(mtab)		((mtab).xdim && (mtab).ydim && (mtab).bpp)
#define MODE_EQUAL(mode1,mode2)		(((mode1).xdim == (mode2).xdim) && \
									 ((mode1).ydim == (mode2).ydim) && \
									 ((mode1).bpp == (mode2).bpp))

#define MAX_SVGA_DEVICES				50
#define MAX_SVGA_RESOLUTIONS			50

#define SVGAOK							 1
#define SVGAERROR_UNKNOWN				-1
#define SVGAERROR_UNKNOWN_CHIP			-2
#define SVGAERROR_MODE_NOT_SUPPORTED    -3

typedef struct
{
	int color_depth;

	void (*draw_pixel)(int x, int y, int color);
	int  (*get_pixel)(int x, int y);
	void (*draw_hline)(int x1, int x2, int y, int color);
	void (*draw_vline)(int x, int y1, int y2, int color);
	void (*draw_fill_rect)(int x1, int y1, int x2, int y2, int color);

	void (*copy_screen_rect)(int x1, int y1, int x2, int y2, int dx, int dy);
} display_draw_table;

typedef struct
{
    char name[256];							/* Display driver name */
	char desc[256];							/* Description or version */
    int  (*detect)(void);					/* Detect display card */
    int  (*init)(void);						/* Init driver */
    void (*set_draw_table)(display_draw_table *dt);			/* Overwrite draw table */
    void (*setpage_read)(int page);			/* Set page for reading */
    void (*setpage_write)(int page);		/* Set page for writing/(reading) */
    int  (*setmode)(mode_info_t *mode);		/* Set video mode */
    int  (*modeavailable)(int mode);		/* Check for video mode */

	mode_info_t *mode_table;

	int bank_size;							/* Size of one bank */
	int bank_mode;							/* 1..Single bank mode
										     * 2..Double bank mode */
	int use_lfb;							/* Use Linear Frame Buffer? */
	unsigned int linear_base;				/* Physical base address of Frame Buffer */

} display_device_driver;


extern display_device_driver *active_display_driver;


/* Display Drivers */
#define DISP_VGA16			0x01
#define DISP_VESA2			0x02


/* VGA index register ports */
#define CRT_IC  0x3D4		/* CRT Controller Index - color emulation */
#define CRT_IM  0x3B4		/* CRT Controller Index - mono emulation */
#define ATT_IW  0x3C0		/* Attribute Controller Index & Data Write Register */
#define GRA_I   0x3CE		/* Graphics Controller Index */
#define SEQ_I   0x3C4		/* Sequencer Index */
#define PEL_IW  0x3C8		/* PEL Write Index */
#define PEL_IR  0x3C7		/* PEL Read Index */

/* VGA data register ports */
#define CRT_DC  0x3D5		/* CRT Controller Data Register - color emulation */
#define CRT_DM  0x3B5		/* CRT Controller Data Register - mono emulation */
#define ATT_R   0x3C1		/* Attribute Controller Data Read Register */
#define GRA_D   0x3CF		/* Graphics Controller Data Register */
#define SEQ_D   0x3C5		/* Sequencer Data Register */
#define MIS_R   0x3CC		/* Misc Output Read Register */
#define MIS_W   0x3C2		/* Misc Output Write Register */
#define IS1_RC  0x3DA		/* Input Status Register 1 - color emulation */
#define IS1_RM  0x3BA		/* Input Status Register 1 - mono emulation */
#define PEL_D   0x3C9		/* PEL Data Register */
#define PEL_MSK 0x3C6		/* PEL mask register */

/* standard VGA indexes max counts */
#define CRT_C   24		/* 24 CRT Controller Registers */
#define ATT_C   21		/* 21 Attribute Controller Registers */
#define GRA_C   9		/* 9  Graphics Controller Registers */
#define SEQ_C   5		/* 5  Sequencer Registers */
#define MIS_C   1		/* 1  Misc Output Register */

/* VGA registers saving indexes */
#define CRT     0		/* CRT Controller Registers start */
#define ATT     (CRT+CRT_C)	/* Attribute Controller Registers start */
#define GRA     (ATT+ATT_C)	/* Graphics Controller Registers start */
#define SEQ     (GRA+GRA_C)	/* Sequencer Registers */
#define MIS     (SEQ+SEQ_C)	/* General Registers */
#define EXT     (MIS+MIS_C)	/* SVGA Extended Registers */








