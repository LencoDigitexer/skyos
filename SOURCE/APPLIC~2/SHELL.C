/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : application\shell.c
/* Last Update: 09.01.1999
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Kernel Debug Shell
/************************************************************************/
#include "sched.h"
#include "rtc.h"
#include "version.h"
#include "ctype.h"
#include "devices.h"
#include "version.h"
#include "ide.h"
#include "error.h"
#include "vfs.h"
#include "net.h"
#include "socket.h"
#include "svga\modes.h"
#include "svga\svgadev.h"
#include "twm.h"

struct twm_window shell = {0};          // Kernel Shell Window

extern unsigned int MAXX;               // Screen X Resolution --> svgadev.c
extern unsigned int MAXY;               // Screen Y Resolution --> svgadev.c

extern char task_time;                  // delay between timertick --> sched.c

extern unsigned int scheduler_debug_wait; // delay between each taskswitch --> sched.c

extern struct s_net_device *active_device; // active network device --> 3c509.c

extern unsigned int net_arp_debug ;        // debug parameters
extern unsigned int net_ip_debug ;
extern unsigned int net_icmp_debug ;
extern unsigned int net_eth_debug ;
extern unsigned int net_udp_debug ;
extern unsigned int net_tcp_debug ;
extern unsigned int tftpd_monitor_debug;

extern unsigned char *act_device;       // actual vfs device --> vfs.c

extern int timerticks;                  // timerticks counter --> sched.c
extern int tasksactive;                 // multiaskting active? --> sched.c

extern struct task_struct *current;     // current task --> sched.c
extern struct task_struct *focus;       // current focus task --> sched.c

extern int TASK_FILESYSTEM;             // pid of filesystem task
extern int TASK_GUI;                    // pid of GUI task

// Prototypes
void help_task(void);                   // --> help.c
void system_monitor(void);              // --> sysmon.c
void device_manager_task(void);         // --> devman.c


static unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

void kernel_debug_shell(void)
{
 struct ipc_message *m;
 struct inode inode;
 unsigned char command[50]={0};
 unsigned char buf[50]={0};
 unsigned char buf2[50]={0};
 unsigned char buf3[50]={0};
 unsigned char buf4[512] ={0};
 unsigned char *buffer;
 unsigned int starttime;
 struct time t;
 struct time t2;
 int ret;
 int i;
 int j;
 int k;
 char ch;
 struct task_struct *task;
 struct object_entry *object_list;

 buffer = (unsigned char*)valloc(512);

 printk("Kernel Shell V2.0a activated.\n");
 printk("");

 shell.x = 10;
 shell.length = 300;
 shell.y = MAXY - 110;
 shell.heigth = 50;
 strcpy(shell.title,"Kernel Debug Shell V1.0a");
 shell.acty = 30;
 shell.actx = 5;

 line(1,25,350,25,15);
 open_window(&shell);
 draw_window(&shell);

 while (1)
 {
   clear_window(&shell);
   gets_window(&shell, command);

   parse(command,buf,0);
   parse(command,buf2,1);
   parse(command,buf3,2);
   parse(command,buf4,3);

   if (!strcmp(buf,"arpdump"))
     arp_dump();
   if (!strcmp(buf,"ethernet"))
     net_buff_debug();

   if (!strcmp(buf,"dumppage"))
     dump_pagetable();

   if (!strcmp(buf,"clock"))
   {
     if (!strcmp(buf2,"set"))
     {
        i = atoi(buf3);
        set_clock(i);
     }
     if (!strcmp(buf2,"sched"))
     {
       starttime = timerticks;

       i = atoi(buf3);

       while (i)
       {
         sys_clock_system_performance();
         i--;
       }
       printk("Elapsed timerticks/seconds: %d/%d",(int)(timerticks - starttime),(int)((timerticks - starttime)/18.2));
     }
   }


   if (!strcmp(buf,"bootlog"))
     dump_messages();

   if (!strcmp(buf,"mount"))
   {
     if ((buf2[0]) && (buf3[0]))
     {
       ret = sys_mount(buf2,buf3);
       error(ret);
     }
     else printk("mount <device> <protocol>");
   }

   if (!strcmp(buf,"mkfs"))
   {
     if ((buf2[0]) && (buf3[0]))
     {
       ret = sys_mkfs(buf2,buf3);
       error(ret);
     }
     else printk("mkfs <device> <protocol>");
   }

   if (!strcmp(buf,"unmount"))
   {
     if (buf2[0])
     {
       ret=sys_unmount(buf2);
       error(ret);
     }
     else printk("unmount <device>");
   }

   if (!strcmp(buf,"fopen"))
   {
     if (buf2[0])
     {
       ret = sys_open(buf2, 0);
       error(ret);
     }
     else printk("fopen <filename>");
   }

   if (!strcmp(buf,"fread"))
   {
     if (buf2[0])
     {
       i = atoi(buf2);
       ret = sys_fread(i);
       error(ret);
     }
     else printk("fread <filedescriptor>");
   }

   if (!strcmp(buf,"cd"))
   {
     printk("Directory:");
     console_gets(buf2);
     ret=sys_change_dir(buf2);
     error(ret);
   }

   if (!strcmp(buf,"open"))
   {
     printk("Filename:");
     console_gets(buf2);
     ret=sys_open(buf2,0);
     error(ret);
   }

   if (!strcmp(buf,"set actdevice"))
   {
     printk("Devicename:");
     console_gets(act_device);
   }

   if (!strcmp(buf,"vfs"))
   {
     if (!strcmp(buf2,"lookup"))
     {
       struct inode in;
       sys_lookup_inode(buf3, &in);
     }

     if (!strcmp(buf2,"show"))
     {
       if (!strcmp(buf3,"mount"))
       {
         show_mount_table();
       }
       if (!strcmp(buf3,"protocol"))
       {
         show_protocol_table();
       }
       if (!strcmp(buf3,"file"))
       {
         show_file_table();
       }

     }
   }

   if (!strcmp(buf,"dump ipc")) dump_ipclog();

   if (!strcmp(buf,"show device status"))
     devices_dump();

   if (!strcmp(buf,"kernelinfo"))
     kernelinfo();


   if (!strcmp(buf,"memdump"))
   {
     printk("size of short: %d",sizeof(short));
     printk("size of int  : %d",sizeof(int));
     printk("size of long : %d",sizeof(long));
     printk("Enter address to dump (dez): ");
     console_gets(buf);
     i = atoi(buf);
     printk("dumping area 0x%00000008X:",i);
     memory_dump(i,200);
   }

   if (!strcmp(buf,"task"))
     tasksactive = 1;

   if (!strcmp(buf,"task control"))
   {
     printk("PID  : ");
     console_gets(buf2);
     i = atoi(buf2);
     printk("State: ");
     console_gets(buf2);
     j = atoi(buf2);
     task_control(i,j);
   }

   if (!strcmp(buf,"send"))
   {
     m->type = atoi(buf3);
     send_msg(atoi(buf2),m);
   }

   if (!strcmp(buf,"readblock"))
   {
     printk("Device: ");
     console_gets(buf);
     printk("First Block: ");
     console_gets(buf2);
     i = atoi(buf2);
     printk("Count: ");
     console_gets(buf2);
     j = atoi(buf2);
     printk("Dump to screen (y/n): ");
     console_gets(buf2);
     ret = 0;

     if (buf2[0] == 'y')
       ret = 1;

     starttime = timerticks;
     get_time(&t);
     k = 0;
     while (k<j)
     {
       readblock(buf, i+k, buffer);
       if (ret) memory_dump(buffer, 512);
       k++;
     }
     get_time(&t2);
     printk("Elapsed seconds (ca.)  : %d",t2.sec - t.sec);
     printk("Elapsed seconds exactly: %d",(int)((timerticks - starttime)/18.2));
   }

   if (!strcmp(buf,"readwrite block"))
   {
     printk("Device: ");
     console_gets(buf);
     printk("First Block: ");
     console_gets(buf2);
     i = atoi(buf2);
     printk("Count: ");
     console_gets(buf2);
     j = atoi(buf2);

     starttime = timerticks;
     get_time(&t);
     k = 0;
     while (k<j)
     {
       readblock(buf, i+k, buffer);             // read block
       writeblock(buf, i+k, buffer);            // write it back
       k++;
     }
     get_time(&t2);
     printk("Elapsed seconds (ca.)  : %d",t2.sec - t.sec);
     printk("Elapsed seconds exactly: %d",(int)((timerticks - starttime)/18.2));
   }

   if (!strcmp(buf,"chkdsk"))
   {
     printk("Device: ");
     console_gets(buf);
     printk("First Block: ");
     console_gets(buf2);
     i = atoi(buf2);
     printk("Count: ");
     console_gets(buf2);
     j = atoi(buf2);

     k = 0;
     ret = 0;
     while (k<j)
     {
       readblock(buf, i+k, buffer);
       ret = ret ^buffer[0];
       k++;
     }
     printk("Checkcode: 0x%02X",ret);
   }

   if (!strcmp(buf,"ps"))
     ps();

   if (!strcmp(buf,"regs"))
   {
     asm("mov %%esp, %0":"=r" (i));
     printk("ESP: 0x%00000008x  %dd",i,i);
   }



   if (!strcmp(buf,"show global system info"))
   {
   }

   if (!strcmp(buf,"show desc"))
   {
      dump_tss();
   }

   if (!strcmp(buf,"sysmemtable"))
     {
        dump_system_mem_table();
     }

   if (!strcmp(buf,"kernelmem"))
     {
        dump_kernel_mem();
     }

}
}



