/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\mm\mmap.c
/* Last Update: 21.12.1998
/* Version    : beta
/* Coded by   : Hayrapetian Gregory
/* Docus from : PC Intern 5, Moderne Betriebssysteme, Internet
/************************************************************************/
/* Definition:
/*   This file implements the memory manager.
/************************************************************************/

#include "memory.h"

int set_memmap(unsigned int block, char bit)
{
 if (get_memmap(block) != bit)
     memmap[block >> 3] ^= (1 << (block % 8));
 return 1;
}

int get_memmap(unsigned int block)
{
 return (memmap[block>>3] >> (block % 8)) & 1;
}

unsigned int kvalloc(unsigned int size)
{
 /* base adr: map_adr */
 unsigned int block = map_count;

 while ((get_memmap(block) != 0) && (block < block_num)) block++;
 if (block == block_num)
 {
   struct mem_list *first = free_list->prev;

   if (block_num >= MEMMAP_MAX_SIZE)
   {
     printk("mem.c: kvalloc - Bitmap full  Counter: %d",map_count);
     return -1;
   }

   block_num += MEMMAP_ENLARGE;
   first->end -= MEMMAP_ENLARGE * BLOCK_SIZE;

   if (first->end <= first->start)
   {
     printk("mem.c: kvalloc - Out of memory");
     return -2;
   }
 }

 set_memmap(block,1);
 map_count = block+1;
 if (map_count >= block_num)
   map_count = 0;

 return map_adr - (block * BLOCK_SIZE);   
}

int kvfree(unsigned int ptr)
{
 unsigned int block = (map_adr - ptr) / BLOCK_SIZE;

 if (block > block_num)
   printk("mem.c: kvfree - Wrong address to free");

 set_memmap(block,0);
}
