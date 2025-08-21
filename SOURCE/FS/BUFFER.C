/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : fs\buffer.c
/* Last Update: 04.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Software cache functions. This file allows to cache blocks in a
/*   dynamic cache buffer. Using LRU.
/************************************************************************/

#include "buffer.h"

unsigned int cache_active = 0;


struct s_cache *cache;

int cache_init(void)
{
   int i;

   cache = (struct s_cache*)valloc(sizeof(struct s_cache));
   if (!cache)
   {
     printk("buffer.c: Unable to allocate memory for cache...");
     return -1;
   }

   for (i=0;i<MAX_CACHE_BLOCKS;i++)
      cache->block[i].sector = cache->block[i].device = cache->block[i].count = 0;

   printk("buffer.c: Initializing cache... Base: 0x%00000008x End: 0x%00000008x",cache, cache+512*MAX_CACHE_BLOCKS);
   printk("          Size is %dkb",MAX_CACHE_BLOCKS/2);
   cache_active = 1;

   return 0;
}

unsigned int cache_hit(int device, int nr, unsigned char *buffer)
{
  int i = 0;

  for (i=0;i<MAX_CACHE_BLOCKS;i++)
  {
    if ((cache->block[i].device == device) && (cache->block[i].sector == nr))
    {
      memcpy(buffer, cache->block[i].cache, 512);
      cache->block[i].count = 0;
//      printk("Found in cache: %d",nr);
//      memory_dump(buffer,100);
      return 1;
    }
    else
      cache->block[i].count++;
  }
  return 0;
}

unsigned int cache_add(int device, int nr, unsigned char *buffer)
{
  int i;
  int used;
  int index;

  // save in cache structure
  for (i=0;i<MAX_CACHE_BLOCKS;i++)
  {
    if ((cache->block[i].device == device) && (cache->block[i].sector == nr))
    {
      memcpy(cache->block[i].cache, buffer, 512);
      cache->block[i].device = device;
      cache->block[i].sector = nr;
      cache->block[i].count = 0;
//      printk("Replaced: %d",nr);
      return;
    }
  }

  // look for free storage

  for (i=0;i<MAX_CACHE_BLOCKS;i++)
  {
    if (cache->block[i].sector == 0)
    {
       memcpy(cache->block[i].cache, buffer, 512);
       cache->block[i].device = device;
       cache->block[i].sector = nr;
       cache->block[i].count = 0;
//       printk("New allocated at %d, BlockNr: %d",i,nr);
       return 0;
    }
  }

  // else replace the block, which is used less.
  used = cache->block[0].count;
  index = 0;

  for (i=1;i<MAX_CACHE_BLOCKS;i++)
  {
    if (cache->block[i].count > used)
    {
      used= cache->block[i].count;
      index = i;
    }
  }
  memcpy(cache->block[index].cache, buffer, 512);
  cache->block[index].device = device;
  cache->block[index].sector = nr;
  cache->block[index].count = 0;

//  printk("Replaced used less. Nr: %d",index);

  return 0;
}

unsigned int cache_reinit(int device)
{
   int i;

   for (i=0;i<MAX_CACHE_BLOCKS;i++)
   {
      if (cache->block[i].device == device)
        cache->block[i].sector = cache->block[i].device = cache->block[i].count = 0;
   }

   return 0;
}

