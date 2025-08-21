/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applic~2\quicklau.c
/* Last Update: 24.08.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*  This is the quick-launch panel.
/************************************************************************/
#include "newgui.h"
#include "controls.h"
#include "sched.h"

window_t *win_quick;

struct quicklaunch_entry
{
  unsigned char name[255];
  void (*addr)(void);
};

extern fm_init_app();
extern app_devman();
extern pal_viewer();
extern system_monitor();
extern app_memman();
extern memmon_init_app();
extern imgview_init_app();
extern app_videocon();
extern ps();
extern ctldemo_init_app();
extern waveplay_init_app();
extern terminal_init_app();
extern wininfo_init_app();
extern app_taskman();
extern msg_init_app();
extern console_init_app();
extern testapp();
extern testapp2();
extern debugger_init_app();
extern skypad_init_app();
extern shutdown();
extern cs_init_app();

static struct quicklaunch_entry quicklaunch[]
   = {
      "Debugger/Disasm",debugger_init_app,
      "SkyPad V0.1a",skypad_init_app,
      "SkyFileMan V0.1a",fm_init_app,
      "SkyConsole", console_init_app,
      "Message Window", msg_init_app,
      "Device Manager", app_devman,
      "System Monitor", system_monitor,
      "Memory Monitor", memmon_init_app,
      "Video Configuration", app_videocon,
      "Window Information", wininfo_init_app,
      "Cache Statistic",cs_init_app,
      "Palette Viewer", pal_viewer,
      "Listbox Demo",     ctldemo_init_app,
      "Image Viewer", imgview_init_app,
      "Wave Player", waveplay_init_app,
      "Terminal", terminal_init_app,
      "Control Demo", testapp,
      "Memory Manager", app_memman,
      "Task Manager", app_taskman,
      "Process Viewer", ps,
      "Shutdown", shutdown,
      0,0};

static ctl_button_t *but_app[25] = {0};

static void quicklau_task()
{
  struct ipc_message m;
  int i = 0;

  while (quicklaunch[i].addr != 0) i++;
  i++;
  win_quick = create_window("SkyOS QuickLaunch",30,30,200,2 + i*18,WF_STANDARD & ~WF_STYLE_BUTCLOSE);

  i=0;
  while (quicklaunch[i].addr != 0)
  {
     but_app[i] = create_button(win_quick, 2, 2+i*18,192, 16, quicklaunch[i].name, i);
     i++;
  }

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            quicklaunch[m.MSGT_GUI_CTL_ID].addr();
            break;

       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_quick);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_quick, WF_STOP_OUTPUT, 0);
            set_wflag(win_quick, WF_MSG_GUI_REDRAW, 0);
            break;
     }
  }
}


void quick_init_app(void)
{
  if (!valid_app(win_quick))
  {
    CreateKernelTask(quicklau_task,"quicklau",NP,0);
  }
  else
  {
    redraw_window(win_quick);
  }
}
