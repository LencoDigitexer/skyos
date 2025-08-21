/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 2000 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/************************************************************************/
/* File       : applications\skyfm.c
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
#include "vfs.h"

window_t *win_fm;

#define RESID_OPEN  1
#define RESID_FILES 3

int fm_update(unsigned char *buf, ctl_listbox_t *listbox)
{
  int i=0;
  int f=-1;
  unsigned int starttime;
  unsigned int endtime;
  struct inode i_dir;
  struct inode i_file;
  int ret = -1;
  unsigned char name[255];

  ret = sys_find(buf,&i_dir);
  if (ret == SUCCESS)
  {
      ret = SUCCESS;

      listbox_del_all(listbox);

//    listbox->enabled = 0;
      i = 0;

      while (ret==SUCCESS)
      {
           ret = sys_get_inode(&i_dir, &i_file, i);
           memset(name,' ',255);
           memcpy(name, i_file.u.fat_dir_entry.name,8);
           memcpy(name+14,i_file.u.fat_dir_entry.ext,3);
           if (i_file.u.fat_dir_entry.attr & ATTR_DIR)
              memcpy(name+20,"<folder>\0",9);
           else
              name[17] = '\0';

           listbox_add(listbox, 0, name);
           i++;
      }

//    listbox->enabled = 1;
//    listbox_add(listbox, 0, " ");
  }
  return ret;
}

void fm_task()
{
  struct ipc_message *m;
  ctl_text_t *txt_file;
  ctl_button_t *but;
  ctl_input_t *ipt;
  ctl_menu_t *menu;
  ctl_listbox_t *listbox;
  int ret;


  char buf[1024];

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_fm = create_window("Sky FileManager V0.1a",20,20,600,420,WF_STANDARD);

  txt_file = create_text(win_fm,20,30,60,25,"Folder:",TEXT_ALIGN_LEFT,0);
  ipt = create_input(win_fm,80,30,400,20,1);

  but = create_button(win_fm,520,30,60,20,"Open",RESID_OPEN);

  listbox = create_listbox(win_fm, 20, 70, 560, 30, RESID_FILES);

  menu = create_menu(win_fm,4);
  add_menuitem(win_fm, menu, "Action", 10, 0);
  add_menuitem(win_fm, menu, "Exit", 100, 10);

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_MENU_ITEM:
            if (m->MSGT_GUI_CTL_ID == 100)
              destroy_app(win_fm);
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m->MSGT_GUI_CTL_ID == RESID_OPEN)
            {
               unsigned char name[255];

               GetItemText_input(win_fm,ipt,buf);
               ret = fm_update(buf, listbox);
               if (ret ==-1) msgbox(ID_OK,MODAL,"Folder doesn't exist!");
            }
            break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_fm);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_fm, WF_STOP_OUTPUT, 0);
            set_wflag(win_fm, WF_MSG_GUI_REDRAW, 0);
            break;
     }

  }

}

void fm_init_app()
{
  if (!valid_app(win_fm))
    CreateKernelTask(fm_task,"fileman",NP,0);

}
