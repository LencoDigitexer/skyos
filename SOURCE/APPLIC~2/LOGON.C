/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\logon.c
/* Last Update: 18.09.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*  This file implements the logon task.
/************************************************************************/
#include "sched.h"
#include "newgui.h"
#include "controls.h"

#define ID_LOGON 1
#define ID_SHUTDOWN 2
#define ID_USERNAME 10
#define ID_PASSWORD 11
#define ID_TXT 255

unsigned int logon_status = 0;
unsigned int logon_task_pid = 0;
window_t *win_logon;
ctl_button_t *but_logon,*but_shutdown;
ctl_input_t *ipt_username, *ipt_password;
ctl_text_t *txt_1, *txt_2, *txt_3, *txt_4,*txt_username, *txt_password;

extern unsigned int MAXX;
extern unsigned int MAXY;

void logon_redraw(void)
{
}

unsigned int logon_state(void)
{
  return logon_status;
}

void logon_finished(int success)
{
  /* init the SKY Panel */
  skypanel_init();

  /* start applications */
  console_init_app();
  msg_init_app();
  quick_init_app();
  app_videocon();

  destroy_app(win_logon);
}

void logon_do(void)
{
  unsigned char username[255];
  unsigned char password[255];

  GetItemText_input(win_logon, ipt_username, username);
  GetItemText_input(win_logon, ipt_password, password);

  logon_finished(1);
}

void logon_layout(void)
{
  but_logon = create_button(win_logon,40,120,170,25,"Logon",ID_LOGON);
  but_shutdown = create_button(win_logon,240,120,170,25,"Shutdown",ID_SHUTDOWN);
  txt_1 = create_text(win_logon, 20, 20, 380, 15, "Welcome to SkyOS V2.0",ID_TXT);
  txt_2 = create_text(win_logon, 20, 40, 380, 15, "Please identify with username and password.",ID_TXT);

  txt_username = create_text(win_logon, 20, 70, 100, 15, "Username:",ID_TXT);
  txt_password = create_text(win_logon, 20, 100, 100, 15, "Password:",ID_TXT);

  ipt_username = create_input(win_logon,120, 70, 250, 15, ID_USERNAME);
  ipt_password = create_input(win_logon,120, 100, 250, 15, ID_PASSWORD);
}

void logon_task()
{
  struct ipc_message *m;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_logon = create_window("SkyOS Logon",MAXX/2 - 175,MAXY/2-100,400,200,
                    WF_STANDARD & ~WF_STYLE_BUTCLOSE & ~WF_STATE_MOVEABLE & ~WF_STATE_SIZEABLE);
  logon_layout();

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_LOGON:
            logon_task_pid = m->sender;
            break;

       case MSG_GUI_CTL_BUTTON_PRESSED:
            switch (m->MSGT_GUI_CTL_ID)
            {
               case ID_LOGON:
                 logon_do();
                 break;
               case ID_SHUTDOWN:
                 shutdown();
                 break;
            }
            break;

       case MSG_MOUSE_BUT1_PRESSED:
            break;
       case MSG_TIMER:
            break;
       case MSG_MOUSE_MOVE:
            break;
       case MSG_GUI_WINDOW_CREATED:
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_logon);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_logon, WF_STOP_OUTPUT, 0);
            set_wflag(win_logon, WF_MSG_GUI_REDRAW, 0);
            logon_redraw();
            break;
     }
  }
}

void logon()
{
  // logon task already started?
  if (!valid_app(win_logon))
  {
    // logon already done?
    if (!logon_state())
        CreateKernelTask(logon_task,"logon",NP,0);
  }
}
