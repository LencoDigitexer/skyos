/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : apps\msgbox.c
/* Last Update: 16.10.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* This implements the SkyOS message boxes.
/*   Features: Model, NonModal, Customizable buttons (Ok,Cancel, Yes, No)
/************************************************************************/

#include <stdarg.h>
#include "newgui.h"
#include "controls.h"
#include "sched.h"
#include "msgbox.h"

#define MAX_MSGBOXES 20

extern unsigned int MAXX;
extern unsigned int MAXY;
extern struct task_struct *current;

struct s_msgbox
{
  unsigned int pid;
  unsigned int modal;
  unsigned int id;
  unsigned int ctl_id;

  ctl_button_t *but1;
  ctl_button_t *but2;
  ctl_button_t *but3;
  ctl_text_t *text;
  window_t *win;
};

static struct s_msgbox lmsgbox[20] = {0};

static char msgbox_str[2048];
static int str_nr, str_ofs[20];
static int msgbox_pid;
int open_msgboxes = 0;

/* If modal == 1 application will be stopped until message box is closed. */
int msgbox(int type, int modal, char *fmt, ...)
{
  char buf[2048]={0};
  va_list args;
  char *mstr;
  struct ipc_message m;

  struct ipc_message wait_m;

  va_start(args, fmt);
  vsprintf(buf,fmt,args);
  va_end(args);

  mstr = (char*) valloc(strlen(buf)+1);
  if (mstr == NULL)
    return -1;

  strcpy(mstr,buf);

  m.type = MSG_SHOW_ALERT;
  m.MSGT_STRING_ADDRESS = mstr;
  m.MSGT_MSGBOX_MODAL   = modal;
  m.MSGT_MSGBOX_TYPE    = type;
  send_msg(msgbox_pid, &m);

  if (modal == 1)
  {
     wait_msg(&wait_m, MSG_MSGBOX_REPLY);
     return wait_m.MSGT_MSGBOX_TYPE;
  }

  return 0;
}

/*void alert_checked()
{
  struct ipc_message m;

  m.type = MSG_ALERT_CHECKED;
  send_msg(alert_pid, &m);
  printk("alertwin: sended");
} */

void createbox(struct ipc_message *m)
{
  int i;
  int length;
  int type;
  unsigned char str[255];
  struct task_struct *t;


  for (i=0;i<MAX_MSGBOXES;i++)
    if (lmsgbox[i].id == 0) break;

  lmsgbox[i].pid = m->sender;

  lmsgbox[i].modal = m->MSGT_MSGBOX_MODAL;

  type = m->MSGT_MSGBOX_TYPE;

  length = strlen(m->MSGT_STRING_ADDRESS)*8 + 40;

  if (type == ID_OK)
  {
    if (length < 70+40) length = 70+40;
  }
  else if ((type & ID_YES)  && (type & ID_NO) && (type & ID_CANCEL))
  {
    if (length < 70*3 + 20*2 + 40) length = 70*3 + 20*2 + 40;
  }
  else if ((type & ID_OK)  && (type & ID_CANCEL))
  {
    if (length < 70*2 + 20 + 40) length = 70*2 + 20 + 40;
  }
  else if ((type & ID_YES)  && (type & ID_NO))
  {
    if (length < 70*2 + 20 + 40) length = 70*2 + 20 + 40;
  }

  sprintf(str,"Message - unknown");
  t = (struct task_struct*)GetTask(m->sender);
  if (t)
  {
   if (t->name[0] != "\0")
     sprintf(str,"Message - %s",t->name);
  }

  lmsgbox[i].win = create_window(str,
               MAXX/2 - length/2 + ((open_msgboxes)*10),
               MAXY/2 - 60       + ((open_msgboxes)*10),
               length,
               120,
               WF_STANDARD);

  if (type == ID_OK)
    lmsgbox[i].but1 = create_button(lmsgbox[i].win,length/2 - 35,80,70,20,"OK",ID_OK);

  else if ((type & ID_YES) && (type & ID_NO) && (type & ID_CANCEL))
  {
    lmsgbox[i].but1 = create_button(lmsgbox[i].win,length/2 - 35 - 70 - 20,80,70,20,"Yes",ID_YES);
    lmsgbox[i].but2 = create_button(lmsgbox[i].win,length/2 - 35      ,80,70,20,"No",ID_NO);
    lmsgbox[i].but3 = create_button(lmsgbox[i].win,length/2 + 35+20   ,80,70,20,"Cancel",ID_CANCEL);
  }
  else if ((type & ID_OK) && (type & ID_CANCEL))
  {
    lmsgbox[i].but1 = create_button(lmsgbox[i].win,length/2 - 10 - 70 ,80,70,20,"OK",ID_OK);
    lmsgbox[i].but2 = create_button(lmsgbox[i].win,length/2 + 10      ,80,70,20,"Cancel",ID_CANCEL);
  }
  else if ((type & ID_YES) && (type & ID_NO))
  {
    lmsgbox[i].but1 = create_button(lmsgbox[i].win,length/2 - 10 - 70 ,80,70,20,"Yes",ID_YES);
    lmsgbox[i].but2 = create_button(lmsgbox[i].win,length/2 + 10      ,80,70,20,"No",ID_NO);
  }


  lmsgbox[i].text = create_text(lmsgbox[i].win,20,20,length - 35, 25,m->MSGT_STRING_ADDRESS,TEXT_ALIGN_LEFT,0);

  lmsgbox[i].id = lmsgbox[i].win->id;
  open_msgboxes++;

}


void msgbox_task()
{
  struct ipc_message m;
  struct ipc_message m_modal;
  int length;
  int i;
  unsigned int w;

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
              for (i=0;i<MAX_MSGBOXES;i++)
                if (lmsgbox[i].id == m.MSGT_GUI_CTL_WIN) break;

              if (i==MAX_MSGBOXES)
              {
                printk("msgbox.c: Msgbox not open");
                break;
              }

              lmsgbox[i].id = 0;

              if (lmsgbox[i].modal == 1)
              {
                m_modal.type = MSG_MSGBOX_REPLY;
                m_modal.MSGT_MSGBOX_TYPE = m.MSGT_GUI_CTL_ID;
                send_msg(lmsgbox[i].pid, &m_modal);
              }

              destroy_window(lmsgbox[i].win);

              open_msgboxes--;
              break;

       case MSG_SHOW_ALERT:
            createbox(&m);

/*            w = i;

            strcpy(msgbox_str,m.MSGT_STRING_ADDRESS);

            str_nr = 0;
            for (i=strlen(msgbox_str);i>=0;i--)
            {
              if (msgbox_str[i] == '\n')
              {
                msgbox_str[i] = '\0';
                str_ofs[str_nr++] = i+1;
              }
            }
            str_ofs[str_nr++] = 0;*/

            redraw_window(lmsgbox[w].win);
            break;

     }
  }
}

void msgbox_init_app()
{
   msgbox_pid = ((struct task_struct *)CreateKernelTask(msgbox_task,"msgbox",NP,0))->pid;
}

