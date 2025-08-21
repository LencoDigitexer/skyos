/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 -1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\syslog.c
/* Last Update: 16.12.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux
/************************************************************************/
/* Definition:
/*   Implementation of the kernel syslog task.
/************************************************************************/
#include "list.h"
#include "sched.h"

struct list *syslog_content = NULL;
struct window_t *syslog_win;
unsigned int syslog_lines = 0;

int syslog = 0;

void syslog_add(unsigned char *str)
{
  unsigned int flags;
  unsigned char* fi;
  int i=0;
  int j=0;

  save_flags(flags);
  cli();

  fi=(unsigned char*)get_item(syslog_content,i++);

  while (fi!=NULL)
  {
    win_outs(syslog_win, 10, 40+j*10, 7, fi);
    j++;
    fi=(unsigned char*)get_item(syslog_content,i++);
  }

  if (syslog_lines > 10)
  {
    syslog_content=(struct list*)del_item(syslog_content,0);
    syslog_lines--;
  }

  syslog_content = (struct list*)add_item(
    syslog_content,str,strlen(str));

  pass_msg(syslog_win, MSG_GUI_REDRAW);

  syslog_lines++;

  restore_flags(flags);
}

void syslog_redraw(void)
{
  unsigned int flags;
  unsigned char str[255];
  unsigned char* fi;
  int i=0;
  int j=0;

  save_flags(flags);
  cli();

  fi=(unsigned char*)get_item(syslog_content,i++);

  while (fi!=NULL)
  {
    win_outs(syslog_win, 10, 40+j*10, 8, fi);
    j++;
    fi=(unsigned char*)get_item(syslog_content,i++);
  }

  restore_flags(flags);
}

void syslog_task()
{
  struct ipc_message *m;
  unsigned int ret;
  int i=0;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_MOUSE_BUT1_PRESSED:
            break;
       case MSG_MOUSE_BUT1_RELEASED:
            break;
       case MSG_GUI_REDRAW:
            hide_cursor();
            syslog_redraw();
            show_cursor();
            break;
     }
  }
}

void syslog_init(void)
{
  printk("syslog.c: Initializing system message logging...");

 // syslog_win = (struct window_t*)create_app(syslog_task,"Syslog");

  syslog = 0;
}
