/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\trap.c
/* Last Update: 11.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus from : Intel 80386.txt / Linux
/************************************************************************/
/* Definition:
/*   Intel processor interrupts and trap handling.
/************************************************************************/
#include "sched.h"
#include "system.h"
#include "head.h"

unsigned long intr_count = 0;

// Interrupt entry points (coded in kernel.s (asm))
extern void do_page_fault();
extern void do_irq_ide();
extern void do_irq_timer();
extern void do_irq_floppy();
extern void do_irq_key();
extern void do_irq_unknown();
extern void do_nothing();
extern void do_irq_el3();
extern void do_irq_mouse();

extern struct task_struct *current;
extern int timerticks;

volatile unsigned long exc_cr2=0;
volatile unsigned long exc_eip=0;

struct twm_window exc={0};

// Display a processor exception
void display_exception(int nr, unsigned char* message)
{
  unsigned char str[255];
  unsigned int cr2;

  exc.x = 200;
  exc.length = 400;
  exc.y = 230;
  exc.heigth = 140;
  exc.style = 128;
  strcpy(exc.title, current->name);

  draw_window(&exc);

  sprintf(str,"Processor exception number 0x%02X occured.",nr);
  out_window(&exc,str);
  out_window(&exc,message);
  sprintf(str,"Current Task: %s/%d",current->name, current->pid);
  out_window(&exc,str);
  out_window(&exc,"");
  if (nr == 14)
  {
    out_window(&exc,"Page fault parameters:");
    sprintf(str,"Linear address: 0x%x",exc_cr2);
    out_window(&exc,str);
    sprintf(str,"Occured at EIP: 0x%x",exc_eip);
    out_window(&exc,str);
  }
  out_window(&exc,"This task will be closed.");
  cli();
  task_control(current->pid, 5);
  sti();
}

// This functions are called from the low-level interrupt handlers,
// which are coded in assembler
void divide_error()
{
 display_exception(0,"Exception: Divide Error.");
 while(1);
}

void debug()
{
 display_exception(1,"Exception: Debug.");
 while(1);
}

void nmi()
{
 display_exception(2,"Exception: Non maskable interrupt occured.");
 while(1);
}

void int3()
{
 display_exception(3,"Exception: Interrupt 3 called.");
 while(1);
}

void overflow()
{
 display_exception(4,"Exception: Overflow.");
 while(1);
}

void bounds()
{
 display_exception(5,"Exception: Bounds.");
 while(1);
}

void invalid_op()
{
 display_exception(6,"Exception: Invalid Opcode.");
 while(1);
}

void device_not_available()
{
 display_exception(7,"Exception: Device not available.");
 while(1);
}

void double_fault()
{
 display_exception(8,"Exception: Double Fault.");
 while(1);
}

void coprocessor_segment_overrun()
{
 display_exception(9,"Exception: Coprocessor segment overrun.");
 while(1);
}

void invalid_TSS()
{
 display_exception(10,"Exception: Invalid Task Status Segment.");
 while(1);
}

void segment_not_present()
{
 display_exception(11,"Exception: Segment not present.");
 while(1);
}

void stack_segment()
{
 display_exception(12,"Exception: Stack segment error");
 while(1);
}
void general_protection()
{
 display_exception(13,"Exception: General Protection fault occured.");
 while(1);
}

void page_fault(unsigned int addr, unsigned int eip)
{
 unsigned char str[255];

 cli();
 exc_cr2 = addr;
 exc_eip = eip;
 display_exception(14,"Exception: Page fault.");
 while(1);
}

void reserved()
{
 display_exception(15,"Exception: Reserved.");
 while(1);
}
void coprocessor_error()
{
 display_exception(16,"Exception: Coprocessor error.");
 while(1);
}

void bad_IRQ_interrupt()
{
 display_exception(255,"IRQ: Unhandled Interrupt Request occured.");
 while(1);
}

// Standard interrupt handlers (if no other handler is set)
static void (*bad_interrupt[16])(void) = {
       bad_IRQ_interrupt, bad_IRQ_interrupt,
       bad_IRQ_interrupt, bad_IRQ_interrupt,
       bad_IRQ_interrupt, bad_IRQ_interrupt,
       bad_IRQ_interrupt, bad_IRQ_interrupt,
       bad_IRQ_interrupt, bad_IRQ_interrupt,
       bad_IRQ_interrupt, bad_IRQ_interrupt,
       bad_IRQ_interrupt, bad_IRQ_interrupt,
       bad_IRQ_interrupt, bad_IRQ_interrupt
};

// Set an interrupt gate in IDT
void Set_intr_gate(unsigned int *IDT, unsigned long num, void *func, unsigned int flag)
{
   IDT[2*num+1] = (((unsigned int) func) & 0xFFFF0000) | flag;
   IDT[2*num] = (((unsigned int) func) & 0x0000FFFF) | (KERNEL_CS << 16);
}

// unused
void trap_enable_scheduler(void)
{
   cli();
   set_trap_gate(0x20,do_irq_timer);
   sti();
}


void interrupt_init()
{
  int i;
  unsigned char tmp;

  set_trap_gate(0,&divide_error);
  set_trap_gate(1,&debug);
  set_trap_gate(2,&nmi);
  set_trap_gate(3,&int3);
  set_trap_gate(4,&overflow);
  set_trap_gate(5,&bounds);
  set_trap_gate(6,&invalid_op);
  set_trap_gate(7,&device_not_available);
  set_trap_gate(8,&double_fault);
  set_trap_gate(9,&coprocessor_segment_overrun);
  set_trap_gate(10,&invalid_TSS);
  set_trap_gate(11,&segment_not_present);
  set_trap_gate(12,&stack_segment);
  set_trap_gate(13,&general_protection);
  set_trap_gate(14,&do_page_fault);
  set_trap_gate(15,&reserved);
  set_trap_gate(16,&coprocessor_error);

  Set_intr_gate((unsigned int*)idt,0x60,&do_nothing, INTR_USER);

  for (i=0;i< 16;i++)
    set_intr_gate(0x20+i,bad_interrupt[i]);

  set_trap_gate(0x20,do_irq_timer);
  set_intr_gate(0x21,do_irq_key);

  // Init serial port for mouse
  // COM1  IRQ4  Port: 0x3f8
  // COM2  IRQ3  Port: 0x2f8

  set_intr_gate(0x24,do_irq_mouse);
  set_intr_gate(0x23,do_irq_mouse);

  set_intr_gate(0x26,do_irq_floppy);

  set_intr_gate(0x2E,do_irq_ide);
  set_intr_gate(0x2F,do_irq_ide);

  intr_count = 0;
}


void set_taskgate_desc(register struct segdesc_s * segdp, unsigned short selector)
{
   segdp->limit_low = 0;
   segdp->granularity = 0;
   segdp->base_high = 0;
   segdp->base_middle = 0;
   segdp->base_low = selector;
   segdp->access = 133;
}

void dump_tss(void)
{
  unsigned int base = 0;
  unsigned int limit = 0;
  struct tss_struct *tss;
  struct segdesc_s * segdp;
  int i,j,k;
  struct twm_window w={0};

  char str[255];
  char str2[255];
  int num;

  w.x = 250;
  w.length = 530;
  w.y = 140;
  w.heigth = 250;
  strcpy(w.title,"Dump Descriptor");
  w.actx = 5;
  w.acty = 30;

  draw_window(&w);

  out_window(&w, "Number:");
  gets_window(&w, str);
  num = atoi(str);

  segdp = (struct segdesc_s*) &(gdt[num]);

  sprintf(str,"GDT at address 0x%x\n",gdt);
  out_window(&w, str);
  sprintf(str,"GDT at address 0x%x\n",&gdt);
  out_window(&w, str);
  sprintf(str,"Descriptor at address 0x%x\n",segdp);
  out_window(&w, str);
  sprintf(str,"Descriptor at address 0x%x\n",&segdp);
  out_window(&w, str);

  base = (segdp->base_high << 24);
  base |= (segdp->base_middle << 16);
  base |= segdp->base_low;

  sprintf(str,"Base at: 0x%00000008x    %dd\n",base,base);
  out_window(&w, str);

  limit = (((segdp->granularity) & 15) << 16) + segdp->limit_low;
  sprintf(str,"TSS - Limit  : 0x%00000008x    %dd\n",limit,limit);
  out_window(&w, str);
  sprintf(str,"TSS - Type   : 0x%02x          %dd\n",segdp->access,segdp->access);
  out_window(&w, str);

  if (segdp->access & 1)
  {
    if (segdp->access & 8)
    {
       if (base >= 0xC0000000)
         base-= 0xC0000000;

       tss = (struct tss_struct*)base;
       sprintf(str,"CS : 0x%00000008x, DS : 0x%00000008x, SS    : 0x%00000008x\n",tss->cs, tss->ds, tss->ss);
       out_window(&w, str);
       sprintf(str,"EIP: 0x%00000008x, EBP: 0x%00000008x, ESP   : 0x%00000008x\n",tss->eip, tss->ebp, tss->esp);
       out_window(&w, str);
       sprintf(str,"LDT: 0x%00000008x, TRB: 0x%00000008x, EFLAGS: 0x%00000008x\n",tss->ldt, tss->trace,tss->eflags);
       out_window(&w, str);
    }
 }
}


void SetSegmentDesc(void *entry,
                    unsigned int base, unsigned int limit,
                    unsigned char rights, unsigned char gran)
{
    *((unsigned int *) entry) = (limit & 0xFFFF) | ((base & 0xFFFF) << 16);

    *((unsigned int *) ( ((char *) entry) + 4 ) ) =
        ((base & 0x00FF0000) >> 16) | (base & 0xFF000000) |
        (rights << 8) | ((gran & 0xF0) << 16) |
        ((limit & 0x000F000) << 4);
}

void SetTSSDesc(void *entry,
                    unsigned int base, unsigned int limit,
                    unsigned char rights, unsigned char gran)
{
    base+=0xC0000000;
    *((unsigned int *) entry) = (limit & 0xFFFF) | ((base & 0xFFFF) << 16);

    *((unsigned int *) ( ((char *) entry) + 4 ) ) =
        ((base & 0x00FF0000) >> 16) | (base & 0xFF000000) |
        (rights << 8) | ((gran & 0xF0) << 16) |
        ((limit & 0x000F000) << 4);
}

