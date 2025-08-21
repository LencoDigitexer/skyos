/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\msgwin.c
/* Last Update: 07.10.1999
/* Version    : alpha
/* Coded by   : Gregory Hayrapetian
/* Docus      :
/************************************************************************/

#include "newgui.h"
#include "sched.h"

#include <stdarg.h>

#define MSG_ROWS  30


typedef struct out_string
{
  char str[256];

  struct out_string *next;
  struct out_string *prev;
} out_string_t;

typedef struct out_field
{
  out_string_t *outstr_first;
  int active_row;
  int cursor_pos;
  unsigned char cursor_color;
  int pass_char, console_out;

} out_field_t;
void msg_redraw(window_t *win, out_field_t *outfield);

window_t *win_msg=NULL;

void msg_init_all(out_field_t *outfield)
{
  outfield->outstr_first = NULL;
  outfield->active_row = -1;
}

void msg_add_line(window_t *win, out_field_t *outfield, char *str)
{
  out_string_t *ostr = (out_string_t*) valloc(sizeof(out_string_t));
  int i=0;

  ostr->next = ostr->prev = NULL;
  strcpy(ostr->str,str);
  ostr->str[strlen(str)] = '\0';

  if (outfield->outstr_first == NULL)
  {
	ostr->prev = ostr;
    outfield->outstr_first = ostr;
  }
  else
  {
    ostr->prev = outfield->outstr_first->prev;
    outfield->outstr_first->prev->next = ostr;
    outfield->outstr_first->prev = ostr;
  }

  outfield->active_row++;

  ostr = outfield->outstr_first;

  if (outfield->active_row >= MSG_ROWS)
  {

    while (ostr->next)
    {
      win_outs(win,10,10+i*13,ostr->str,COLOR_LIGHTGRAY,255);
      win_outs(win,10,10+i++*13,ostr->next->str,COLOR_BLACK,255);

      ostr = ostr->next;
    }
    
	outfield->active_row = MSG_ROWS-1;
	ostr = outfield->outstr_first;
	outfield->outstr_first = ostr->next;
	outfield->outstr_first->prev = ostr->prev;

	vfree(ostr);
  }
  else
    msg_redraw(win, outfield);

}

void msg_clear_all()
{
}

void msg_redraw(window_t *win, out_field_t *outfield)
{
  out_string_t *ostr = outfield->outstr_first;
  int i=0;

  while (ostr)
  {
    win_outs(win,10,10+i++*13,ostr->str,COLOR_BLACK,255);
    ostr = ostr->next;
  }
}

void show_msg2(char *str)
{
/*
  if (!valid_app(win_msg))
    return;

  msg_add_line(str);
  msg_redraw();
*/
}

int show_msgf(char *fmt, ...)
{
  char buf[255]={0};
  va_list args;

  va_start(args, fmt);
  vsprintf(buf,fmt,args);
  va_end(args);

  show_msg(buf);
}

int show_msg(char *str)
{
  char *mstr;
  struct ipc_message m;


  if (!valid_app(win_msg))
    return -1;

  mstr = (char*) valloc(strlen(str)+1);
  if (mstr == NULL)
    return -2;

  strcpy(mstr,str);

  m.type = MSG_SHOW_MSG;
  m.MSGT_STRING_ADDRESS = mstr;
  send_msg(win_msg->ptask->pid, &m);

  return 0;
}

void react_keypressed(window_t *win, char ch)
{
  struct ipc_message m;

  switch (ch)
  {
    case 'a': win_msg = win;
		      break;
  }
}

void msg_task()
{
  window_t *win_msgwin;
  struct ipc_message m;
  out_field_t outfield;

  win_msgwin = create_window("Message window",200,50,500,450,WF_STANDARD);

  msg_init_all(&outfield);
  win_msg = win_msgwin;

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_READ_CHAR_REPLY:
            react_keypressed(win_msgwin, m.MSGT_READ_CHAR_CHAR);
	    break;
       case MSG_SHOW_MSG:
            msg_add_line(win_msgwin, &outfield,m.MSGT_STRING_ADDRESS);
            vfree(m.MSGT_STRING_ADDRESS);
            msg_redraw(win_msgwin, &outfield);
	    break;
       case MSG_GUI_WINDOW_CREATED:
            msg_add_line(win_msgwin, &outfield,"Message window ready");
            msg_add_line(win_msgwin, &outfield,"Press 'a' to activate window");
            msg_add_line(win_msgwin, &outfield," ");
            msg_redraw(win_msgwin, &outfield);
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_msgwin);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_msgwin, WF_STOP_OUTPUT, 0);
            set_wflag(win_msgwin, WF_MSG_GUI_REDRAW, 0);
            msg_redraw(win_msgwin, &outfield);
            break;
     }

  }

}

void msg_init_app()
{
//  if (!valid_app(win_msg))
    CreateKernelTask(msg_task,"msgwin",NP,0);
}


