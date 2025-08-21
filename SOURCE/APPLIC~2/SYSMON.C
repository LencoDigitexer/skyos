/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\sysmon.c
/* Last Update: 13.01.1999
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the SKY System monitor
/*   and the palette viewer
/************************************************************************/
#include "sched.h"
#include "devman.h"
#include "newgui.h"

#define SYSMON_SYSTEM 1
#define SYSMON_VFS 2
#define SYSMON_TASKS 3

extern int cache_enabled;
extern int idleticks;
extern int timerticks;
window_t *win_sysmon;
//window_t *win_palmon;
unsigned int sysmon_delta_idle;
unsigned int sysmon_delta_ticks;
unsigned int sysmon_x=40;
unsigned int sysmon_type = SYSMON_SYSTEM;

void sysmon_redraw(void)
{
   unsigned char str[255];
   unsigned int i=0;
   unsigned int flags;
   unsigned int delta;


   win_draw_fill_rect(win_sysmon,0,0,90,10,COLOR_GRAY);
   win_outs(win_sysmon, 20, 1, "System",COLOR_LIGHTGRAY,255);
   win_draw_fill_rect(win_sysmon,92,0,180,10,COLOR_GRAY);
   win_outs(win_sysmon, 128, 1, "VFS",COLOR_LIGHTGRAY,255);
   win_draw_fill_rect(win_sysmon,182,0,270,10,COLOR_GRAY);
   win_outs(win_sysmon, 185, 1, "Tasks",COLOR_LIGHTGRAY,255);

   if (sysmon_type == SYSMON_SYSTEM)
   {
/*
     if (sysmon_delta_idle == 0)
       sysmon_delta_idle = 1;
     if (sysmon_delta_ticks == 0)
       sysmon_delta_ticks = 1;
*/
     if (sysmon_delta_ticks == 0)
       delta = 100;
     else
       delta = 100 - (sysmon_delta_idle *100 / sysmon_delta_ticks);

//     delta = 100 - (sysmon_delta_idle *100 / sysmon_delta_ticks);

     win_outs(win_sysmon,10,150,"  0%",COLOR_BLACK,255);
     win_outs(win_sysmon,10,50,"100%",COLOR_BLACK,255);
     win_draw_vline(win_sysmon,39,50,150,COLOR_BLACK);
     win_draw_fill_rect(win_sysmon,39,151,200,151,COLOR_BLACK);

//     if (delta==0) delta = 1;
     win_draw_vline(win_sysmon,sysmon_x,150-delta,150,COLOR_GREEN);
     win_draw_vline(win_sysmon,sysmon_x,50,149-delta,COLOR_LIGHTGRAY);

     sysmon_x++;
     if (sysmon_x == 200) sysmon_x = 40;
   }

   if (sysmon_type == SYSMON_TASKS)
   {
     win_draw_fill_rect(win_sysmon,10,50,120,140,COLOR_LIGHTGRAY);
     sprintf(str,"Taskswitches  : %d",timerticks);
     win_outs(win_sysmon,10,50,str,COLOR_BLACK,255);
     sprintf(str,"   System/User: %d",timerticks-idleticks);
     win_outs(win_sysmon,10,60,str,COLOR_BLACK,255);
     sprintf(str,"   Idle       : %d",idleticks);
     win_outs(win_sysmon,10,70,str,COLOR_BLACK,255);
     sprintf(str,"Clock frequenz: 100HZ");
     win_outs(win_sysmon,10,90,str,COLOR_BLACK,255);
   }

}

void sysmon_task()
{
  struct ipc_message m;
  unsigned int ret;
  unsigned int start_idle;
  unsigned int start_ticks;
  unsigned char str[255];

  win_sysmon = create_window("System Monitor",400,200,400,300, WF_STANDARD);

  settimer(5);

  start_idle = idleticks;
  start_ticks = timerticks;

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_MOUSE_BUT1_PRESSED:
       show_msgf("x: %d  y: %d",m.MSGT_MOUSE_X,m.MSGT_MOUSE_Y);
               if ((m.MSGT_MOUSE_X < 90) && (m.MSGT_MOUSE_Y < 30))
                  sysmon_type = SYSMON_SYSTEM;
               else if ((m.MSGT_MOUSE_X < 180) && (m.MSGT_MOUSE_Y < 30))
                  sysmon_type = SYSMON_VFS;
               else if ((m.MSGT_MOUSE_X < 270) && (m.MSGT_MOUSE_Y < 30))
                  sysmon_type = SYSMON_TASKS;
            break;
       case MSG_TIMER:
            sysmon_delta_idle = idleticks - start_idle;
            sysmon_delta_ticks = timerticks - start_ticks;
            sysmon_redraw();

            start_idle = idleticks;
            start_ticks = timerticks;
            break;
       case MSG_MOUSE_MOVE:
            break;
       case MSG_GUI_WINDOW_CREATED:
            sysmon_redraw();
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            cleartimer();
            destroy_app(win_sysmon);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_sysmon, WF_STOP_OUTPUT, 0);
	    set_wflag(win_sysmon, WF_MSG_GUI_REDRAW, 0);
	    sysmon_redraw();
            break;
     }
  }
}

void system_monitor()
{
  if (!valid_app(win_sysmon))
	CreateKernelTask(sysmon_task,"sysmon",RTP,0);
	  
}

void palmon_redraw(window_t *win)
{
   unsigned char str[255];
   unsigned int i=0,j;


   for (i=0;i<=255;i++)
      for (j=0;j<4;j++)
           win_draw_vline(win,0+i*4+j,0,400,i);

}

void palmon_task()
{
  struct ipc_message m;
  unsigned int ret;
  rect_t rrect, *rect;
  window_t *win_palmon;

  win_palmon = create_window("Palette Viewer",450,250,400,300, WF_STANDARD);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_WINDOW_CREATED:
//            palmon_redraw(&screen_rect);
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_palmon);
            break;
       case MSG_GUI_REDRAW_EX:
            rect = (rect_t *) m.MSGT_GUI_RECT;
            rrect.x1 = rect->x1;
            rrect.y1 = rect->y1;
            rrect.x2 = rect->x2;
            rrect.y2 = rect->y2;
            vfree(m.MSGT_GUI_RECT);

            show_msgf("rect: (%d,%d,%d,%d)",rrect.x1,rrect.y1,rrect.x2,rrect.y2);

//	    palmon_redraw(&rrect);
       case MSG_GUI_REDRAW:
            set_wflag(win_palmon, WF_STOP_OUTPUT, 0);
	    set_wflag(win_palmon, WF_MSG_GUI_REDRAW, 0);
	    palmon_redraw(win_palmon);
            break;
     }
  }
}

void pal_viewer()
{
//  if (!valid_app(win_palmon))
	CreateKernelTask(palmon_task,"palmon",NP,0);
}



