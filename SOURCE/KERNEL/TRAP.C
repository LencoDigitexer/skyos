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
#include "head.h"
#include "twm.h"
#include "msg_lib.h"
#include "types.h"
#include "rtc.h"

struct s_task_gate_desc
{
   unsigned short not_used_1;
   unsigned short selector;
   unsigned char not_user_2;
   unsigned char type;
   unsigned short not_used_3;
};

unsigned long intr_count = 0;

// Interrupt entry points (coded in kernel.s (asm))
extern void do_irq_ide();
extern void do_irq_timer();
extern void do_irq_floppy();
extern void do_irq_key();
extern void do_irq_unknown();
extern void do_nothing();
extern void do_irq_el3();
extern void do_irq_mouse1();
extern void do_irq_mouse2();
extern void do_syscall();

extern void loop();

extern struct task_struct *current;
extern int timerticks;

struct twm_window exc={0};
extern unsigned int MAXX, MAXY;
extern unsigned int TASK_EXCEPTION;
struct tss_struct exc_handler[17];
unsigned char exc_handler_stack[17][1024];

void SetTSSDesc(void *entry,
                    unsigned int base, unsigned int limit,
                    unsigned char rights, unsigned char gran);

#define SAVE_ALL \
 asm("xor %eax, %eax"); \
 asm("movl $0x18, %eax"); \
 asm("movw %ax, %ds"); \
 asm("movw %ax, %es"); \
 asm("movw %ax, %ss");

void exception(int nr)
{
  // Interrupts must still be disabled
  // Problems if exception occurs in Newgui, Keyboard, Mouse and VGA Driver

  exception_app(nr, current);

  task_control(current->pid, TASK_STOPPED);

  Scheduler();
}

void bad_IRQ_interrupt(unsigned int stack, unsigned int errorcode)
{
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
   set_intr_gate(0x20,do_irq_timer);
   sti();
}

const char	ExceptionMess[19][80] =
				{ "Exception 00: DIVISION BY ZERO OR DIVISION OVERFLOW\r\n",
				  "Exception 01: DEBUG EXCEPTION DETECTED\r\n",
				  "Exception 02: NON MASKABLE INTERRUPT\r\n",
				  "Exception 03: BREAKPOINT INSTRUCTION DETECTED\r\n",
				  "Exception 04: INTO DETECTED OVERFLOW\r\n",
				  "Exception 05: BOUND RANGE EXCEEDED\r\n",
				  "Exception 06: INVALID OPCODE\r\n",
				  "Exception 07: PROCESSOR EXTENSION NOT AVAILABLE\r\n",
				  "Exception 08: DOUBLE FAULT DETECTED\r\n",
				  "Exception 09: PROCESSOR EXTENSION PROTECTION FAULT\r\n",
				  "Exception 0A: INVALID TASK STATE SEGMENT\r\n",
				  "Exception 0B: SEGMENT NOT PRESENT\r\n",
				  "Exception 0C: STACK FAULT\r\n",
				  "Exception 0D: GENERAL PROTECTION FAULT\r\n",
				  "Exception 0E: PAGE FAULT\r\n",
				  "Exception 0F: UNKNOWN EXCEPTION\r\n",
				  "Exception 10: COPROCESSOR ERROR\r\n",
				  "Exception 11: ALIGNMENT ERROR\r\n",
				  "Exception 12: MACHINE CHECK EXCEPTION\r\n"};

void	(*ExcHandler)(IntRegs *, int);

#define EXC_STUB_NAME2(nr) nr##_stub(void)
#define EXC_STUB_NAME(nr) EXC_STUB_NAME2(Exception##nr)

#define EXC_STUB(Nr)                      \
void EXC_STUB_NAME(Nr);                   \
__asm__ (                                 \
        "\n.align 4\n"                    \
        "_Exception" #Nr "_stub:\n\t"     \
        "push   $0\n\t"                   \
        "pushal\n\t"                      \
        "pushl %ds\n\t"                   \
        "pushl %ss\n\t"                   \
        "pushl %es\n\t"                   \
        "pushl %fs\n\t"                   \
        "pushl %gs\n\t"                   \
	    "pushl  $" #Nr "\n\t"             \
        "lea   4(%esp), %EAX\n\t"         \
        "pushl %eax\n\t"                  \
        "call   _DoExc\n\t"               \
	    "addl   $8,%esp\n\t"              \
        "popl   %eax\n\t"                 \
        "popl   %eax\n\t"                 \
        "popl   %eax\n\t"                 \
        "popl   %eax\n\t"                 \
        "popl   %eax\n\t"                 \
        "popal\n\t"                       \
        "popl   %eax\n\t"                 \
        "iret\n"                          \
       );

#define EXC_STUB_ERR(Nr)                  \
void EXC_STUB_NAME(Nr);                   \
__asm__ (                                 \
        "\n.align 4\n"                    \
        "_Exception" #Nr "_stub:\n\t"     \
        "pushal\n\t"                      \
        "pushl %ds\n\t"                   \
        "pushl %ss\n\t"                   \
        "pushl %es\n\t"                   \
        "pushl %fs\n\t"                   \
        "pushl %gs\n\t"                   \
	    "pushl $" #Nr "\n\t"              \
        "lea   4(%esp), %EAX\n\t"         \
        "pushl %eax\n\t"                  \
        "call   _DoExc\n\t"               \
        "addl   $8,%esp\n\t"              \
        "popl   %eax\n\t"                 \
        "popl   %eax\n\t"                 \
        "popl   %eax\n\t"                 \
        "popl   %eax\n\t"                 \
        "popl   %eax\n\t"                 \
        "popal\n\t"                       \
        "popl   %eax\n\t"                 \
        "iret\n"                          \
       );


EXC_STUB(0);
EXC_STUB(1);
EXC_STUB(2);
EXC_STUB(3);
EXC_STUB(4);
EXC_STUB(5);
EXC_STUB(6);
EXC_STUB(7);
EXC_STUB_ERR(8);
EXC_STUB(9);
EXC_STUB_ERR(10);
EXC_STUB_ERR(11);
EXC_STUB_ERR(12);
EXC_STUB_ERR(13);
EXC_STUB_ERR(14);
EXC_STUB(15);
EXC_STUB(16);
EXC_STUB_ERR(17);
EXC_STUB_ERR(18);

void interrupt_init()
{
  int i;
  unsigned char str[255];

  set_system_gate(0x80, &do_syscall);

  for (i=0;i< 16;i++)
    set_intr_gate(0x20+i,bad_interrupt[i]);

  set_intr_gate(0x20,do_irq_timer);
  set_intr_gate(0x21,do_irq_key);

  // Init serial port for mouse
  set_intr_gate(0x24,do_irq_mouse1);
  set_intr_gate(0x23,do_irq_mouse2);

  set_intr_gate(0x26,do_irq_floppy);

  set_intr_gate(0x2E,do_irq_ide);
  set_intr_gate(0x2F,do_irq_ide);

  intr_count = 0;

  set_intr_gate(0, (unsigned long) Exception0_stub);
  set_intr_gate(1, (unsigned long) Exception1_stub);
  set_intr_gate(2, (unsigned long) Exception2_stub);
  set_intr_gate(3, (unsigned long) Exception3_stub);
  set_intr_gate(4, (unsigned long) Exception4_stub);
  set_intr_gate(5, (unsigned long) Exception5_stub);
  set_intr_gate(6, (unsigned long) Exception6_stub);
  set_intr_gate(7, (unsigned long) Exception7_stub);
  set_intr_gate(8, (unsigned long) Exception8_stub);
  set_intr_gate(9, (unsigned long) Exception9_stub);
  set_intr_gate(10, (unsigned long) Exception10_stub);
  set_intr_gate(11, (unsigned long) Exception11_stub);
  set_intr_gate(12, (unsigned long) Exception12_stub);
  set_intr_gate(13, (unsigned long) Exception13_stub);
  set_intr_gate(14, (unsigned long) Exception14_stub);
  set_intr_gate(15, (unsigned long) Exception15_stub);
  set_intr_gate(16, (unsigned long) Exception16_stub);
  set_intr_gate(17, (unsigned long) Exception17_stub);
  set_intr_gate(18, (unsigned long) Exception18_stub);
}

void DoExc(IntRegs *regs, int nr)
{
	DefaultExceptionHandler(regs, nr);
}

static void DefaultExceptionHandler(IntRegs *r, int nr)
{
	unsigned long CR2;
   unsigned int addr;
   int i;
   unsigned char str[255];
   unsigned char str2[255];
   struct time t;
   int sec;
   unsigned int funcaddr;

	__asm__ __volatile__
	  ("movl %%cr2,%0"
	  :"=r" (CR2)
	  : );

   outportb(0x3c8,32);

   for (i=0;i<64;i++)
   {
    outportb(0x3c9,i);   /* RED   */
    outportb(0x3c9,0);   /* GREEN */
    outportb(0x3c9,0);   /* BLUE  */
   }

   exc.x = (MAXX-600)/2;
   exc.length = 600;
   exc.y = (MAXY-440)/2;
   exc.heigth = 440;
   exc.style = 128;
   strcpy(exc.title, current->name);

   draw_window(&exc);

   sprintf(str,"%s",ExceptionMess[nr]);
   out_window(&exc,str);
   sprintf(str,"Current Task: %s/%d",current->name, current->pid);
   out_window(&exc,str);
   out_window(&exc,"");

	sprintf(str, "Error Code = %08lX, CR2 = %08lX\r\n", r->ErrorCode, CR2);
   out_window(&exc,str);
	sprintf(str, "EAX = %08lX, EBX = %08lX, ECX = %08lX, EDX = %08lX", r->EAX, r->EBX, r->ECX, r->EDX);
   out_window(&exc,str);
	sprintf(str, "EFL = %08lX, ESI = %08lX, EDI = %08lX, EBP = %08lX\r\n", r->EFLAGS, r->ESI, r->EDI, r->EBP);
   out_window(&exc,str);
	sprintf(str, "ESP = %08lX, EIP = %08lX", r->ESP, r->EIP);
   out_window(&exc,str);

	sprintf(str, "CS = %04X, DS = %04X, ES = %04X", r->CS, r->DS, r->ES);
   out_window(&exc,str);
	sprintf(str, "FS = %04X, GS = %04X, SS = %04X", r->FS, r->GS, r->SS);
   out_window(&exc,str);

   out_window(&exc,"");
   out_window(&exc,"");

   addr = r->EIP;

   funcaddr = mapfile_get_function(addr,str2);

   if (!funcaddr)
     sprintf(str,"Function: unknown");
   else
     sprintf(str,"Function: %s",str2);

   out_window(&exc,str);

   for(i = 0; i < 15; i++)
   {
      addr = unassemble(0x10000, addr, 0, r, str);
      out_window(&exc, str);
    }

   out_window(&exc,"System will reboot in 30 seconds... ");
	// Halt machine

   sec = 0;
   i = 0;

   get_time(&t);
   sec = t.sec;

   while (1)
   {
	  get_time(&t);
     if (sec != t.sec)
     {
       i++;
       sec = t.sec;
     }
     if (i==30)
     {
       out_window(&exc,"reboot now...");
       reboot();
     }
   }
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
        ((limit & 0xFFFF0000) << 4);
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


