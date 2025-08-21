/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\pnp.c
/* Last Update: 21.07.1999
/* Version    :
/* Coded by   : Szeleney Robert
/************************************************************************/
/* Definition:
/*   Plug'n'Play Support
/************************************************************************/
#include "sched.h"
#include "head.h"

#define PACKED __attribute__ ((packed))

struct PNP_Expansion_Header
{
   unsigned char sign[4]               PACKED;
   unsigned char revision              PACKED;
   unsigned char length                PACKED;
   unsigned short next_header          PACKED;
   unsigned char reserved              PACKED;
   unsigned char checksum              PACKED;
   unsigned int  device_ident          PACKED;
   unsigned short manufacturer         PACKED;
   unsigned short product              PACKED;
   unsigned char  device_type[3]       PACKED;
   unsigned char device_indicator      PACKED;
   unsigned short vector_boot_connection PACKED;
   unsigned short vector_disconnect    PACKED;
   unsigned short bootstrap_entry      PACKED;
   unsigned short reserved2            PACKED;
   unsigned short vector_static_resource PACKED;
};

struct PNP_Option_ROM
{
        unsigned short  magic              PACKED;                 /* 0xAA55 */
        unsigned char   len                PACKED;                 /* length 512byte blocks */
        unsigned long   entry              PACKED;                        /* entry point */
        unsigned char   reserved[19]       PACKED;
        unsigned short  exp_offs           PACKED;             /* expansion header offs */
};

struct PNP_BIOS
{
	unsigned char	magic[4]                			PACKED;
	unsigned char	version						PACKED;		/* BCD number */
	unsigned char	length						PACKED;		/* length in BYTES */
	unsigned short	control_field					PACKED;
	unsigned char	crc	                                        PACKED;
	unsigned long	event_notification_flag_addr	                PACKED;
	unsigned short	rmode_entry_ip					PACKED;
	unsigned short	rmode_entry_cs					PACKED;
	unsigned short	pmode_entry_ip					PACKED;
	unsigned long	pmode_entry_cs					PACKED;
	unsigned long	OEM_device_id					PACKED;
	unsigned short	rmode_data_seg					PACKED;
	unsigned long	pmode_data_seg					PACKED;
};


void search_pnp_bios(void)
{
  struct PNP_BIOS *p_bios;
  unsigned int addr;
  unsigned int data;
  unsigned int *pdata;


  for (addr = 0xF0000; addr<0xFFFFF ; addr+=16)
  {
    p_bios = (struct PNP_BIOS*) addr;

    if (!strncmp(p_bios->magic, "$PnP",4))
    {
      printk("pnp.c: PNP Bios found at 0x%00005X",addr);
      printk("pnp.c: PNP Version %d.%d",(p_bios->version >> 4) & 0xF, p_bios->version & 0xF);
      printk("pnp.c: PNP BIOS PM-Segment: 0x%x",p_bios->pmode_entry_cs);
      printk("pnp.c: PNP BIOS PM-Offset : 0x%x",p_bios->pmode_entry_ip);
      printk("pnp.c: PNP BIOS PM-Dataseg: 0x%x",p_bios->pmode_data_seg);

      SetSegmentDesc(&gdt[FIRST_TSS_ENTRY+1], p_bios->pmode_entry_cs, 0xFFFF,
                     0x9A, 0x00);

      printk("pnp.c: GDT Table at: 0x%x",&gdt);
      printk("pnp.c: Selector    : %d",FIRST_TSS_ENTRY+1);

    }
  }
}

void pnp_init(void)
{
  unsigned char* addr;
  struct PNP_Option_ROM *p_opt;
  struct PNP_Expansion_Header *p_exp;

  search_pnp_bios();

  for ( addr = 0xC0000 ; addr < 0xF0000 ; addr += 2048 )
  {
    p_opt = (struct PNP_Option_ROM*)addr;

    if (p_opt->magic == 0xAA55)
    {
       printk("pnp.c: PNP Option ROM Header at 0x%x",addr);
       printk("pnp.c: Offset to exp. header is 0x%x",p_opt->exp_offs);
       printk("pnp.c: Address of exp. header is 0x%x",addr + p_opt->exp_offs);
    }
  }


/*  printk("pnp.c: Searching for Plug and Play BIOS...");

  for (addr = 0xF0000; addr <0xFFFFF; addr +=16)
  {
    if (!strncmp(addr, "$PnP",4))
    {
      printk("pnp.c: PnP BIOS found at 0x%00000X",addr);
      header = addr;
    }
  }

  if (addr == 0xFFFFF)
  {
    printk("pnp.c: No PnP BIOS found.");
    return;
  }

  printk("pnp.c: Signature: %c%c%c%c",header->sign[0],header->sign[1],
         header->sign[2],header->sign[3]);

  if (header->manufacturer != 0)
    printk("pnp.c: %s",header->manufacturer);
  else
    printk("pnp.c: No manufacturer information");

  if (header->product != 0)
    printk("pnp.c: %s",header->product);
  else
    printk("pnp.c: No productname information");

  a = header + 0x0E;
  b = *a;

  a = b;

  printk("pnp.c: %s",a);

  */
}



