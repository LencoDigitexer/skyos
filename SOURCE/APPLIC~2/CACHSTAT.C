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
#include "buffer.h"

extern unsigned int cache_active;
extern struct s_cache *cache;

int cs_redraw(window_t *win)
{
  int v, dy;
  char str[256];
  int color;



  win_draw_hline(win,20,360,220,COLOR_BLACK);

  for (v=0;v<MAX_CACHE_BLOCKS;v++)
  {
    dy = cache->block[v].count;
    if (dy>150) dy = 150;

    win_draw_vline(win, 20+v,70, 220-dy,COLOR_LIGHTGRAY);

    if (dy<10) color = COLOR_LIGHTGREEN;
    else if (dy<100) color = COLOR_LIGHTRED;
    else color = COLOR_RED;
    win_draw_vline(win, 20+v,220-dy, 219, color);
  }

  sprintf(str,"Slot %d",MAX_CACHE_BLOCKS-1);
  win_outs(win,20+MAX_CACHE_BLOCKS,230,str,COLOR_BLACK,255);
}

void cs_task()
{
  struct ipc_message m;
  window_t *win_cs = NULL;

  win_cs = create_window("Cache Statistic",100,100,400,300,WF_STANDARD);

  settimer(5);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_TIMER:
            cs_redraw(win_cs);
            break;
       case MSG_GUI_WINDOW_CREATED:
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            cleartimer();
            destroy_app(win_cs);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_cs, WF_STOP_OUTPUT, 0);
   	      set_wflag(win_cs, WF_MSG_GUI_REDRAW, 0);
            cs_redraw(win_cs);
            break;
     }
  }

}

void cs_init_app()
{
//  if (!valid_app(win_cs))
    CreateKernelTask(cs_task, "CacheState", NP,0);
}

