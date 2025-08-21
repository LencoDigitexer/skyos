/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : fs\binary\binary.c
/* Last Update: 25.01.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus from :
/************************************************************************/
/* Definition:
/*  Supports the binary file format.
/*  CS = DS, Offset = 0;
/************************************************************************/
#include "fcntl.h"
#include "a_out.h"

#define SCAN_CHUNK 256

int binary_aout_load(unsigned int addr, unsigned char *name)
{
  struct exec x;
  int hfile;
  int err;
  unsigned int actual;
  unsigned int text_start;	/* text start in memory */
  unsigned int text_size;
  unsigned int text_offset;	/* text offset in file */
  unsigned int data_size;
  unsigned char* user_space;

  user_space = (unsigned char*)addr;

  hfile = sys_open(name, O_RDONLY);
  if (hfile < 0)
  {
     show_msgf("aout.c: unable to open file...");
     return -1;
  }

  err = sys_read(hfile, &x, sizeof(x));

  if (err < 0)
  {
     show_msgf("aout.c: error while reading header...");
     sys_close(hfile);
     return -1;
  }

  show_msgf("aout.c: magic %04o", (short)x.a_magic);

  switch ((short)x.a_magic & 0xFFFF)
  {
     case OMAGIC:
	text_start  = 0;
	text_size   = 0;
	text_offset = sizeof(struct exec);
	data_size   = x.a_text + x.a_data;
	break;

    case NMAGIC:
	text_start  = 0;
	text_size   = x.a_text;
	text_offset = sizeof(struct exec);
	data_size   = x.a_data;
	break;

    case ZMAGIC:
    {
    	char buf[SCAN_CHUNK];

        show_msg("aout.c: ZMAGIC found.");
		/* This kludge is not for the faint-of-heart...
		   Basically we're trying to sniff out the beginning of the text segment.
		   We assume that the first nonzero byte is the first byte of code,
		   and that x.a_entry is the virtual address of that first byte.  */
	for (text_offset = 0; ; text_offset++)
	{
		if ((text_offset & (SCAN_CHUNK-1)) == 0)
		{
                        sys_seek(hfile, text_offset,0);
                        err = sys_read(hfile, buf, SCAN_CHUNK);
			if (err<0)
                        {
                             show_msgf("aout.c: error while reading code...");
                             sys_close(hfile);
                             return -1;
                        }
			if (err < SCAN_CHUNK)
				buf[err] = 0xff; /* ensure termination */

			if (text_offset == 0)
				text_offset = sizeof(struct exec);
		}
		if (buf[text_offset & (SCAN_CHUNK-1)])
			break;
	}
	/* Account for the (unlikely) event that the first instruction
	   is actually an add instruction with a zero opcode.
	   Surely every a.out variant should be sensible enough at least
		   to align the text segment on a 32-byte boundary...  */
	text_offset &= ~0x1f;

	text_start = x.a_entry;
	text_size = x.a_text;
	data_size   = x.a_data;
	break;
    }

    case QMAGIC:
	text_start	= 0x1000;
	text_offset	= 0;
	text_size	= x.a_text;
	data_size	= x.a_data;
	break;

    default:
	/* Check for NetBSD big-endian ZMAGIC executable */
	if ((int)x.a_magic == 0x0b018600) {
		text_start  = 0x1000;
		text_size   = x.a_text;
		text_offset = 0;
		data_size   = x.a_data;
		break;
	}
	return (-1);
     }

     /* If the text segment overlaps the same page as the beginning of the data segment,
     then cut the text segment short and grow the data segment appropriately.  */
/*     if ((text_start + text_size) & 0xfff)
     {
	unsigned int incr = (text_start + text_size) & 0xfff;
 	if (incr > text_size) incr = text_size;
	text_size -= incr;
	data_size += incr;
     }*/

     show_msgf("aout.c: text_start %08x text_offset %08x",text_start, text_offset);
     show_msgf("text_size %08x data_size %08x",text_size, data_size);

     /* Load the read-only text segment, if any.  */
     if (text_size > 0)
     {
        sys_seek(hfile, text_offset, 0);
        err = sys_read(hfile, user_space + text_start, text_size);
	if (err < 0)
        {
            show_msgf("aout.c: error while reading code (2)...");
            sys_close(hfile);
            return -1;
        }
     }
	/* Load the read-write data segment, if any.  */
	if (data_size + x.a_bss > 0)
	{
            sys_seek(hfile, text_offset + text_size,0);
            err = sys_read(hfile, user_space + text_size + text_start, data_size + x.a_bss);
  	    if (err < 0)
            {
              show_msgf("aout.c: error while reading data ...");
              sys_close(hfile);
              return -1;
            }
	}

	/*
	 * Load the symbol table, if any.
	 * First the symtab, then the strtab.
	 */
	if (x.a_syms > 0)
	{
		unsigned strtabsize;

		/*
		 * Figure out size of strtab.
		 * The size is the first word and includes itself.
		 * If there is no strtab, this file is bogus.
		 */
                sys_seek(hfile,text_offset + text_size + data_size + x.a_syms,0);
                err = sys_read(hfile, &strtabsize, sizeof(strtabsize));
		if (err < 0)
                {
                  show_msgf("aout.c: error while reading strtab ...");
                  sys_close(hfile);
                  return -1;
                }

	}

  sys_close(hfile);
  return x.a_entry;
}

