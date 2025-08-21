/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 2000 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/************************************************************************/
/* File       : applications\skypad.c
/* Last Update:
/* Version    : alpha
/* Coded by   :
/* Docus      :
/************************************************************************/

#include "newgui.h"
#include "controls.h"
#include "sched.h"
#include "msgbox.h"
#include "fcntl.h"
#include "error.h"

window_t *win_skypad;

extern unsigned int timerticks;


void skypad_task()
{
  struct ipc_message *m;
  ctl_text_t *txt_file;
  ctl_button_t *but;
  ctl_input_t *ipt;
  ctl_menu_t *menu;
  ctl_listbox_t *listbox;
  int i=0;
  int f=-1;
  unsigned int starttime;
  unsigned int endtime;


  char buf[1024];

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_skypad = create_window("SkyPad V0.1a",20,20,600,420,WF_STANDARD);

  txt_file = create_text(win_skypad,20,30,60,25,"File:",TEXT_ALIGN_LEFT,0);
  ipt = create_input(win_skypad,80,30,400,20,1);

  but = create_button(win_skypad,520,30,60,20,"Load",1);

  listbox = create_listbox(win_skypad, 20, 70, 560, 30, 3);

  menu = create_menu(win_skypad,4);
  add_menuitem(win_skypad, menu, "Action", 10, 0);
  add_menuitem(win_skypad, menu, "Exit", 100, 10);

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_MENU_ITEM:
            if (m->MSGT_GUI_CTL_ID == 100)
              destroy_app(win_skypad);
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:

            if (m->MSGT_GUI_CTL_ID == 1)
            {
               GetItemText_input(win_skypad,ipt,buf);

               if (f>=0)
                 sys_close(f);

               f = sys_open(buf,O_RDONLY | O_TEXT);
               if (f < 0)
               {
                  msgbox(ID_OK,1,"Failed to open file");
                  break;
               }

               listbox_del_all(listbox);

               starttime = timerticks;

               listbox->enabled = 0;
               i = 1;

               while (i>0)
               {
                 i =  sys_read(f,buf,1024);
                 if (i!=0) listbox_add(listbox, 0, buf);
               }

               listbox->enabled = 1;
               listbox_add(listbox, 0, " ");
               endtime = timerticks;
               msgbox(ID_OK,MODAL,"Ticks: %d, secs: %d",endtime-starttime, (endtime-starttime)/100);
            }

            break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_skypad);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_skypad, WF_STOP_OUTPUT, 0);
            set_wflag(win_skypad, WF_MSG_GUI_REDRAW, 0);
            break;
     }

  }

}

void skypad_init_app()
{
  if (!valid_app(win_skypad))
    CreateKernelTask(skypad_task,"SkyPad",NP,0);

}

