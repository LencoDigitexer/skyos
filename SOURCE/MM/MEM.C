/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996-1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\mem.c
/* Last Update: 21.12.1998
/* Version    : beta
/* Coded by   : Hayrapetian Gregory
/* Docus from : PC Intern 5, Moderne Betriebssysteme, Internet
/************************************************************************/
/* Definition:
/*   This file implements the memory manager.
/************************************************************************/
/* Main functions:                                                      */
/*           - valloc: Allocates a size of memory                       */
/*           - vfree: Frees an allocation                               */
/*           - get_freemem: Returns the number of memory available      */
/*           - memstat: Displays all allocations and free spaces        */
/*           - memlist: Shows how much memory a task has reserved       */
/*                                                                      */
/* All other functions are used by the memory manager, and should not   */
/* be called by an other task.                                          */
/************************************************************************/
#include "sched.h"
#include "memory.h"
#include "critical.h"

extern int newgui_pid;
static int msg_nest=1;

extern struct task_struct *current;

int memstart;
critical_section_t csect_mm;

/*
 * Insert a new mem_list element
 */
struct mem_list *ins_list(unsigned int start, unsigned int end,struct mem_list *list)
{
 struct mem_list *l;

 l = (struct mem_list *) kvalloc(sizeof(struct mem_list));
 l->next  = l->prev  = NULL;
 l->start = start;
 l->end   = end;
 l->pid   = (current->pid < 0) ? 0 : current->pid;

 if (list == NULL)
 {
  l->prev = l;
  list = l;
 }
 else
 {
  l->prev = list->prev;
  list->prev->next = l;
  list->prev = l;
 }

 return list;
}

/*
 * Delete a mem_list element
 */
struct mem_list *del_list(struct mem_list *l,struct mem_list *list)
{

 if (l->prev->next)
 {
  l->prev->next = l->next;

  if (l->next) l->next->prev = l->prev;
  else list->prev = l->prev;
 }
 else
 {
  if (l->next)
  {
   l->next->prev = l->prev;
   list = l->next;
  }
  else list = NULL;
 }

 kvfree(l);
 return list;
}

/*
 * Insert a new mem_list element in free_list, sort by address
 * Merge rooms
 */
void ins_list_sort(unsigned int start, unsigned int end)
{
 struct mem_list *l, *f;
 char merge = 0;

 f = free_list;

 if (f == NULL)
 {
   free_list = ins_list(start,end,free_list);
   return;
 }

 if (start >= free_list->start)
 {
   while (start >= f->start) f = f->next;

   if (end + 1 == f->start)
   {
    f->start = start;
    end      = f->end;
    merge    = 1;
   }

   f = f->prev;
   if (f->end + 1 == start)
   {
    f->end = end;
    f = f->next;
    if (merge) free_list = del_list(f, free_list);
    else merge = 1;
   }

   if (!merge)
   {
    l = (struct mem_list *) kvalloc(sizeof(struct mem_list));
    l->next = l->prev = NULL;
    l->start = start;
    l->end   = end;

    l->next = f->next;
    f->next->prev = l;
    l->prev = f;
    f->next = l;
    if (!l->next) l->next->prev = l;
   }
 }
 else
 {
  if (end + 1 == free_list->start) free_list->start = start;
  else
  {
   l = (struct mem_list *) kvalloc(sizeof(struct mem_list));
   l->next = l->prev = NULL;
   l->start = start;
   l->end   = end;

   l->prev = free_list->prev;
   free_list->prev = l;
   l->next = free_list;
   free_list = l;
  }
 }

}

/*
 * Allocates a size of memory
 * Uses LAST FIT method
 */
unsigned int valloc(unsigned int size)
{
 struct mem_list *f = free_list->prev;
 unsigned int ptr;
 int flags;

 if (size < 1) return 0;

 do
 {
  if (size <= (f->end - f->start + 1))
  {
   save_flags(flags);
   cli();
//   enter_critical_section(&csect_mm);

   res_list = ins_list(f->start,f->start + size - 1,res_list);
   ptr = f->start;
   f->start += size;
   if (f->start > f->end) free_list = del_list(f,free_list);

   restore_flags(flags);
//   leave_critical_section(&csect_mm);

   return ptr;
  }
  f = f->prev;
 } while (f != free_list);

 printk("mem.c: valloc() - %s is out of memory",current->name);
 return 0;
}

unsigned int getmem_pid(unsigned int pid)
{
 struct mem_list *r = res_list;
 int i=0;

 while (r)
 {
  if (r->pid == pid)
    i += (r->end - r->start);
  r = r->next;
 }

 return i;
}


/*
 * Frees an allocation
 * Look from last to first memory entry
 */
int vfree(unsigned int ptr)
{
 struct mem_list *r;
 int i, j;
 int flags;

 r = res_list->prev;

 save_flags(flags);
 cli();
// enter_critical_section(&csect_mm);

 while ((r != res_list) && (r->start != ptr)) r = r->prev;

 if (r->start != ptr)
 {
  printk("mem.c: vfree() - Wrong address to free: %d  task: %s",ptr,current->name);
  restore_flags(flags);
//  leave_critical_section(&csect_mm);

  return -1;
 }

 ins_list_sort(r->start, r->end);
 res_list = del_list(r, res_list);
 if (map_count > 0)
   map_count--;
 restore_flags(flags);
// leave_critical_section(&csect_mm);

 return 0;
}

unsigned int get_freemem()
{
 struct mem_list *f = free_list;
 unsigned int mem=0;

 while (f)
 {
   mem += f->end - f->start + 1;
   f = f->next;
 }

 return mem;
}

void memstat()
{
 struct mem_list *r = res_list;
 struct mem_list *f = free_list;
 int rc=0,fc=0;
 char str[256];

 show_msg("");
 show_msg("res_list:");
 while (r)
 {
  rc++;
  sprintf(str,"offset: 0x%08x size: %-9u  pid: %02d",r->start, r->end-r->start+1,r->pid);
  show_msg(str);
  r = r->next;
 }

 show_msg("free_list:");
 while (f)
 {
  fc++;
  sprintf(str,"offset: 0x%08x size: %u",f->start, f->end-f->start+1);
  show_msg(str);
  f = f->next;
 }

 show_msg(" ");
 sprintf(str,"res_list entries : %d",rc);
 show_msg(str);
 sprintf(str,"free_list entries: %d",fc);
 show_msg(str);
 sprintf(str,"Memory free: %d",get_freemem());
 show_msg(str);
}

/*
void memstat()
{
 struct mem_list *r = res_list;
 struct mem_list *f = free_list;
 int rc=0,fc=0;

 printk("");
 printk("res_list:");
 while (r)
 {
  rc++;
  printk("start: %d  end: %d  pid: %02d",r->start,r->end,r->pid);
  r = r->next;
 }

 printk("free_list:");
 while (f)
 {
  fc++;
  printk("start: %d  end: %d",f->start,f->end);
  f = f->next;
 }

 printk(" ");
 printk("res_list entries : %d",rc);
 printk("free_list entries: %d",fc);
 printk("Memory free: %d",get_freemem());
}
*/
void memlist()
{
 struct task_struct *task;
 struct mem_list *r;
 unsigned int i,mem;
 char str[256];

 show_msg("Task memory list:");

 for (i=0;i<50;i++)
 {
   mem = 0;
   r = res_list;

   while (r)
   {
     if (r->pid == i)
        mem += r->end - r->start + 1;
     r = r->next;
   }

   task = GetTask(i);

   if (task)
   {
     sprintf(str,"Task %c%10s%c pid: %02d:  %d bytes reserved",'"',task->name,'"',i,mem);
     show_msg(str);
   }
 }

}

/*
void memlist()
{
 struct task_struct *task;
 struct mem_list *r;
 unsigned int i,mem;
 char str[256];


 show_msg2("Task memory list:");

 for (i=0;i<20;i++)
 {
   if ((task = GetTask(i)) != NULL)
   {
     mem = 0;
     r = res_list;

     while (r)
     {
       if (r->pid == task->pid)
          mem += r->end - r->start + 1;
       r = r->next;
     }

     sprintf(str,"Task %02d:  %d bytes reserved",task->pid,mem);
     show_msg2(str);
   }
 }

}
*/

void free_pmem(int pid)
{
 struct mem_list *r = res_list;

 while (r)
 {
  if (r->pid == pid)
    vfree(r->start);

  r = r->next;
 }
}

void page_alloc(int pages)
{
 unsigned int i,j;
 unsigned int *ptr_dir = page_dir, *ptr_tbl = page_table;

 ptr_dir += 768;           /* beginns at 0xc00000000 */

 for (j=0;j<pages;j++)        /* allocation for n pages */
 {
   for (i=0;i<1024;i++)
   {
     *ptr_tbl = (0x400000*(j+1) | 0x7) + 0x1000*i;
     ptr_tbl++;
   }

   ptr_dir++;
   *ptr_dir = (unsigned int) 0x250007 + 0x1000*j;
 }
}

void page_alloc_video()
{
 unsigned int i,lfb_adr;
 unsigned int *ptr_dir = (unsigned int*) PAGE_DIR, *ptr_tbl, *ptr_table;

 lfb_adr = get_lfb_adr();
 ptr_table = (unsigned int*) 0x240000;
 ptr_tbl = ptr_table;
 ptr_dir += 0x340;           /* 0x340: beginns at 0x10000000 */
                             
 for (i=0;i<1024;i++)
 {
   *ptr_tbl = lfb_adr + 0x7 + 0x1000*i;
   ptr_tbl++;
 }

 *ptr_dir = (unsigned int) ptr_table | 0x7;
}

int mem_init(void)
{
  int i;
  unsigned int *pointer = (unsigned int*)0x82000;

  printk("mem.c: Initializing memory manager...\n");

  memstart = 0x200000;

  map_count = 0;
  block_num = MEMMAP_ENLARGE;

  for (i=0;i<MEMMAP_MAX_SIZE >> 3;i++)
     memmap[i] = 0;

  res_list = free_list = NULL;

  max_mem = ((unsigned int)*pointer) / 0x100000;

  printk("mem.c: Memory detected: %dMB",max_mem);

  map_adr = max_mem*0x100000-BLOCK_SIZE; 
  printk("mem.c: Map address at %x",map_adr);

  page_dir = (unsigned int*) PAGE_DIR;
  page_table = (unsigned int*) 0x250000;     /* for x page tables */

  page_alloc((max_mem>>2) - 1);
  printk("mem.c: Pages allocated for using %dMB at 0x%00000008x",max_mem,page_table);

  free_list = ins_list(0x250000+0x1000*(max_mem>>2),max_mem*0x100000-1-0x14000,free_list);

  create_critical_section(&csect_mm);

  return max_mem;
}

unsigned int palloc(int pages)
{
 struct mem_list *f = free_list->prev;
 unsigned int ptr, size = 4096*pages;

 do
 {
  if (((f->end - f->start + 1) + (4096 - f->start % 4096)) >= size)
  {
   enter_critical_section(&csect_mm);

   ptr = f->start;
   ptr += 4096 - ptr % 4096;

   res_list = ins_list(ptr,ptr + size - 1,res_list);
   f->start += size + (4096 - f->start % 4096);
   if (f->start > f->end) free_list = del_list(f,free_list);

   leave_critical_section(&csect_mm);

   return ptr;
  }
  f = f->prev;
 } while (f != free_list);

 printk("mem.c: palloc() - Out of memory");
 return 0;
}

