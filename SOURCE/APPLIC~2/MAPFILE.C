/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 2000 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/************************************************************************/
/* File       : applications\mapfile.c
/* Last Update: 23.03.1999
/* Version    : alpha
/* Coded by   :
/* Docus      :
/************************************************************************/
#include "newgui.h"
#include "controls.h"
#include "sched.h"
#include "msgbox.h"
#include "error.h"
#include "fcntl.h"

#define PACKED __attribute__((packed))

window_t *win_mapfile;
ctl_listbox_t *listbox_m;

unsigned int mapfile_loaded = 0;
unsigned int mapfile_win = 0;

struct map_entry
{
  unsigned int addr           PACKED;
  unsigned char name[60]      PACKED;
};

struct list *maplist = NULL;                        // map list


unsigned int mapfile_get_function(unsigned int addr, unsigned char *str)
{
  int i=0;
  unsigned int diff = 0xFFFFFFFF;
  unsigned int lastdiff = 0xFFFFFFFF;

  struct map_entry *mi;
  struct map_entry *bestfit=NULL;

  if (!mapfile_loaded) return 0;

  mi=(struct map_entry*)get_item(maplist,i++);

  while (mi!=NULL)
  {
//    msgbox(ID_OK,1,"addr = %x, name = %s,",mi->addr,mi->name);

    if (addr > mi->addr)
    {
      diff = addr - mi->addr;

      if (diff < lastdiff)
      {
//        msgbox(ID_OK,1,"%s",mi->name);
        lastdiff = diff;
        bestfit = mi;
      }
    }

    mi=(struct map_entry*)get_item(maplist,i++);
  }
  if (bestfit == NULL)
    return 0;

  strcpy(str, bestfit->name);
  str[60] = "\0";

  return bestfit->addr;
}

int mapfile_load(unsigned char *name)
{
  struct map_entry *entry;
  unsigned char *buffer;
  int ret = 0;
  int count = 0;
  int maxcount = 0;
  int symbols = 0;
  int handle;


  int size;
  unsigned char str[255];

  handle = sys_open(name, O_RDONLY | O_BINARY);
  if (handle < 0)
  {
     msgbox(ID_OK,1,"Unable to open file");
     return 0;
  }

  size = sys_filelength(handle);

  buffer = (unsigned char*)valloc(100000);

  while (!ret)
  {
    count = sys_read(handle, buffer + maxcount, 512);
    maxcount+=count;
    if (count != 512) ret = 1;
  }

  count = 0;
  while (maxcount>0)
  {
     entry = (struct map_entry*)(buffer + count);
     entry->name[59] = "\0";

     sprintf(str,"0x%00000008X %s", entry->addr, entry->name);
     if (mapfile_win)
       listbox_add(listbox_m, 0, str);

     maplist=(struct list*)add_item(maplist,entry,sizeof(struct map_entry));
     symbols++;

     maxcount -= sizeof(struct map_entry);
     count += sizeof(struct map_entry);
  }

  sys_close(handle);

  return 1;
}


void mapfile_task()
{
  struct ipc_message *m;
  ctl_button_t *but;
  ctl_input_t *ipt;
  ctl_menu_t *menu;
  ctl_tree_t *tree;
  ctl_text_t *txt_file;
  ctl_text_t *txt_map;
  int i=0;

  char buf[256];

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

/*  i = mapfile_load("//hd0a/mapfile.sys");
  if (i)
  {
    mapfile_loaded = 1;
    msgbox(ID_OK,0,"Mapfile loaded");
  }
  else*/
  {
    win_mapfile = create_window("Mapfile loader",50,50,500,400,WF_STANDARD);
    mapfile_win = 1;

    txt_file = create_text(win_mapfile,20,30,100,25,"Mapfile:",TEXT_ALIGN_LEFT,0);
    ipt = create_input(win_mapfile,120,30,200,20,1);

    SetItemText_input(win_mapfile, ipt, "//hd0a/mapfile.sys");

    but = create_button(win_mapfile,20,80,100,20,"Load",2);

    txt_map = create_text(win_mapfile,20,120,100,25,"Symbols:",TEXT_ALIGN_LEFT,0);
    listbox_m = create_listbox(win_mapfile, 20, 140, 200, 10, 3);
  }

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m->MSGT_GUI_CTL_ID == 2)
            {
               GetItemText_input(win_mapfile, ipt, buf);
               i = mapfile_load(buf);

               if (i)
               {
                 mapfile_loaded = 1;
                 msgbox(ID_OK,1,"Mapfile loaded");
//                 destroy_window(win_mapfile);
                 continue;
               }
               else
                 msgbox(ID_OK,1,"Mapfile error");
            }

            break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_window(win_mapfile);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_mapfile, WF_STOP_OUTPUT, 0);
            set_wflag(win_mapfile, WF_MSG_GUI_REDRAW, 0);
            break;
     }

  }

}

void mapfile_init_app()
{
  if (!valid_app(win_mapfile))
    CreateKernelTask(mapfile_task,"mapfile",NP,0);
}

