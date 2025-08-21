/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applic~2\sdeclt.c
/* Last Update: 07.10.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Client
/************************************************************************/
#include "newgui.h"
#include "controls.h"
#include "sched.h"

static ctl_text_t *txt_port;
static ctl_text_t *txt_ip;
static ctl_text_t *txt_info_3;

static ctl_input_t *ipt_port;
static ctl_input_t *ipt_ip;
static ctl_input_t *ipt_date;

static ctl_button_t *but_connect;
static ctl_button_t *but_shutdown;
static ctl_button_t *but_exit;

static window_t *win_sdeclt;

#define ID_PORT 10
#define ID_CONNECT 100
#define ID_EXIT    101
#define ID_IP      102
#define ID_SHUTDOWN 103

void sdeclt_layout(void)
{
  char str[255];

  txt_port = create_text(win_sdeclt,10,10,80,12,"Port:",TEXT_ALIGN_LEFT,1);
  txt_ip   = create_text(win_sdeclt,10,24,80,12,"IP  :",TEXT_ALIGN_LEFT,2);

  ipt_port = create_input(win_sdeclt,100,10,80,10,ID_PORT);
  ipt_ip   = create_input(win_sdeclt,100,24,80,10,ID_IP);

  but_connect  = create_button(win_sdeclt, 10, 130, 70, 18, "Connect", ID_CONNECT);
  but_shutdown = create_button(win_sdeclt, 900, 130, 70, 18, "Shutdown", ID_SHUTDOWN);
  but_exit     = create_button(win_sdeclt, 180, 130, 70, 18, "Exit", ID_EXIT);
}

void sdeclt_shutdown(void)
{
    destroy_app(win_sdeclt);
}

void sdeclt_task()
{
  struct ipc_message *m;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_sdeclt = create_window("SkyOS DE-Client",420,100,200,200,WF_STANDARD);

  sdeclt_layout();

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m->MSGT_GUI_CTL_ID == ID_EXIT)
              sdeclt_shutdown();
            break;

       case MSG_GUI_WINDOW_CLOSE:
            sdeclt_shutdown();
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_sdeclt, WF_STOP_OUTPUT, 0);
            set_wflag(win_sdeclt, WF_MSG_GUI_REDRAW, 0);
            break;
     }
  }
}

void sdeclt_init_app(void)
{
  if (!valid_app(win_sdeclt))
    CreateKernelTask(sdeclt_task,"sdeclt",NP,0);

}
