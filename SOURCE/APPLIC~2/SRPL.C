/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\srpl.c
/* Last Update: 17.06.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
#include "module.h"
#include "newgui.h"
#include "sched.h"
#include "rtc.h"
#include "fcntl.h"
#include "controls.h"

#include <stdarg.h>

#define ROWS  30

#define MSG_USER 1000

static char content[ROWS][256] = {0};

static window_t *win_srpl = 0;
static window_t *win_srpl_execute = 0;

static struct task_struct *programm_task = 0;

static int cursor_x = 0;
static int row = 0;


#define ID_NEW 1
#define ID_EXECUTE 2

void srpl_prepare_for_execute(void);

/***********************************************************************
/* Redraws the programm commands
/***********************************************************************/
static void srpl_redraw(void)
{
  int i;

  for (i=0;i<ROWS;i++)
    if (content[i][0] != '\0')
      win_outs(win_srpl,10,10+i*13,content[i],COLOR_BLACK,255);
}

/***********************************************************************
/* Clears all commands
/***********************************************************************/
static void srpl_init(void)
{
   int i;

   for (i=0;i<ROWS;i++)
   {
     memset(content[i],0,256);
   }
   cursor_x = row = 0;
}

/***********************************************************************
/* Adds a char to a command
/***********************************************************************/
static void add_char(char ch)
{
  int i;

  for (i=0;i<256;i++)
  {
    if (content[row][i] == '\0')
    {
      content[row][i] = ch;
      content[row][i+1] = '\0';
      break;
    }
  }
}

/***********************************************************************
/* Clears a char from a command
/***********************************************************************/
static void backspace(void)
{
  int i;

  for (i=0;i<256;i++)
  {
    if (content[row][i] == '\0')
    {
      if (i>0)
      {
        win_outs(win_srpl,10,10+(row)*13,content[row],COLOR_LIGHTGRAY, 255);
        content[row][i-1] = '\0';
        win_outs(win_srpl,10,10+(row)*13,content[row],COLOR_BLACK, 255);
      }
      else
      {
        if (row>0)
        {
          row--;
        }
      }
      break;
    }
  }
}

/***********************************************************************
/* Next line for a commmand
/***********************************************************************/
static void enter(void)
{
   if (row < (ROWS-1))
     row++;
}

/***********************************************************************
/* Keypressed in command window
/***********************************************************************/
static void srpl_keypressed(unsigned char ch)
{
  switch (ch)
  {
    case '\n':
    case 13: enter();
             break;
    case  8: backspace();
             break;
    default: add_char(ch);
             cursor_x++;
             break;
  }

  srpl_redraw();
}

/***********************************************************************
/* Command TASK
/***********************************************************************/
static void srpl_task()
{
  struct ipc_message m;
  char str[256];
  int n=0;
  ctl_button_t *but_new, *but_execute;

  win_srpl = create_window("SKY Runtime Programming Language",60,60,900,600,WF_STANDARD);

  srpl_init();

  but_new =     create_button(win_srpl,10,570,70,25,"New",ID_NEW);
  but_execute = create_button(win_srpl,300,570,70,25,"Execute",ID_EXECUTE);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_READ_CHAR_REPLY:
            srpl_keypressed(m.MSGT_READ_CHAR_CHAR);
            break;
       case MSG_GUI_WINDOW_CREATED:
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_srpl);
            win_srpl = 0;
            exit(0);
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:
            switch (m.MSGT_GUI_CTL_ID)
            {
               case ID_NEW:
                        srpl_init();
                        srpl_redraw();
                        break;
               case ID_EXECUTE:
                        srpl_prepare_for_execute();
                        break;
            }
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_srpl, WF_STOP_OUTPUT, 0);
            set_wflag(win_srpl, WF_MSG_GUI_REDRAW, 0);
            srpl_redraw();
            break;
     }
  }
}

void srpl_parse_line(int line, unsigned char *str)
{
  unsigned char *p;
  unsigned int i;

  if (!strncmp(str,"print(",6))
  {
     p = str + 6;
     show_msgf(p);
  }
  else if (!strcmp(str,"reboot")) reboot();
  else if (!strcmp(str,"redraw all")) redraw_all();

  else if (!strncmp(str,"memdump ",8))
  {
     p = str + 8;
     i = atoi(p);
     mem_dump(i, 100);
  }
  else show_msgf("Error 0001: syntax error in line %d",line);
}

void srpl_programm(void)
{
 int i = 0;

 for (i=0;i<ROWS;i++)
 {
    if (content[i][0] != '\0')
    {
      srpl_parse_line(i, content[i]);
    }
 }
 exit(0);
 while(1);
}

/***********************************************************************
/* Redraw the programm window
/***********************************************************************/
void srpl_execute_redraw(void)
{
}

/***********************************************************************
/* Programm TASK
/***********************************************************************/
void srpl_execute_task(void)
{
  struct ipc_message m;
  char str[256];
  int n=0;

  win_srpl_execute = create_window("SKY Runtime Programm...",200,200,600,200,WF_STANDARD);

  programm_task = (struct task_struct*)CreateKernelTask(srpl_programm, "srpl_prog", NP,0);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_WINDOW_CREATED:
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            programm_task->state = TASK_DEAD;
            destroy_app(win_srpl_execute);
            win_srpl_execute = 0;
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_srpl, WF_STOP_OUTPUT, 0);
            set_wflag(win_srpl, WF_MSG_GUI_REDRAW, 0);
            srpl_execute_redraw();
            break;
     }
  }
}

/***********************************************************************
/* Prepares to execute a new programm
/***********************************************************************/
void srpl_prepare_for_execute(void)
{
   if (!valid_app(win_srpl_execute))
   {
     CreateKernelTask(srpl_execute_task,"srpl_exec",NP,0);
   }
}


void srpl_init_app()
{
  /* Console Application */
  if (!valid_app(win_srpl))
  {
    CreateKernelTask(srpl_task,"srpl",NP,0);
  }
}
