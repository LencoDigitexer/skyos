/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996-1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\memdump.c
/* Last Update: 22.12.1998
/* Version    : beta
/* Coded by   : Hayrapetian Gregory
/* Docus from : PC Intern 5, Moderne Betriebssysteme, Internet
/************************************************************************/

#include "system.h"

#define KERNEL_DMS_START 0x20000  /* KERNEL_DYNAMIC_MEMORY_SPACE
                                     should be far behind the kernel */

#define PAGE_DIR 0x1000

#define KERNEL   -1
#define RESERVED -9
#define MAX_MEM   20

struct sysmem_entry             /* System Memory Table entry */
{
  int start;
  int end;
  int owner;
};

struct pt_entry                 /* Page Table entry */
{
   int entry;
};

struct kernelmem_entry          /* Entry for one allocated memory block of */
{                               /* the kernel */
  int start;
  int end;
  struct kernelmem_entry *next;
  struct kernelmem_entry *last;
};

struct sysmem_entry sysmem_table[MAX_MEM] = {0};

struct kernelmem_entry *km = NULL;
int memstart;


int sysmem_newentry(int start, int end, int owner)
{
  int i=0;

  while ((i<MAX_MEM) && (sysmem_table[i].owner != 0))
    i++;

  if (i==MAX_MEM)
  {
    printk("\nmem.c: No system memory slots available.\n");
    return -1;
  }

  sysmem_table[i].owner = owner;
  sysmem_table[i].start = start;
  sysmem_table[i].end   = end;
  return 0;
}


unsigned int kalloc(unsigned int size)
{
  unsigned int base;

  base = memstart;
  memstart += size;

  return base;
}

void kfree(unsigned int pointer)
{
}

void mem_update_startaddr(int startaddr)
{
  memstart = startaddr;
}

void dump_kernel_mem(void)
{
   int i;
   struct kernelmem_entry *entry;

   entry = km;

   printk("Dumping kernel memory: \n\n");
   printk("--------------------------------------------------------------------------\n");
   printk("Start      | End         \n");
   printk("--------------------------------------------------------------------------\n");
   if (entry == NULL) return;
   do
   {
      printk("0x%00000008x | 0x%00000008x\n",
      entry->start, entry->end);
   } while(entry->next != NULL);
   printk("\n");
}


void dump_system_mem_table(void)
{
   int i;

   printk("Dumping system memory slots: \n\n");
   printk("--------------------------------------------------------------------------\n");
   printk("Start      | End         | Owner                                        \n");
   printk("--------------------------------------------------------------------------\n");
   for (i = 0; i< MAX_MEM; i++)
   {
     printk("0x%00000008x | 0x%00000008x | 0x%00000008x\n",
    sysmem_table[i].start,
    sysmem_table[i].end,
    sysmem_table[i].owner );
   }
   printk("\n");
}

/************************************************************************/
/* Page Table Functions
/************************************************************************/
unsigned int getpt_entry(unsigned int base, int entry)
{
  unsigned int *pointer;
  unsigned int value;

  if (base == 0)
  {
     pointer = (unsigned int*) PAGE_DIR + entry;
     return (*pointer);
  }
  else
  {
     base = base >> 11;
     base = base << 11;

     pointer = (unsigned int*) base + entry;
     return (*pointer);
  }
}

void setpt_entry(unsigned int base, int entry, unsigned int v)
{
  unsigned int *pointer;
  unsigned int value;

  if (base == 0)
  {
     pointer = (unsigned int*) PAGE_DIR + entry;
     *pointer = v;
  }
  else
  {
     base = base >> 11;
     base = base << 11;

     pointer = (unsigned int*) base + entry;
     *pointer = v;
  }
}


void dump_page_directory(void)
{
   int i,j;
   char ch=0;

   printk("Dumping page directory at 0x1000: \n\n");
   printk("--------------------------------------------------------------------------\n");
   printk("Table | Adress \n");
   printk("--------------------------------------------------------------------------\n");
   for (i = 0; i< 7; i++)
   {
     printk("%00005d  | 0x%00000008x\n",i, getpt_entry(0,i));
   }

   printk("Dumping page table at 0x2000: \n\n");
   printk("--------------------------------------------------------------------------\n");
   printk("Table | Adress \n");
   printk("--------------------------------------------------------------------------\n");

   i=0;

   while (ch!=27)
   {
     for (j=0;j<=15;j++)
       printk("%00005d  | 0x%00000008x\n",i+j, getpt_entry(1,i+j));

     i = j;

     ch = getch();
   }
   printk("\n");
}

void dump_pagetable(void)
{
   int i,j,k;
   struct twm_window w={0};
   char ch=0;
   char str[255];

   w.x = 500;
   w.length = 250;
   w.y = 140;
   w.heigth = 440;
   strcpy(w.title,"Page Tables");
   w.actx = 5;
   w.acty = 30;

   draw_window(&w);

   i = 0;
   k = 0;
   while (ch != 27)
   {
      clear_window(&w);
      out_window(&w,"Page Typ: Directory");

      out_window(&w,"Index   Address");
      for (i=0;i<=35;i++)
      {
         sprintf(str,"%0004d:    %00000008x",k,getpt_entry(0,k));
         out_window(&w, str);
         k++;
      }
      ch = getch();
      if (ch=='1')
        k = 100;
      if (ch=='2')
        k = 200;
      if (ch=='3')
        k = 300;
      if (ch=='4')
        k = 400;
      if (ch=='5')
        k = 500;
      if (ch=='6')
        k = 600;
      if (ch=='7')
        k = 700;
      if (ch=='8')
        k = 800;
      if (ch=='9')
        k = 900;
   }
}

