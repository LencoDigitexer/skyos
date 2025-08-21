/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 2000 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\color.c
/* Last Update: 02.04.1999
/* Version    : alpha
/* Coded by   :
/* Docus      : Allegro color.c
/************************************************************************/
/* Definition:
/*   Color functions
/************************************************************************/
/* Updated:
/************************************************************************/
#include "color.h"
#include <limits.h>

/* 1.5k lookup table for color matching */
static unsigned int col_diff[3*128] ={0};


/* bestfit_init:
 *  Color matching is done with weighted squares, which are much faster
 *  if we pregenerate a little lookup table...
 */
static void bestfit_init()
{
   int i;

   for (i=1; i<64; i++) {
      int k = i * i;
      col_diff[0  +i] = col_diff[0  +128-i] = k * (59 * 59);
      col_diff[128+i] = col_diff[128+128-i] = k * (30 * 30);
      col_diff[256+i] = col_diff[256+128-i] = k * (11 * 11);
   }
}

/* bestfit_color:
 *  Searches a palette for the color closest to the requested R, G, B value.
 */
int bestfit_color(PALETTE pal, int r, int g, int b)
{
   int i, coldiff, lowest, bestfit;

   if (col_diff[1] == 0)
      bestfit_init();

   bestfit = 0;
   lowest = INT_MAX;

   if ((r == 63) && (g == 0) && (b == 63))
      i = 0;
   else
      i = 1;

   while (i<PAL_SIZE)
   {
      RGB *rgb = &pal[i];
      coldiff = (col_diff + 0) [ (rgb->g - g) & 0x7F ];
      if (coldiff < lowest)
      {
	 coldiff += (col_diff + 128) [ (rgb->r - r) & 0x7F ];
	 if (coldiff < lowest) {
	    coldiff += (col_diff + 256) [ (rgb->b - b) & 0x7F ];
	    if (coldiff < lowest) {
	       bestfit = rgb - pal;    /* faster than `bestfit = i;' */
	       if (coldiff == 0)
		  return bestfit;
	       lowest = coldiff;
	    }
	 }
      }
      i++;
   }

   return bestfit;
}

void color_init(void)
{
  get_palette(0,255,_current_palette);
  bestfit_init();
}

