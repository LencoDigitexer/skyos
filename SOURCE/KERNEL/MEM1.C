#define NULL (void*)0

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

// ***************************************
struct mem_list
{
  unsigned int start, end;
  struct mem_list *next;
  struct mem_list *prev;
};

struct mem_list *res_list, *free_list;
char memmap[1024];  /* for kvalloc(); 8192 blocks with 16 bytes */


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
 * Uses NEXT FIT methode
 */
void ins_list_sort(unsigned int start, unsigned int end)
{
 struct mem_list *l, *f;
 char merge = 0;

 f = free_list;

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
 */
unsigned int valloc(unsigned int size)
{
 struct mem_list *f = free_list;
 unsigned int ptr;

 if (size < 1) return 0;

 while (f != 0)
 {
  if (size <= (f->end - f->start + 1))
  {

   res_list = ins_list(f->start,f->start + size - 1,res_list);
   ptr = f->start;
   f->start += size;
   if (f->start > f->end) free_list = del_list(f,free_list);

   return ptr;
  }
  f = f->next;
 }

 printk("mem.c: valloc() - Out of memory");

 return 0;
}

/*
 * Frees an allocation
 */
int vfree(unsigned int ptr)
{
 struct mem_list *r;
 int  i, j;

 r = res_list;

 while ((r != 0) && (r->start != ptr)) r = r->next;
 if (r == 0)
 {
  printk("mem.c: vfree() - Wrong address to free: %d",ptr);
  return -1;
 }

 ins_list_sort(r->start, r->end);
 res_list = del_list(r, res_list);
 return 0;
}

void memstat()
{
 struct mem_list *r = res_list;
 struct mem_list *f = free_list;
 char map[12];
 int i;

 printk("");
 printk("res_list:");
 while (r)
 {
  printk("start: %d  end: %d",r->start,r->end);
  r = r->next;
 }

 printk("free_list:");
 while (f)
 {
  printk("start: %d  end: %d",f->start,f->end);
  f = f->next;
 }

 printk("Memory free: %d",get_freemem());

}

int set_memmap(unsigned int block, char bit)
{
 if (block >= 8192) return -1;

 if (get_memmap(block) != bit)
     memmap[block >> 3] ^= (1 << (block % 8));

 return 1;
}

int get_memmap(unsigned int block)
{
 if (block >= 8192) return -1;
 return (memmap[block>>3] >> (block % 8)) & 1;
}

unsigned int kvalloc(unsigned int size)
{
 // base adr: 0x200000
 unsigned int block=0;

 while (get_memmap(block) != 0) 
 {
  block++;
  if (block == 8192) return NULL;
 }
 set_memmap(block,1);

 return 0x200000 + (block << 4);
}

int kvfree(unsigned int ptr)
{
 unsigned int block = (ptr - 0x200000) >> 4;

 set_memmap(block,0);
}

unsigned int getmem()
{
  return memstart;
}

unsigned int get_freemem()
{
 struct mem_list *f = free_list;
 unsigned int mem=0;

 while (f)
 {
   mem += f->end - f->start;
   f = f->next;
 }

 return mem;
}
// ***************************************



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


void mem_init(void)
{
  int i;

  printk("mem.c: Initializing memory manager...\n");

  memstart = 0x220000;  // 0x200000 - 0x220000 for kvalloc()

  //init_rmem(memstart);
  printk("mem.c: memstart at 0x%00000008x",memstart);

// **************************************************
  for (i=0;i<1024;i++)
     memmap[i] = 0;

  res_list = free_list = NULL;
  free_list = ins_list(0x250000,0x400000,free_list);
// **************************************************

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
