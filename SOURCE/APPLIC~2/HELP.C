/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\help.c
/* Last Update: 04.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the Help Application
/************************************************************************/
#include "sched.h"
#include "devman.h"
#include "ccm.h"
#include "newgui.h"

extern struct s_hardware *installed_hardware;
extern struct s_irq_usage *irq_usage;
extern struct system_info current_system;
extern struct list *listhardware;

window_t *win_dm;

void help(void)
{
    show_msgf("List of all commands:");
    show_msgf("");
    show_msgf("actdev           alert         applist");
    show_msgf("arpdump          blend         bootlog");
    show_msgf("cache            clock         close");
    show_msgf("*ctldemo         debug         devices");
    show_msgf("*devicemanager   !div0         !em");
    show_msgf("ethernet         freemem       filelength");
    show_msgf("filetable        fullmove      *imgview");
    show_msgf("!int3            !int8         ip");
    show_msgf("ipclog           !kill         killapp");
    show_msgf("kvalloc          lm            mapfile");
    show_msgf("memlist          memdump       *memman");
    show_msgf("*memmon          memstat       memtest");
    show_msgf("!module          mount         mounttable");
    show_msgf("*msgwin          net           nwin");
    show_msgf("open             palloc        *palviewer");
    show_msgf("protocoltable    read          readblock");
    show_msgf("redraw           sched         seek");
    show_msgf("setback          showmsg       shutdown");
    show_msgf("*sysmon          *taskman      *terminal");
    show_msgf("test             *testapp      !testlfb");
    show_msgf("unmount          valloc        vfree");
    show_msgf("!vesainfo        *wininfo      *debugger");
    show_msgf("*skypad          *fm           *cs");
    show_msgf("*mapfile");

    show_msgf("");
    show_msgf("! Some commands with false parameters");
    show_msgf("would cause a system-reboot.");
    show_msgf("mount a device with: \"mount hd0a fat\" for example");
    show_msgf("access a file with : \"//hd0a/config.sys\" for example");
}

void dm_redraw(void)
{
   struct s_hardware *p;
   struct s_irq *pirq;
   struct s_dma *pdma;
   struct s_port *pport;
   unsigned char str[255];
   unsigned char name[255];
   int i;
   int j;
   char scpu[50];
   char sfpu[50];
   struct cpu_info *cpu;
   int count = 0;

   cpu = &(current_system.cpuinfo);

   if (cpu->family == CPU_FAMILY_386)
     sprintf(scpu,"80386-Class CPU");
   if (cpu->family == CPU_FAMILY_486)
     sprintf(scpu,"80486-Class CPU");
   if (cpu->family == CPU_FAMILY_PENTIUM)
     sprintf(scpu,"Pentium-Class(\"586\")");
   if (cpu->family == CPU_FAMILY_PENTIUM_PRO)
     sprintf(scpu,"Pentium-Pro-Class(\"686\")");

   if (cpu->family > CPU_FAMILY_486)
   {
     if (cpu->feature_flags & CPUF_ON_CHIP_FPU)
       sprintf(sfpu,"Build in floating-point unit.");
     else
       sprintf(sfpu,"No Build in floating-point unit.");

     sprintf(str, "Processor: %s %s",scpu,sfpu);
     win_outs(win_dm,10,10+count*10,str,COLOR_BLACK,255);

   }
   else
   {
     sprintf(str, "Processor: %s",scpu);
     win_outs(win_dm,10,10+count*10,str,COLOR_BLACK,255);
   }
   count++;

   sprintf(str,"Memory   : %d MB",current_system.memory / 1024);
   win_outs(win_dm,10,10+(count++)*10,str,COLOR_BLACK,255);

   sprintf(str,"Systembus: %s",(current_system.bus==1)?"EISA":"ISA");
   win_outs(win_dm,10,10+(count++)*10,str,COLOR_BLACK,255);

   win_outs(win_dm,10,10+(count++)*10,"",COLOR_BLACK,255);

   win_outs(win_dm,10,10+(count++)*10,"List of all installed hardware:",COLOR_BLACK,255);
   win_outs(win_dm,10,10+(count++)*10,"-------------------------------",COLOR_BLACK,255);
   win_outs(win_dm,10,10+(count++)*10,"Status   Device",COLOR_BLACK,255);

   i=0;
   p = (struct s_hardware*)get_item(listhardware,i++);

   while (p!=NULL)
   {
     sprintf(str,"");
     win_outs(win_dm,10,10+(count++)*10,str,COLOR_BLACK,255);
     sprintf(str,"%-6s   %s","OK",p->name);
     win_outs(win_dm,10,10+(count++)*10,str,COLOR_BLACK,255);

     j = 0;
     pirq = (struct s_irq*)get_item(p->irq, j++);
     while (pirq!=NULL)
     {
       sprintf(str,"%-6s   IRQ: %02d","",pirq->nummer);
       win_outs(win_dm,10,10+(count++)*10,str,COLOR_BLACK,255);
       pirq=(struct s_irq*)get_item(p->irq,j++);
     }

     j = 0;
     pdma = (struct s_dma*)get_item(p->dma, j++);
     while (pdma!=NULL)
     {
       sprintf(str,"%-6s   DMA: %02d","",pdma->nummer);
       win_outs(win_dm,10,10+(count++)*10,str,COLOR_BLACK,255);
       pdma=(struct s_dma*)get_item(p->dma,j++);
     }

     j = 0;
     pport = (struct s_port*)get_item(p->port, j++);
     while (pport!=NULL)
     {
       sprintf(str,"%-6s   Ports: 0x%000X - 0x%000X","",pport->from, pport->to);
       win_outs(win_dm,10,10+(count++)*10,str,COLOR_BLACK,255);
       pport=(struct s_port*)get_item(p->port,j++);
     }
     p = (struct s_hardware*)get_item(listhardware,i++);
   }
   return;
}

void device_manager_task(void)
{
     struct ipc_message m;
     unsigned char buffer[255]={0};

     win_dm = create_window("Device manager",300,80,500,450,WF_STANDARD);

     while (1)
     {
       wait_msg(&m, -1);

       switch (m.type)
       {
         case MSG_GUI_REDRAW:
           set_wflag(win_dm, WF_STOP_OUTPUT, 0);
           set_wflag(win_dm, WF_MSG_GUI_REDRAW, 0);
           dm_redraw();
           break;
         case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_dm);
            break;
       }
     }

}

void device_manager(void)
{
  if (!valid_app(win_dm))
    CreateKernelTask(device_manager_task,"devman",NP,0);
}


