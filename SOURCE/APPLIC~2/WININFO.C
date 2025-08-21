/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\wininfo.c
/* Last Update: 03.02.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
#include "newgui.h"
#include "sched.h"
#include "controls.h"

#define RESID_BUT_NEXT_WIN      1
#define RESID_BUT_PREV_WIN      2
#define RESID_BUT_NEXT_CTL      3
#define RESID_BUT_PREV_CTL      4

#define RESID_TXT_WID           10
#define RESID_TXT_NAME          11
#define RESID_TXT_PID           12
#define RESID_TXT_CID           13
#define RESID_TXT_CTYPE         14
#define RESID_TXT_DATA          15
#define RESID_TXT_FIELD         16

#define RESID_IPT_WID           20
#define RESID_IPT_NAME          21
#define RESID_IPT_PID           22
#define RESID_IPT_CID           23
#define RESID_IPT_CTYPE         24
#define RESID_IPT_DATA          25
#define RESID_IPT_POSX          26
#define RESID_IPT_POSY          27
#define RESID_IPT_FIEL          28

#define RESID_MENU              30
#define RESID_MNU_FILE          31
#define RESID_MIT_EXIT          32

window_t *win_wininfo;

ctl_button_t *but_next_win, *but_prev_win, *but_next_ctl, *but_prev_ctl;
ctl_text_t *txt_wid, *txt_wname, *txt_pid, *txt_cid, *txt_ctype, *txt_data, *txt_field;
ctl_input_t *ipt_wid, *ipt_wname, *ipt_pid, *ipt_cid, *ipt_ctype, *ipt_data, *ipt_posx, *ipt_posy, *ipt_field;
ctl_menu_t *menu;

int wid;
control_t *current_ctl;

void wininfo_redraw()
{
  char str[256];
  control_t *ctl;
  rect_t *rect;

  sprintf(str,"%d",wid);
  SetItemText_input(win_wininfo, ipt_wid, str);
  SetItemText_input(win_wininfo, ipt_wname, win_list[wid]->name);
  sprintf(str,"%d",win_list[wid]->ptask->pid);
  SetItemText_input(win_wininfo, ipt_pid, str);

  if (current_ctl)
  {
    sprintf(str,"%d",current_ctl->ctl_id);
    SetItemText_input(win_wininfo, ipt_cid, str);

    switch (current_ctl->ctl_type)
    {
      case CTL_BUTTON:
           {
             ctl_button_t *ctl_but = current_ctl->ctl_ptr;

             rect = &ctl_but->rect;
             strcpy(str,"Button");
             SetItemText_input(win_wininfo, ipt_data, ctl_but->name);
           }
           break;
      case CTL_INPUT:
           {
             ctl_input_t *ctl_ipt = current_ctl->ctl_ptr;

             rect = &ctl_ipt->rect;
             strcpy(str,"Input field");
             SetItemText_input(win_wininfo, ipt_data, ctl_ipt->data);
           }
           break;
      case CTL_MENU:
           {
             ctl_menu_t *ctl_menu = current_ctl->ctl_ptr;

             rect = NULL;
             strcpy(str,"Menu");
             SetItemText_input(win_wininfo, ipt_data, " ");
           }
           break;
      case CTL_TEXT:
           {
             ctl_text_t *ctl_text = current_ctl->ctl_ptr;

             rect = &ctl_text->rect;
             strcpy(str,"Text field");
             SetItemText_input(win_wininfo, ipt_data, ctl_text->name);
           }
           break;
      default:
           {
             rect = NULL;
             strcpy(str,"Unknown");
             SetItemText_input(win_wininfo, ipt_data, " ");
           }
           break;
    }

    SetItemText_input(win_wininfo, ipt_ctype, str);

    if (rect)
    {
      sprintf(str,"%d",rect->x1);
      SetItemText_input(win_wininfo, ipt_posx, str);
      sprintf(str,"%d",rect->y1);
      SetItemText_input(win_wininfo, ipt_posy, str);
    }
  }
  else
  {
    SetItemText_input(win_wininfo, ipt_cid, "");
    SetItemText_input(win_wininfo, ipt_ctype, "");
    SetItemText_input(win_wininfo, ipt_data, "");
    SetItemText_input(win_wininfo, ipt_posx, "");
    SetItemText_input(win_wininfo, ipt_posy, "");
  }
}

void wininfo_react(int ctl_id)
{
  switch (ctl_id)
  {
    case RESID_BUT_NEXT_WIN: if (wid < znum-1) wid++;
                             current_ctl = win_list[wid]->controls;
                             break;
    case RESID_BUT_PREV_WIN: if (wid > 0) wid--;
                             current_ctl = win_list[wid]->controls;
                             break;
    case RESID_BUT_NEXT_CTL: if (current_ctl && current_ctl->next) current_ctl = current_ctl->next;
                             break;
    case RESID_BUT_PREV_CTL: if (current_ctl && (current_ctl != win_list[wid]->controls)) current_ctl = current_ctl->prev;
                             break;
    case RESID_MIT_EXIT:     destroy_app(win_wininfo);
                             break;
  }
}


void wininfo_task()
{
  struct ipc_message m;
  char buf[256];

  win_wininfo = create_window("Window Information",300,200,500,400,WF_STANDARD & ~WF_STYLE_BUTCLOSE);

  but_next_win = create_button(win_wininfo,150,150,70,20,"Next",RESID_BUT_NEXT_WIN);
  but_prev_win = create_button(win_wininfo,50,150,70,20,"Prev",RESID_BUT_PREV_WIN);

  but_next_ctl = create_button(win_wininfo,150,280,70,20,"Next",RESID_BUT_NEXT_CTL);
  but_prev_ctl = create_button(win_wininfo,50,280,70,20,"Prev",RESID_BUT_PREV_CTL);


  txt_wid = create_text(win_wininfo,20,30,100,25,"Window id:",TEXT_ALIGN_LEFT,RESID_TXT_WID);
  txt_wname = create_text(win_wininfo,20,60,100,25,"Name:",TEXT_ALIGN_LEFT,RESID_TXT_NAME);
  txt_pid = create_text(win_wininfo,20,90,100,25,"PID:",TEXT_ALIGN_LEFT,RESID_TXT_PID);

  txt_cid = create_text(win_wininfo,20,190,100,25,"Control id:",TEXT_ALIGN_LEFT,RESID_TXT_CID);
  txt_ctype = create_text(win_wininfo,20,220,110,25,"Control type:",TEXT_ALIGN_LEFT,RESID_TXT_CTYPE);
  txt_data = create_text(win_wininfo,180,190,110,25,"Control data:",TEXT_ALIGN_LEFT,RESID_TXT_DATA);
  txt_field = create_text(win_wininfo,240,220,110,25,"Field:",TEXT_ALIGN_LEFT,RESID_TXT_FIELD);


  ipt_wid = create_input(win_wininfo,130,30,50,20,RESID_IPT_WID);
  ipt_wname = create_input(win_wininfo,130,60,200,20,RESID_IPT_NAME);
  ipt_pid = create_input(win_wininfo,130,90,50,20,RESID_IPT_PID);

  ipt_cid = create_input(win_wininfo,130,190,50,20,RESID_IPT_CID);
  ipt_ctype = create_input(win_wininfo,130,220,100,20,RESID_IPT_CTYPE);
  ipt_data = create_input(win_wininfo,300,190,150,20,RESID_IPT_DATA);
  ipt_posx = create_input(win_wininfo,300,220,40,20,RESID_IPT_POSX);
  ipt_posy = create_input(win_wininfo,350,220,40,20,RESID_IPT_POSY);

  menu = create_menu(win_wininfo,RESID_MENU);
  add_menuitem(win_wininfo, menu, "File", RESID_MNU_FILE, 0);
  add_menuitem(win_wininfo, menu, "Exit", RESID_MIT_EXIT, RESID_MNU_FILE);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_wininfo);
            break;
       case MSG_GUI_CTL_MENU_ITEM:
            wininfo_react(m.MSGT_GUI_CTL_ID);
            wininfo_redraw();
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:
            wininfo_react(m.MSGT_GUI_CTL_ID);
            wininfo_redraw();
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_wininfo, WF_STOP_OUTPUT, 0);
            set_wflag(win_wininfo, WF_MSG_GUI_REDRAW, 0);
            wininfo_redraw();
            break;
     }

  }

}

void wininfo_init_app()
{
  wid = 0;
  current_ctl = NULL;

  if (!valid_app(win_wininfo))
    CreateKernelTask(wininfo_task,"wininfo",NP,0);
}
