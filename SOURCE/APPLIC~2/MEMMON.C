/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\memmon.c
/* Last Update: 04.01.1999
/* Version    : alpha
/* Coded by   : Gregory Hayrapetian
/* Docus      :
/************************************************************************/

#include "newgui.h"
#include "sched.h"
#include "fcntl.h"

int memmon_redraw(window_t *win, int x, int y, int value)
{
  int v, dy;
  char str[256];

  v = get_freemem();

  dy = v - value;
  value = v;

  y += dy;
  if (y > 200) y = 200-(y%200);
  if (y < 0) y = -y%200;

  sprintf(str,"Free: %d  Delta: %d  y: %d",v,-dy,y);

  win_draw_fill_rect(win,10,210,350,230,COLOR_LIGHTBLUE);
  win_draw_vline(win,x,10+y,210,COLOR_LIGHTRED);
  win_draw_vline(win,x,10,9+y,COLOR_LIGHTGRAY);
  win_outs(win,15,213,str,COLOR_BLACK,255);

  return value;
}

void memmon_task()
{
  struct ipc_message m;
  window_t *win_memmon = NULL;
  int x, y, value;

  x = 10;
  value = get_freemem();
  y = 200;

  win_memmon = create_window("Memory monitor",400,400,400,300,WF_STANDARD);

  settimer(5);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_TIMER:
            memmon_redraw(win_memmon,x,10,value);

            x++;
            if (x > 350) x = 10;

            break;
       case MSG_GUI_WINDOW_CREATED:
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            cleartimer();
            destroy_app(win_memmon);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_memmon, WF_STOP_OUTPUT, 0);
	    set_wflag(win_memmon, WF_MSG_GUI_REDRAW, 0);
            memmon_redraw(win_memmon,x,10,value);
            break;
     }
  }

}

void memmon_init_app()
{
//  if (!valid_app(win_memmon))
    CreateKernelTask(memmon_task, "memmon", NP,0);
}


