/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\memman.c
/* Last Update: 27.01.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the SKY memory manager.
/************************************************************************/
#include "newgui.h"
#include "sched.h"
#include "controls.h"

#define ID_PD_NEXT 1
#define ID_PD_PREV 2
#define ID_PT_NEXT 3
#define ID_PT_PREV 4
#define ID_QUIT 5

window_t *win_memman;

unsigned int kernel_page_dir = 0x1000;
unsigned int kernel_page_dir_off = 0;

unsigned int page_table = 0;
unsigned int page_table_off = 0;

unsigned int page_table_virtual = 0;
unsigned int page_table_stat = 0;

unsigned int page = 0;
unsigned int page_stat = 0;

void redraw_memman(void)
{
  unsigned int *pgt_dir = kernel_page_dir;
  unsigned int *pi;
  unsigned char str[255];
  int i;

  sprintf(str,"Page Directory 0x%00000008X", kernel_page_dir);
  win_outs(win_memman, 100, 10, str, COLOR_BLACK, COLOR_LIGHTGRAY);
  sprintf(str,"Page Table 0x%00000008X", page_table);
  win_outs(win_memman, 400, 10, str, COLOR_BLACK, COLOR_LIGHTGRAY);

  for (i=0;i<=20;i++)
  {
   sprintf(str,"0x%00000008X: 0x%00000008X",
     i * 4096 * 1024 + kernel_page_dir_off * 4096 * 1024,
     (unsigned int)*(pgt_dir + kernel_page_dir_off + i));

   win_outs(win_memman,100, 30 + i * 10 ,str,COLOR_BLACK, COLOR_LIGHTGRAY);
  }

  pi = page_table;

  for (i=0;i<=20;i++)
  {
   sprintf(str,"0x%00000008X: 0x%00000008X",
     page_table_off * 4096 + i * 4096 + page_table_virtual,
     (unsigned int)*(pi + page_table_off + i));

   win_outs(win_memman,400, 30 + i * 10 ,str,COLOR_BLACK, COLOR_LIGHTGRAY);
  }

  if (page_table_stat & 1)
    win_outs(win_memman,100, 250 ,"Present : YES",COLOR_BLACK, COLOR_LIGHTGRAY);
  else
    win_outs(win_memman,100, 250 ,"Present : NO ",COLOR_BLACK, COLOR_LIGHTGRAY);

  if (page_table_stat & 32)
    win_outs(win_memman,100, 260 ,"Accessed: YES",COLOR_BLACK, COLOR_LIGHTGRAY);
  else
    win_outs(win_memman,100, 260 ,"Accessed: NO ",COLOR_BLACK, COLOR_LIGHTGRAY);

  if (page_table_stat & 64)
    win_outs(win_memman,100, 270 ,"Dirty   : YES",COLOR_BLACK, COLOR_LIGHTGRAY);
  else
    win_outs(win_memman,100, 270 ,"Dirty   : NO ",COLOR_BLACK, COLOR_LIGHTGRAY);

  if (page_stat & 1)
    win_outs(win_memman,400, 250 ,"Present : YES",COLOR_BLACK, COLOR_LIGHTGRAY);
  else
    win_outs(win_memman,400, 250 ,"Present : NO ",COLOR_BLACK, COLOR_LIGHTGRAY);

  if (page_stat & 32)
    win_outs(win_memman,400, 260 ,"Accessed: YES",COLOR_BLACK, COLOR_LIGHTGRAY);
  else
    win_outs(win_memman,400, 260 ,"Accessed: NO ",COLOR_BLACK, COLOR_LIGHTGRAY);

  if (page_stat & 64)
    win_outs(win_memman,400, 270 ,"Dirty   : YES",COLOR_BLACK, COLOR_LIGHTGRAY);
  else
    win_outs(win_memman,400, 270 ,"Dirty   : NO ",COLOR_BLACK, COLOR_LIGHTGRAY);

}

void task_memman()
{
  struct ipc_message m;
  unsigned int ret;
  int i,j=0,n=0;
  ctl_button_t *but1, *but2, *but3, *but4, *but5;
  unsigned int *pi;

  win_memman = create_window("SKY Memory Manager",100,100,900,600,WF_STANDARD);

  /* !!! id should be > 0  !!! */
  but1 = create_button(win_memman,10,30,70,25,"Prev",ID_PD_PREV);
  but2 = create_button(win_memman,10,230,70,25,"Next",ID_PD_NEXT);
  but3 = create_button(win_memman,650,30,70,25,"Prev",ID_PT_PREV);
  but4 = create_button(win_memman,650,230,70,25,"Next",ID_PT_NEXT);
  but5 = create_button(win_memman,365,450,70,25,"Quit",ID_QUIT);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_MOUSE_BUT1_PRESSED:
               // Check for click in page directory table
               if ((m.MSGT_MOUSE_X > 100) && (m.MSGT_MOUSE_X < 300) &&
                   (m.MSGT_MOUSE_Y > 30) && (m.MSGT_MOUSE_Y < 240))
               {
                 i = (m.MSGT_MOUSE_Y - 30) / 10;

                 page_table_virtual = i * 4096 * 1024 + kernel_page_dir_off * 4096 * 1024;

                 pi = (unsigned int*)(kernel_page_dir + kernel_page_dir_off*4 + i*4);
                 page_table = (unsigned int)*pi;

                 page_table_stat = page_table & 255;

                 page_table = page_table | 255;
                 page_table = page_table ^ 255;


                 set_wflag(win_memman, WF_STOP_OUTPUT, 0);
                 set_wflag(win_memman, WF_MSG_GUI_REDRAW, 0);
                 redraw_memman();
			   }
               
               // Check for click in page table
               if ((m.MSGT_MOUSE_X > 400) && (m.MSGT_MOUSE_X < 600) &&
                   (m.MSGT_MOUSE_Y > 30) && (m.MSGT_MOUSE_Y < 240))
               {
                 i = (m.MSGT_MOUSE_Y - 30) / 10;

                 pi = (unsigned int*)(page_table + page_table_off*4 + i*4);
                  page = (unsigned int)*pi;

                 page_stat = page_stat & 255;

                 set_wflag(win_memman, WF_STOP_OUTPUT, 0);
                 set_wflag(win_memman, WF_MSG_GUI_REDRAW, 0);
                 redraw_memman();
               }
            break;
       case MSG_GUI_WINDOW_CREATED:
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_memman);
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:
            switch (m.MSGT_GUI_CTL_ID)
            {
               case ID_PD_NEXT:
                        kernel_page_dir_off += 20;
                        redraw_memman();
                        break;
               case ID_PD_PREV:
                        kernel_page_dir_off -= 20;
                        redraw_memman();
                        break;
               case ID_PT_NEXT:
                        page_table_off += 20;
                        redraw_memman();
                        break;
               case ID_PT_PREV:
                        page_table_off -= 20;
                        redraw_memman();
                        break;
               case ID_QUIT:
                        destroy_app(win_memman);
                        break;
            }
            break;

       case MSG_GUI_REDRAW:
            set_wflag(win_memman, WF_STOP_OUTPUT, 0);
            set_wflag(win_memman, WF_MSG_GUI_REDRAW, 0);
            redraw_memman();
            break;
     }
  }
}

void app_memman()
{
  if (!valid_app(win_memman))
  {
    CreateKernelTask(task_memman,"memman",NP,0);
  }
}

