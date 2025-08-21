/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996-1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\memory.h
/* Last Update: 21.12.1998
/* Version    : beta
/* Coded by   : Hayrapetian Gregory
/* Docus from : PC Intern 5, Moderne Betriebssysteme, Internet
/************************************************************************/
/* Definition:
/*   This file implements the memory manager.
/************************************************************************/

#define NULL (void*)0
#define PAGE_DIR 0x1000

#define MEMMAP_MAX_SIZE		32768			/* max number of blocks */
#define BLOCK_SIZE			20
#define MEMMAP_ENLARGE		4096			/* number of blocks to enlarge space for kvalloc() */

struct mem_list
{
  int pid;
  unsigned int start, end;
  struct mem_list *next;
  struct mem_list *prev;
};

struct mem_list *res_list, *free_list;
char memmap[MEMMAP_MAX_SIZE >> 3];		/* for kvalloc(); max 32768 blocks with x bytes */
volatile int map_count;					/* using for NEXT FIT method */

unsigned int *page_table, *page_dir, max_mem;
unsigned int block_num, map_adr;

