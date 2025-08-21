/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applic~2\shutdown.c
/* Last Update: 24.08.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*  This is the shutdown application.
/************************************************************************/
#include "newgui.h"
#include "controls.h"
#include "sched.h"
#include "head.h"

window_t *win_shutdown;
extern window_t *win_msg;
extern unsigned int reboot_state;
#define ID_REBOOT 1

static ctl_button_t *but_ok;
static ctl_button_t *but_cancel;

extern unsigned int MAXX;
extern unsigned int MAXY;
extern unsigned int tasksactive;

static void do_shutdown(void)
{
     unsigned int i;

     cli();

     hide_cursor();	
     show_msg("main.c: shuting down configured devices....");

     if (win_msg != NULL)
       destroy_app(win_msg);

     for (i=0;i<MAXY/2+2;i++)
     {
       line(0,i,MAXX,i,0);
       line(0,MAXY-i, MAXX, MAXY-i,0);
     }

     outs_c(0,MAXX,MAXY/2 - 25,15,"Kernel shutdown sequence initiated.");
     outs_c(0,MAXX,MAXY/2 - 5,15,"System is now ready to reboot...");
     outs_c(0,MAXX,MAXY/2 + 15,15,"press CTRL+ALT+DEL to reboot");

     tasksactive = 0;

     reboot_state = 1;
     sti();
     while (1);


}

static void shutdown_task()
{
  struct ipc_message m;
  int i = 0;

  win_shutdown = create_window("Confirm system shutdown",MAXX / 2 - 140, MAXY / 2 - 50 ,280, 120 ,
                              WF_STANDARD & ~WF_STYLE_BUTCLOSE & ~WF_STATE_MOVEABLE & ~WF_STATE_SIZEABLE);

  but_ok = create_button(win_shutdown, 20, 10,240, 16, "Prepare for system shutdown", 1);
  but_cancel = create_button(win_shutdown, 20, 40,240, 16, "Cancel", 2);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m.MSGT_GUI_CTL_ID == ID_REBOOT) do_shutdown();
            else
              destroy_app(win_shutdown);

            break;

       case MSG_GUI_WINDOW_CLOSE:
              destroy_app(win_shutdown);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_shutdown, WF_STOP_OUTPUT, 0);
            set_wflag(win_shutdown, WF_MSG_GUI_REDRAW, 0);
            break;
     }
  }
}


void shutdown(void)
{
  if (!valid_app(win_shutdown))
  {
    CreateKernelTask(shutdown_task,"shutdown",NP,0);
  }
}