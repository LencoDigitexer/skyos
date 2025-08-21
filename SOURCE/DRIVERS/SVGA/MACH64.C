/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\svga\ati.c
/* Last Update: 03.12.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Allegro
/************************************************************************/
/* Definition:
/*   A ATI MACH64 SVGA videocard device driver.
/************************************************************************/
int _mach64_wp_sel;              /* mach64 write bank address */
int _mach64_rp_sel;              /* mach64 read bank address */
int _mach64_off_pitch;           /* mach64 display start address */



/* get_mach64_port:
 *  Calculates the port address for accessing a specific mach64 register.
 */

static int get_mach64_port(int io_type, int io_base, int io_sel, int mm_sel)
{
   if (io_type)
   {
      return (mm_sel << 2) + io_base;
   }
   else
   {
      if (!io_base)
	 io_base = 0x2EC;

      return (io_sel << 10) + io_base;
   }
}


/* mach64_detect:
 *  Detects the presence of a mach64 card.
 */
int mach64_detect(unsigned char ah,unsigned short cx,unsigned short dx )
{
   int scratch_reg;
   unsigned int old;

   if (ah)
      return 0;

   /* test scratch register to confirm we have a mach64 */
   scratch_reg = get_mach64_port(cx, dx, 0x11, 0x21);

   old = inportl(scratch_reg);

   outportl(scratch_reg, 0x55555555);
   if (inportl(scratch_reg) != 0x55555555)
   {
      outportl(scratch_reg, old);
      return 0;
   }

   outportl(scratch_reg, 0xAAAAAAAA);
   if (inportl(scratch_reg) != 0xAAAAAAAA)
   {
      outportl(scratch_reg, old);
      return 0;
   }

   outportl(scratch_reg, old);

   /* calculate some port addresses... */
   _mach64_wp_sel = get_mach64_port(cx, dx, 0x15, 0x2D);
   _mach64_rp_sel = get_mach64_port(cx, dx, 0x16, 0x2E);
   _mach64_off_pitch = get_mach64_port(cx, dx, 5, 5);

   return 1;
}




