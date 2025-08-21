/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\debugger.c
/* Last Update: 03.02.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
#include "newgui.h"
#include "sched.h"
#include "controls.h"
#include "types.h"

window_t *win_debugger;

extern unsigned int next_pid;

static ctl_input_t *ipt_selector, *ipt_offset;
static ctl_text_t *txt_selector, *txt_offset;
static ctl_input_t *debug_ipt_cs, *debug_ipt_ds,*debug_ipt_es, *debug_ipt_fs;
static ctl_input_t *debug_ipt_gs, *debug_ipt_ss,*debug_ipt_eax, *debug_ipt_ebx;
static ctl_input_t *debug_ipt_ecx, *debug_ipt_edx,*debug_ipt_eip, *debug_ipt_ebp;
static ctl_input_t *debug_ipt_cr3, *debug_ipt_esp,*debug_ipt_esp0;
static ctl_input_t *debug_txt_cs, *debug_txt_ds,*debug_txt_es, *debug_txt_fs;
static ctl_input_t *debug_txt_gs, *debug_txt_ss,*debug_txt_eax, *debug_txt_ebx;
static ctl_input_t *debug_txt_ecx, *debug_txt_edx,*debug_txt_eip, *debug_txt_ebp;
static ctl_input_t *debug_txt_cr3, *debug_txt_esp,*debug_txt_esp0,*debug_ipt_state;
static ctl_listbox_t *debug_list;





static ctl_menu_t *menu;

static debug_offset = 0x10000;

#define RESID_IPT_SELECTOR      1
#define RESID_IPT_OFFSET        2

#define RESID_IPT_CS            10
#define RESID_IPT_DS            11
#define RESID_IPT_ES            12
#define RESID_IPT_FS            13
#define RESID_IPT_GS            14
#define RESID_IPT_SS            15
#define RESID_IPT_EAX           16
#define RESID_IPT_EBX           17
#define RESID_IPT_ECX           18
#define RESID_IPT_EDX           19
#define RESID_IPT_EIP           20
#define RESID_IPT_EBP           21
#define RESID_IPT_ESP           22
#define RESID_IPT_ESP0          23
#define RESID_IPT_CR3           24



#define RESID_TXT               100
#define RESID_MENU              30
#define RESID_MNU_FILE          31
#define RESID_MIT_EXIT          32

#define RESID_LIST              33
#define RESID_IPT_STATE             34

static int wid;

void debugger_redraw()
{
  unsigned char str[255];
  unsigned int addr;
  IntRegs regs;
  unsigned int i;

  addr = debug_offset;

  win_draw_fill_rect(win_debugger,20,80,310,230,COLOR_LIGHTGRAY);
  for(i = 0; i < 20; i++)
  {
  	 addr = unassemble(0x10000, addr, 0, &regs, str);
    win_outs(win_debugger, 20, 80+i*10, str, COLOR_BLACK, COLOR_LIGHTGRAY);
  }
}

void debugger_task()
{
  struct ipc_message m;
  char buf[256];
  char str[256];
  ctl_check_t *c;
  struct task_struct *t;
  int i;

  win_debugger = create_window("SkyOS - Debugger 1.0",0,0,640,480,WF_STANDARD);

  txt_selector = create_text(win_debugger,20,30,100,25,"Selector:",TEXT_ALIGN_LEFT,RESID_TXT);
  txt_offset   = create_text(win_debugger,20,50,100,25,"Offset  :",TEXT_ALIGN_LEFT,RESID_TXT);

  ipt_selector = create_input(win_debugger,130,30,100,20,RESID_IPT_SELECTOR);
  ipt_offset   = create_input(win_debugger,130,50,100,20,RESID_IPT_OFFSET);

  debug_ipt_cs  = create_input(win_debugger, 470, 80, 100, 14, RESID_IPT_CS);
  debug_ipt_ds  = create_input(win_debugger, 470, 95, 100, 14, RESID_IPT_DS);
  debug_ipt_es  = create_input(win_debugger, 470, 110, 100, 14, RESID_IPT_ES);
  debug_ipt_fs  = create_input(win_debugger, 470, 125, 100, 14, RESID_IPT_FS);
  debug_ipt_gs  = create_input(win_debugger, 470, 140, 100, 14, RESID_IPT_GS);
  debug_ipt_eax  = create_input(win_debugger, 470, 155, 100, 14, RESID_IPT_EAX);
  debug_ipt_ebx  = create_input(win_debugger, 470, 170, 100, 14, RESID_IPT_EBX);
  debug_ipt_ecx  = create_input(win_debugger, 470, 185, 100, 14, RESID_IPT_ECX);
  debug_ipt_edx  = create_input(win_debugger, 470, 200, 100, 14, RESID_IPT_EDX);
  debug_ipt_cr3  = create_input(win_debugger, 470, 215, 100, 14, RESID_IPT_CR3);
  debug_ipt_eip  = create_input(win_debugger, 470, 230, 100, 14, RESID_IPT_EIP);

  debug_ipt_ss  = create_input(win_debugger, 470, 245, 100, 14, RESID_IPT_SS);
  debug_ipt_esp  = create_input(win_debugger, 470, 260, 100, 14, RESID_IPT_ESP);
  debug_ipt_esp0  = create_input(win_debugger, 470, 275, 100, 14, RESID_IPT_ESP0);

  debug_txt_cs  = create_text(win_debugger, 420, 80, 40, 14, "CS:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_ds  = create_text(win_debugger, 420, 95, 40, 14, "DS:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_es  = create_text(win_debugger, 420, 110, 40, 14, "ES:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_fs  = create_text(win_debugger, 420, 125, 40, 14, "FS:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_gs  = create_text(win_debugger, 420, 140, 40, 14, "GS:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_eax  = create_text(win_debugger, 420, 155, 40, 14, "EAX:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_ebx  = create_text(win_debugger, 420, 170, 40, 14, "EBX:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_ecx  = create_text(win_debugger, 420, 185, 40, 14, "ECX:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_edx  = create_text(win_debugger, 420, 200, 40, 14, "EDX:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_cr3  = create_text(win_debugger, 420, 215, 40, 14, "CR3:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_eip  = create_text(win_debugger, 420, 230, 40, 14, "EIP:",TEXT_ALIGN_LEFT, RESID_TXT);

  debug_txt_ss  = create_text(win_debugger, 420, 245, 40, 14, "SS:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_esp  = create_text(win_debugger, 420, 260, 40, 14, "ESP:",TEXT_ALIGN_LEFT, RESID_TXT);
  debug_txt_esp0  = create_text(win_debugger, 420, 275, 40, 14, "ESP0:",TEXT_ALIGN_LEFT, RESID_TXT);

  debug_list =  create_listbox(win_debugger, 20, 340, 120, 5, RESID_LIST);
  debug_ipt_state =  create_input(win_debugger, 470, 340, 100, 14, RESID_IPT_STATE);

  menu = create_menu(win_debugger,RESID_MENU);
  add_menuitem(win_debugger, menu, "File", RESID_MNU_FILE, 0);
  add_menuitem(win_debugger, menu, "Exit", RESID_MIT_EXIT, RESID_MNU_FILE);

  for (i=0;i<=next_pid;i++)
  {
     t = GetTask(i);
     listbox_add(debug_list, i, t->name);
  }

  msgbox(1, "SkyOS - Debugger 1.0. Copyright(c) 2000 Szeleney Robert");
  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_debugger);
            break;
       case MSG_GUI_CTL_MENU_ITEM:
            debugger_redraw();
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:
            debugger_redraw();
            break;
       case MSG_GUI_TEXT_CHANGED:
            switch (m.MSGT_GUI_TEXT_CHANGED_ITEMID)
            {
              case RESID_IPT_OFFSET:
                GetItemText_input(win_debugger, ipt_offset, buf);
                debug_offset = hex2dec(buf);
                debugger_redraw();
                break;
            }
            break;
       case MSG_GUI_LISTBOX_SELECTED:
            switch (m.MSGT_GUI_LISTBOX_ITEMID)
            {
              case RESID_LIST:
                t = GetTask(m.MSGT_GUI_LISTBOX_DATA);
                if (!t) break;
                if (t->state == 1)
                  sprintf(str,"idle");
                if (t->state == 2)
                  sprintf(str,"running");
                if (t->state == 3)
                  sprintf(str,"ready");
                if (t->state == 4)
                  sprintf(str,"stopped");
                if (t->state == 5)
                  sprintf(str,"dead");
                if (t->state == 6)
                  sprintf(str,"created");
                if (t->state == 7)
                  sprintf(str,"waiting");
                if (t->state == 8)
                  sprintf(str,"sleeping");

                SetItemText_input(win_debugger, debug_ipt_state, str);
                debugger_redraw();
                break;
            }
            break;

       case MSG_GUI_REDRAW:
            set_wflag(win_debugger, WF_STOP_OUTPUT, 0);
            set_wflag(win_debugger, WF_MSG_GUI_REDRAW, 0);
            debugger_redraw();
            break;
     }

  }

}

void debugger_init_app()
{
  wid = 0;

  if (!valid_app(win_debugger))
    CreateKernelTask(debugger_task,"debugger",NP,0);
}