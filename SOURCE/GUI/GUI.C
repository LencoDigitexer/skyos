/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\gui.c
/* Last Update: 04.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the Graphical User Interface
/************************************************************************/
#include "sched.h"
#include "ccm.h"
#include "twm.h"

int TASK_GUI;
#define NULL (void*)0

void gui_dump(struct task_struct *t)
{
/*  struct twm_window *w;
  struct s_common_object *o;
  struct s_common_list *l;

  w = t->window;

  printk("X: %d  Y: %d  Heigth: %d  Length: %d",w->x, w->y, w->heigth, w->length);
  printk("Title: %s",w->title);
  printk("Common Objects:");
  l = w->common_list;
  while (l)
  {
    printk("  Type: %d", l->object->type);
    printk("  X: %d  Y: %d", l->object->x, l->object->y);
    printk("  Length: %d Heigth: %d", l->object->length, l->object->heigth);
    l = l->next;
  }*/
}

void oldgui_create_window(struct task_struct *task)
{
  struct twm_window *window;

  window = (struct twm_window*)valloc(sizeof(struct twm_window));

  window->x = 100;
  window->length = 100;
  window->y = 100;
  window->heigth = 100;
  window->actx = 5;
  window->acty = 30;

  strcpy(window->title, "unknown");

  window->common_list = (void*)0;

  task->window = window;
}

void gui_add_item(struct twm_window *window, struct s_common_object *object)
{
  struct s_common_list *list;

  if (window->common_list == NULL)             // First Item
  {
    window->common_list = (struct s_common_list *)valloc(sizeof(struct s_common_list));
    window->common_list->object = object;
    window->common_list->next = NULL;
  }
  else                  // Add to tail
  {
    list = window->common_list;
    while (list->next != NULL)
      list = list->next;

    list->next = (struct s_common_list *)valloc(sizeof(struct s_common_list));
    list->next->object = object;
    list->next->next = NULL;
  }
}

void gui_config_window(struct twm_window *window, struct twm_window *newwindow)
{
  window->x = newwindow->x;
  window->length = newwindow->length;
  window->y = newwindow->y;
  window->heigth = newwindow->heigth;
  strcpy(window->title, newwindow->title);
}

void gui_task(void)
{
  struct ipc_message *m;
  struct s_common_list * cl;
  struct s_common_object *co;
  struct twm_window *window;
  int task;
  int ret;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_OLDGUI_CREATE_WINDOW:
          oldgui_create_window(m->MSGT_GUI_CREATE_WINDOW_TASK);
          m->type = MSG_OLDGUI_CREATE_WINDOW_REPLY;
          send_msg(m->sender, m);
          break;

       case MSG_GUI_WINDOW_REDRAW:
          draw_window(m->MSGT_WINDOW);
          m->type = MSG_REPLY;
          send_msg(m->sender, m);
          break;

      case MSG_GUI_WINDOW_CONFIG:
          gui_config_window(m->source->window, m->MSGT_WINDOW);
          m->type = MSG_REPLY;
          send_msg(m->sender, m);
          break;

      case MSG_GUI_ADD_ITEM:
          gui_add_item(m->MSGT_WINDOW, m->MSGT_ITEM);
          m->type = MSG_REPLY;
          send_msg(m->sender, m);
          break;

      case MSG_GUI_DUMP:
          gui_dump(m->MSGT_WINDOW);
          m->type = MSG_REPLY;
          send_msg(m->sender, m);
          break;

      case MSG_GUI_DRAW_STRING:
          out_window(m->MSGT_WINDOW, m->MSGT_GUI_DRAW_STRING_STR);
          m->type = MSG_REPLY;
          send_msg(m->sender, m);
          break;
     }
  }
}

int gui_init(void)
{

  printk("gui.c: Initializing graphical user interface...");
  TASK_GUI = ((struct task_struct*)CreateKernelTask((unsigned int)gui_task, "gui", HP,0))->pid;

  return 0;

}

