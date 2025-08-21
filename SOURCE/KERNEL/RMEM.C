/************************************************************************/
/* Real Memory Manager
/* (c) 1998 by Szeleney Robert
/* Last Update: 31.07.1998
/************************************************************************/
/* At this time the rmm handles max, 64mb Ram
/************************************************************************/
#include "twm.h"

char rml[2048] = {0}; /* 64mb * 1024 / 4 / 8 */

/* If addr and addrend == 0, next free 4K Page is used */
unsigned int rml_setbit(unsigned int addr, unsigned int addrend, int bit)
{
  int i,j,k;
  unsigned int pos;
  int bitpos;

  if ((addr == 0) && (addrend == 0))    /* Set next free page */
  {
     i=0;

     while (i < 2048)
     {
       j=0;
       while (j < 8)
       {
         if (!(rml[i] & (1<<j)))
           {
              pos = i*8*4096+(j*4096);
              rml[i] = rml[i] | (1<<j);
              return pos;
           }
         j++;
       }
       i++;
     }
     printk("rmem.c: out of memory...");
     return -1;
  }

  while (addr < addrend)
  {
    pos = addr / 32768;

    if (pos == 0)
      bitpos = addr/4096;
    else
      bitpos = (addr % (32768*pos))/4096;

    if (bit == 1)                               /* Mark as reserved */
      rml[pos] = rml[pos] | (1<<bitpos);
    else
    {
      rml[pos] = rml[pos] | (1<<bitpos);        /* Mark as free */
      rml[pos] = rml[pos] ^ (1<<bitpos);        /* Mark as free */
    }

    addr += 4096;
  }

  return -1;
}

int init_rmem(int memstart)
{
   int i;
   int j;
   int dir = 0;
   unsigned int page;
   unsigned int entry;

   printk("rmem.c: Building real-memory-list...");
   printk("rmem.c: Checking page tabels...");

   i = j = dir = 0;

   while (!dir)
   {
      entry = getpt_entry(0,i);                 /* Get Page Dir entry */

      i++;
      if (i==1023) dir = 1;

      if (entry == 0)
        continue;

      entry = entry >> 11;
      entry = entry << 11;

      for (j=0;j<1024;j++)
      {
        page = getpt_entry(entry, j);   /* Get Page entry */

        if (page & 32)                 /* Is accessed ? */
        {
          page = page >> 12;

          rml[(page/8)] = rml[(page/8)] | (1 << (page % 8));
        }
      }
   }

   printk("rmem.c: Updating real memory list...");

   rml_setbit(0x0,0x3000,1);            /* System, Page Tabels */
   rml_setbit(0x9000,0x10000,1);        /* DMA */
   rml_setbit(0x10000,0x40000,1);       /* Kernel */
   rml_setbit(0xA0000,0xBFFFF,1);       /* Video Ram */
   rml_setbit(0xC0000,0xFFFFF,1);       /* Video Ram */
}

int real_alloc_addr(unsigned int vstartaddr)
{
  int error = 0;
  unsigned int table;
  unsigned int page;
  unsigned int dummy;
  unsigned int i;

  /* search if already an entry in page dir for this address */
  page = getpt_entry(0,vstartaddr / (4096 * 1024));
  if (page == 0)  // allocate new page table
  {
     page = rml_setbit(0,0,1);
     if (page == -1) return -1;
     page += 7;               /* Page entry bits */

//     printk("rmem.c: updateing page dir entry...");
     setpt_entry(0,vstartaddr / (4096 * 1024), page);
  }

  /* Calculate entry nummer */
  table = page;
  dummy = vstartaddr;
  i = 0;

  while (dummy % (4096 * 1024))
  {
     dummy--;
     i++;
  }

  table = table >> 11;
  table = table << 11;

  page = getpt_entry(table, i/4096);
//  printk("rmem.c: tableaddr %x  entry: %d  value: %x", table, i/4096, page);

  if (page == 0)
  {
//     printk("rmem.c: updateing page table entry...");
     dummy = rml_setbit(0,0,1);
     if (dummy == -1) return -1;
     dummy += 7;               /* Page entry bits */

     setpt_entry(table, i/4096, dummy);
  }

  return 0;
}

int real_alloc(unsigned int vstartaddr, unsigned int vendaddr)
{
  int error = 0;

  while (vstartaddr < vendaddr)
  {
//    printk("rmem.c: allocation at address 0x%x",vstartaddr);
    error = real_alloc_addr(vstartaddr);
    if (error != 0)
      return -1;
    vstartaddr+=4096;
  }

  error = real_alloc_addr(vendaddr - 1);
  return error;
}

void dump_rml(void)
{
   int i,j,k;
   struct twm_window w={0};
   char ch=0;
   char ch2=0;
   char str[255];
   int count = 0;
   int free = 0;

   w.x = 500;
   w.length = 250;
   w.y = 140;
   w.heigth = 440;
   strcpy(w.title,"Realmem list");
   w.actx = 5;
   w.acty = 30;

   draw_window(&w);

   rect_window(&w, 10, 30, 230, 400, 15);

   i = 0;
   k = 0;
   while (ch != 27)
   {
     for (k=0;k<45;k++)
     {
       for (j=0;j<8;j++)
       {
         if ( (((rml[i+k] & (1<<j)) >> j)) == 1)
           line_window(&w, 11, 31 + k*8+ j, 170, 31+k*8+j, 4);
         else
         {
           line_window(&w, 11, 31 + k*8+ j, 170, 31+k*8+j, 2);
           free++;
         }
         count++;
       }
     }

     sprintf(str,"%dkb",i*8*4);
     outs_l_window(&w, 230, 31, 15, str);

     i+=45;
     sprintf(str,"%dkb",i*8*4);
     outs_l_window(&w, 230, 392, 15, str);

     sprintf(str,"%d",free);
     outs_l_window(&w, 230, 170, 15, str);


     ch = getch();

     sprintf(str,"%d",free);
     outs_l_window(&w, 230, 170, 0, str);

     sprintf(str,"%dkb",(i-45)*8*4);
     outs_l_window(&w, 230, 31, 0, str);

     sprintf(str,"%dkb",i*8*4);
     outs_l_window(&w, 230, 392, 0, str);

     if (ch=='n')
     {
        while (ch2!=27)
        {
        clear_window(&w);
        out_window(&w, "Allocating 4K Frames at:");

        for (j=0;j<=30;j++)
        {
          sprintf(str,"0x%00000008x",rml_setbit(0,0,1));
          out_window(&w, str);
        }
        ch2 = getch();
        }
     }
    }
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



