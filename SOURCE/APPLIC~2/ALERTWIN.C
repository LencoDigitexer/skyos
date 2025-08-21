/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\alertwin.c
/* Last Update: 18.03.1999
/* Version    : alpha
/* Coded by   : Gregory Hayrapetian
/* Docus      :
/************************************************************************/

#include <stdarg.h>
#include "newgui.h"
#include "controls.h"
#include "sched.h"

window_t *win_alert;
int alert_pid;
char alert_str[2048];
int str_nr, str_ofs[20];


int show_alertf(char *fmt, ...)
{
  char buf[2048]={0};
  va_list args;

  va_start(args, fmt);
  vsprintf(buf,fmt,args);
  va_end(args);

  show_alert(buf);
}

int show_alert(char *str)
{
  char *mstr;
  struct ipc_message m;

  mstr = (char*) valloc(strlen(str)+1);
  if (mstr == NULL)
    return -2;

  strcpy(mstr,str);

  m.type = MSG_SHOW_ALERT;
  m.MSGT_STRING_ADDRESS = mstr;
  send_msg(alert_pid, &m);

  printk("showalert sended");

  return 0;
}

void alert_checked()
{
  struct ipc_message m;

  m.type = MSG_ALERT_CHECKED;
  send_msg(alert_pid, &m);
  printk("alertwin: sended");
}

void alert_task()
{
  struct ipc_message m;
  int i;

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_SHOW_ALERT:
            strcpy(alert_str,m.MSGT_STRING_ADDRESS);

            str_nr = 0;
            for (i=strlen(alert_str);i>=0;i--)
            {
              if (alert_str[i] == '\n')
              {
                alert_str[i] = '\0';
                str_ofs[str_nr++] = i+1;
              }
            }
            str_ofs[str_nr++] = 0;

            printk("starting alertwin..");
            alert_init_app();

            wait_msg(&m, MSG_ALERT_CHECKED);

            printk("alerttask: back!");
            win_alert = NULL;

            vfree(m.MSGT_STRING_ADDRESS);
            break;
     }

  }

}

void alertwin_task()
{
  struct ipc_message m;
  ctl_button_t *but;
  ctl_text_t *text[20];
  int i;

  win_alert = create_window("Alert window",262,309,500,90+str_nr*10,WF_STANDARD);

  for (i=0;i<str_nr;i++)
    text[i] = create_text(win_alert,20,10+i*10,600,25,&alert_str[str_ofs[str_nr-i-1]],TEXT_ALIGN_LEFT,i+5);

  but = create_button(win_alert,210,30+str_nr*10,70,20,"OK",1);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_WINDOW_CLOSE:
            alert_checked();
            destroy_app(win_alert);
            win_alert = NULL;
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m.MSGT_GUI_CTL_ID == 1)
            {
              alert_checked();
              destroy_app(win_alert);
              win_alert = NULL;
            }
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_alert, WF_STOP_OUTPUT, 0);
            set_wflag(win_alert, WF_MSG_GUI_REDRAW, 0);
//            alert_redraw();
            break;
     }

  }

}

void alert_init()
{
  alert_pid = ((struct task_struct*)CreateKernelTask((unsigned int)alert_task, "alert_tsk", NP,0))->pid;
}

void alert_init_app()
{
  printk("valid?");
  if (!valid_app(win_alert))
  {
	CreateKernelTask(alertwin_task,"alertwin",NP,0);
    printk("creating!");
    redraw_window(win_alert);
  }
}


