/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\ctldemo.c
/* Last Update: 23.03.1999
/* Version    : alpha
/* Coded by   : Gregory Hayrapetian
/* Docus      :
/************************************************************************/

#include "newgui.h"
#include "controls.h"
#include "sched.h"

window_t *win_ctldemo;


void ctldemo_task()
{
  struct ipc_message *m;
  ctl_button_t *but;
  ctl_input_t *ipt;
  ctl_menu_t *menu;
  ctl_listbox_t *listbox;
  ctl_tree_t *tree;
  int i=0;

  char buf[256];

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_ctldemo = create_window("Control demo",350,150,400,400,WF_STANDARD);

//  ipt = create_input(win_ctldemo,20,30,60,20,2);

  but = create_button(win_ctldemo,180,80,100,20,"Add",1);
  listbox = create_listbox(win_ctldemo, 20, 80, 120, 5, 3);

  menu = create_menu(win_ctldemo,4);
  add_menuitem(win_ctldemo, menu, "Action", 10, 0);
  add_menuitem(win_ctldemo, menu, "Control", 12, 0);
  add_menuitem(win_ctldemo, menu, "Help", 13, 0);
  add_menuitem(win_ctldemo, menu, "About", 14, 0);

  add_menuitem(win_ctldemo, menu, "Exit", 100, 10);

  add_menuitem(win_ctldemo, menu, "Button", 101, 12);
  add_menuitem(win_ctldemo, menu, "Listbox", 102, 12);


  add_menuitem(win_ctldemo, menu, "Button", 104, 14);
  add_menuitem(win_ctldemo, menu, "Listbox", 105, 14);
  add_menuitem(win_ctldemo, menu, "Button", 104, 14);
  add_menuitem(win_ctldemo, menu, "Listbox", 105, 14);
  add_menuitem(win_ctldemo, menu, "Button", 104, 14);
  add_menuitem(win_ctldemo, menu, "Listbox", 105, 14);
  add_menuitem(win_ctldemo, menu, "Button", 104, 14);
  add_menuitem(win_ctldemo, menu, "Listbox", 105, 14);
  add_menuitem(win_ctldemo, menu, "Button", 104, 14);
  add_menuitem(win_ctldemo, menu, "Listbox", 105, 14);
  add_menuitem(win_ctldemo, menu, "Button", 104, 14);
  add_menuitem(win_ctldemo, menu, "Listbox", 105, 14);

  tree = create_tree(win_ctldemo, 20, 160, 120, 10, 4);


//  SetItemText_input(win_ctldemo, ctl_x, ")

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_MENU_ITEM:
            if (m->MSGT_GUI_CTL_ID == 101)
              destroy_app(win_ctldemo);
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:

            if (m->MSGT_GUI_CTL_ID == 1)
            {
              i++;
              sprintf(buf,"Item %d",i);
              listbox_add(listbox, i, buf);
            }

            break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_ctldemo);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_ctldemo, WF_STOP_OUTPUT, 0);
            set_wflag(win_ctldemo, WF_MSG_GUI_REDRAW, 0);
            break;
     }

  }

}

void ctldemo_init_app()
{
  if (!valid_app(win_ctldemo))
    CreateKernelTask(ctldemo_task,"ctldemo",NP,0);

}


