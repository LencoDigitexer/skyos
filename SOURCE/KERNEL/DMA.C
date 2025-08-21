/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\dma.c
/* Last Update: 04.11.1998
/* Version    : release
/* Coded by   : Szeleney Robert
/* Docus      : Linux, PC Intern 5
/************************************************************************/
/* Definition:
/*   Implements functions to communicate with the DMA Chip.
/************************************************************************/
#define IO_DMA1_BASE    0x00    /* 8 bit slave DMA, channels 0..3 */
#define IO_DMA2_BASE    0xC0    /* 16 bit master DMA, ch 4(=slave input)..7 */

#define DMA_ADDR_0              0x00    /* DMA address registers */
#define DMA_ADDR_1              0x02
#define DMA_ADDR_2              0x04
#define DMA_ADDR_3              0x06
#define DMA_ADDR_4              0xC0
#define DMA_ADDR_5              0xC4
#define DMA_ADDR_6              0xC8
#define DMA_ADDR_7              0xCC

#define DMA_CNT_0               0x01    /* DMA count registers */
#define DMA_CNT_1               0x03
#define DMA_CNT_2               0x05
#define DMA_CNT_3               0x07
#define DMA_CNT_4               0xC2
#define DMA_CNT_5               0xC6
#define DMA_CNT_6               0xCA
#define DMA_CNT_7               0xCE

#define DMA_PAGE_0              0x87    /* DMA page registers */
#define DMA_PAGE_1              0x83
#define DMA_PAGE_2              0x81
#define DMA_PAGE_3              0x82
#define DMA_PAGE_5              0x8B
#define DMA_PAGE_6              0x89
#define DMA_PAGE_7              0x8A

#define DMA_MODE_READ   0x44    /* I/O to memory, no autoinit, increment, single mode */
#define DMA_MODE_WRITE  0x48    /* memory to I/O, no autoinit, increment, single mode */
#define DMA_MODE_CASCADE 0xC0   /* pass thru DREQ->HRQ, DACK<-HLDA only */


void disable_dma(int nummer)
{
  if (nummer < 4)
    outportb(0x0A, nummer + 4);
  else
    outportb(0x0D4, (nummer&3) + 4);
}

void enable_dma(int nummer)
{
  if (nummer < 4)
    outportb(0x0A, nummer);
  else
    outportb(0x0D4, nummer&3);
}

void clear_dma_ff(int nummer)
{
   if (nummer < 4)
     outportb(0xC, 0);
   else
     outportb(0xD8, 0);
}

void set_dma_mode(int nummer, int mode)
{
  if (nummer < 4)
    outportb(0xB, nummer | mode);
  else
    outportb(0xD6, (nummer&3) | mode);
}

void set_dma_page(unsigned int dmanr, char pagenr)
{
   switch(dmanr) {
      case 0:
          outportb(DMA_PAGE_0, pagenr);
          break;
      case 1:
          outportb(DMA_PAGE_1, pagenr );
          break;
      case 2:
          outportb(DMA_PAGE_2, pagenr );
          break;
      case 3:
          outportb(DMA_PAGE_3, pagenr );
          break;
      case 5:
          outportb(DMA_PAGE_5, pagenr & 0xfe );
          break;
      case 6:
          outportb(DMA_PAGE_6, pagenr & 0xfe );
          break;
      case 7:
          outportb(DMA_PAGE_7, pagenr & 0xfe );
          break;
      }
}


void set_dma_addr(unsigned int dmanr, unsigned int a)
{
 set_dma_page(dmanr, a>>16);
 if (dmanr <= 3)
 {
   outportb(((dmanr&3)<<1) + IO_DMA1_BASE , a & 0xff);
   outportb(((dmanr&3)<<1) + IO_DMA1_BASE , (a>>8) & 0xff);
 }
 else
 {
   outportb(((dmanr&3)<<2) + IO_DMA2_BASE, (a>>1) & 0xff );
   outportb(((dmanr&3)<<2) + IO_DMA2_BASE, (a>>9) & 0xff );
 }
}

void set_dma_count(unsigned int dmanr, unsigned int count)
{
    count--;
    if (dmanr <= 3)
    {
       outportb(((dmanr&3)<<1) + 1 + IO_DMA1_BASE, count & 0xff);
       outportb(((dmanr&3)<<1) + 1 + IO_DMA1_BASE, (count>>8) & 0xff );
    }
    else
    {
       outportb(((dmanr&3)<<2) + 2 + IO_DMA2_BASE, (count>>1) & 0xff);
       outportb(((dmanr&3)<<2) + 2 + IO_DMA2_BASE, (count>>9) & 0xff);
    }
}

