/************************************************************************/
/* Sky Operating System V2
/* Copyright (c)1996 - 2000 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/************************************************************************/
/* File       : kernel\main.c
/* Last Update: 31.05.1999
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Starts the kernel. Initializes all modules (mem, devices, scheduler,
/*   interrupts, traps, gui, ...)
/************************************************************************/
#include "module.h"
#include "head.h"
#include "system.h"
#include "version.h"
#include "error.h"
#include "devman.h"
#include "svga\modes.h"
#include "svga\svgadev.h"

struct system_info current_system = {0};

unsigned int VESA = 0;
unsigned int TVGA = 0;

extern unsigned int MAXX;               // Screen X Resolution --> svgadev.c
extern unsigned int MAXY;               // Screen Y Resolution --> svgadev.c

extern int tasksactive;
extern void do_irq_unknown();

unsigned int memory_init = 0;
unsigned int wait(void)
{
  int i;
   int j;

  for (i=0;i<=10000;i++)
     for (j=0;j<=10000;j++);
}

/************************************************************************/
/* The 32-Bit Protected Mode GNU-C Entry Point.                         */
/************************************************************************/
void start_kernel(void)
{
   int i, j;
   unsigned char* charpointer;
   unsigned char ch;
   unsigned short *pointer;
   unsigned int *pidata;
   int id;

   VESA = 0;

   cli();   // make sure interrupts are disabled

/* Initialize Console (set Video Start Address (0xa0000)) */
   console_init();

/* Register all svga device drivers */
   svgadev_init();

/* First check if we are already in VESA2.0 mode. Would have been set
   from startup.asm */
   pointer = (unsigned short*)0x80496;

   switch (*pointer)
   {
     case 0x12: 
     case 0x6a: display_device_init(DISP_VGA16,*pointer);
                break;
     default:   display_device_init(DISP_VESA2,*pointer);
                page_alloc_video();
	        break;
   }

/* Draw Desktop */
   desktop_draw();

   std_pal();

/* All printk's are written into the system initialization window */

   printk("SKY Operating System V2.0 is booting...\n");
   printk("Copyright (c) 1996 - 2000 by Szeleney Robert");
   printk("Build: %s / %s    Kernel Version: %s",__DATE__,__TIME__, KERNEL_VERSION);
   printk("Buildversion: 1358");
   printk("For more information or download see http://skyos.8m.at");
   printk(" ");

/* Initialize Memory Manager */
   current_system.memory = mem_init() * 1024;
   memory_init = 1;


/* Detecting CPU Information */
   cpuid(&(current_system.cpuinfo));
   dump_cpuinfo(&(current_system.cpuinfo));              // --> pc.c

   printk("main.c: Protected Mode, Paging enabled. Unknown interrupt handler set.\n");


/* Check if EISA or ISA bus */
   if (strncmp((char*)0x0FFFD9, "EISA", 4) == 0)
   {
     printk("main.c: System with EISA bus detected.");
//     register_hardware("EISA Motherboard BUS",-1);
     current_system.bus = 1;
   }
   else
   {
     printk("main.c: System with ISA bus detected.");
//     register_hardware("ISA Motherboard BUS",-1);
     current_system.bus = 0;
   }

/* Initialize interrupts, exception and IRQs */
   cli();
   printk("main.c: Initializing interrupts...\n");
   interrupt_init();

/* Initialize real time clock */
   rtc825x_init();

/* Initialize the scheduler */
   printk("main.c: Initializing scheduler...\n");

/* Initialize the vfs-cache */
//   cache_init();

/* Initialize the scheduler */
   sched_init();

/* Initialize the device manager */
   devreg_init();

   printk("main.c: Enabling Interrupts...");
/* Enable Interrupts */
   asm("sti");
   asm("xorl %eax, %eax");
   asm("outb %al, $0x21");
   asm("outb %al, $0xa1");

/* Now it's time to initialize the devices */
   printk("main.c: VDM initialilzing...");
   VDM_Initialize();

//   ne_init(); DISABLED

/* Initialize Keyboard  */
   keyboard_init();

/* Initialize Mouse  */
   mouse_init();

/* Initialize Block Cache */
//   cache_init();

/* Initialize VFS and all SUB-FS's */
   vfs_init();

   skyfs_init();

   msdos_init();

   cache_init();

//   ext2fs_init();

/* Initialize SKYOS networking */

/* register standard installed hardware */
   id = register_hardware(DEVICE_IDENT_SYSTEM_PHERIPHERAL,
                          "Motherboard BIOS", 0);

   id = register_hardware(DEVICE_IDENT_SYSTEM_PHERIPHERAL,
                          "PC Speaker", 0);
   register_port(id, 0x60, 0x62);

   net_init();
/* Initialize Net Packets Buffering */
   init_net_buff();
/* Initialize Network Devices */
   el3_init();
/* Initialize ARP Protocol */
   arp_init();
/* Initialize IP Protocol */
   ip_init();
/* Initialize UDP Protocol */
   udp_init();
/* Initialize TCP Protocol */
   tcp_init();
/* Initialize Socket interface */
   socket_init();
/* Initialize Internet protocol and internet server (e.g. tftpd) */
   inet_init();

   ifsound_init();

/* New GUI */
   printk("main.c: Switching to advanced GUI...");
   newgui_init();

/* Enable Multitasking  */
   tasksactive = 1;

   while (1);    // Scheduler should never return here;
}


