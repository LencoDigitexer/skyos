/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\except.c
/* Last Update: 05.10.1999
/* Version    :
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*  This file implements the exception task.
/************************************************************************/
#include "sched.h"
#include "newgui.h"
#include "controls.h"

window_t *win_exception;
unsigned int TASK_EXCEPTION;

extern unsigned int MAXX;
extern unsigned int MAXY;

struct task_struct *exc_task;
unsigned int exception_nr;

#define ID_TXT 100
ctl_text_t *txt_1;
ctl_text_t *txt_2;



void exception_task()
{
  struct ipc_message *m;
  unsigned char str[255];

  win_exception = create_window("SkyOS Execption",MAXX/2 - 175, MAXY/2 - 100, 400,200,
                   WF_STANDARD);

  sprintf(str,"Module %s caused an exception %d",exc_task->name, exception_nr);

  txt_1 = create_text(win_exception, 20, 20, 380, 15, str,ID_TXT);
  txt_2 = create_text(win_exception, 20, 40, 380, 15, "Task will be terminated",ID_TXT);
  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_EXCEPTION:
            break;

       case MSG_GUI_CTL_BUTTON_PRESSED:
            switch (m->MSGT_GUI_CTL_ID)
            {
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
            txt_1 = txt_2 = 0;
            destroy_app(win_exception);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_exception, WF_STOP_OUTPUT, 0);
            set_wflag(win_exception, WF_MSG_GUI_REDRAW, 0);
            break;
     }
  }
}

void exception_app(unsigned int nr, struct task_struct *t)
{
  if (!valid_app(win_exception))
  {
      exception_nr = nr;
      exc_task = t;
      TASK_EXCEPTION = ((struct task_struct*)CreateKernelTask(exception_task,"exception",NP,0))->pid;

  }

}
