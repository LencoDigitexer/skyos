/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\terminal.c
/* Last Update: 08.04.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
#include "uart.h"
#include "newgui.h"
#include "sched.h"
#include "controls.h"

#define RESID_BUT_TERMINAL_DIAL      1
#define RESID_BUT_TERMINAL_OPEN      2

#define RESID_TXT_TERMINAL_PORT     10
#define RESID_TXT_TERMINAL_BAUD     11
#define RESID_TXT_TERMINAL_NUMMER   12
#define RESID_TXT_TERMINAL_DATA     13
#define RESID_TXT_TERMINAL_STATUS   14

#define RESID_IPT_TERMINAL_PORT     20
#define RESID_IPT_TERMINAL_BAUD     21
#define RESID_IPT_TERMINAL_NUMMER   22
#define RESID_IPT_TERMINAL_DATA     23
#define RESID_IPT_TERMINAL_STATUS   24

#define RESID_MENU              30
#define RESID_MNU_FILE          31
#define RESID_MIT_EXIT          32

window_t *win_terminal;

ctl_menu_t *terminal_menu;

ctl_text_t *terminal_txt_port;
ctl_text_t *terminal_txt_baud;
ctl_text_t *terminal_txt_nummer;
ctl_text_t *terminal_txt_data;
ctl_text_t *terminal_txt_status;

ctl_input_t *terminal_ipt_port;
ctl_input_t *terminal_ipt_baud;
ctl_input_t *terminal_ipt_nummer;
ctl_input_t *terminal_ipt_data;
ctl_input_t *terminal_ipt_status;

ctl_button_t *terminal_but_dial;
ctl_button_t *terminal_but_open;

int t_status = 0;

#define T_CLOSE 0
#define T_OPEN  1
#define T_READY 2
#define T_DIALING 3
#define T_CONNECTED 4

void terminal_redraw()
{
}

unsigned int terminal_send(unsigned int port, unsigned char *str)
{
   int i = 0;

   while (str[i] != 0)
   {
     ser_send((port == 1)?SER_COM1:SER_COM2, str[i], 0x8000, 0, 0);
     i++;
   }
   ser_send((port == 1)?SER_COM1:SER_COM2, 13, 0x8000, 0, 0);
   i++;

   return i;
}





void terminal_task()
{
  struct ipc_message m;
  char buf[256];
  int port=0;
  unsigned int baud;
  unsigned int i;

  win_terminal = create_window("Modem Terminal emulation",300,200,500,400,WF_STANDARD);

  terminal_txt_port = create_text(win_terminal,20,30,100,25,"COM Port:",TEXT_ALIGN_LEFT,RESID_TXT_TERMINAL_PORT);
  terminal_txt_baud = create_text(win_terminal,20,60,100,25,"Baud:    ",TEXT_ALIGN_LEFT,RESID_TXT_TERMINAL_BAUD);
  terminal_txt_nummer   = create_text(win_terminal,20,150,100,25,"Nummer:  ",TEXT_ALIGN_LEFT,RESID_TXT_TERMINAL_NUMMER);
  terminal_txt_data     = create_text(win_terminal,20,180,100,25,"Data:    ",TEXT_ALIGN_LEFT,RESID_TXT_TERMINAL_DATA);
  terminal_txt_data     = create_text(win_terminal,20,210,100,25,"Status:  ",TEXT_ALIGN_LEFT,RESID_TXT_TERMINAL_DATA);

  terminal_ipt_port = create_input(win_terminal,130,30,70,20,RESID_IPT_TERMINAL_PORT);
  terminal_ipt_baud = create_input(win_terminal,130,60,70,20,RESID_IPT_TERMINAL_BAUD);
  terminal_ipt_nummer = create_input(win_terminal,130,150,100,20,RESID_IPT_TERMINAL_NUMMER);
  terminal_ipt_data   = create_input(win_terminal,130,180,250,20,RESID_IPT_TERMINAL_DATA);
  terminal_ipt_status = create_input(win_terminal,130,210,250,20,RESID_IPT_TERMINAL_STATUS);

  terminal_but_dial = create_button(win_terminal,20,250,70,20,"Dial",RESID_BUT_TERMINAL_DIAL);
  terminal_but_open = create_button(win_terminal,150,250,70,20,"Open",RESID_BUT_TERMINAL_OPEN);

  terminal_menu = create_menu(win_terminal,RESID_MENU);
  add_menuitem(win_terminal, terminal_menu, "File", RESID_MNU_FILE, 0);
  add_menuitem(win_terminal, terminal_menu, "Exit", RESID_MIT_EXIT, RESID_MNU_FILE);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_terminal);
            break;
       case MSG_GUI_CTL_MENU_ITEM:
            break;
       case MSG_GUI_TEXT_CHANGED:
            switch (m.MSGT_GUI_TEXT_CHANGED_ITEMID)
            {
               case RESID_IPT_TERMINAL_DATA:
                    GetItemText_input(win_terminal, terminal_ipt_data, buf);
                    i = terminal_send(port, buf);
                    sprintf(buf,"%d bytes transmitted to modem.",i);
                    SetItemText_input(win_terminal, terminal_ipt_data, "\0");
                    SetItemText_input(win_terminal, terminal_ipt_status, buf);
               break;
            }
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:
            switch (m.MSGT_GUI_CTL_ID)
            {
               case RESID_BUT_TERMINAL_DIAL:
                    if (t_status != T_OPEN)
                       SetItemText_input(win_terminal, terminal_ipt_status, "Modem not opened.");
                      break;

               case RESID_BUT_TERMINAL_OPEN:
                       GetItemText_input(win_terminal, terminal_ipt_port, buf);

                       port = atoi(buf);
                       if ((port != 1) && (port != 2))
                         SetItemText_input(win_terminal, terminal_ipt_status,"Invalid port number");

                       else
                       {
                         GetItemText_input(win_terminal, terminal_ipt_baud, buf);
                         baud = atoi(buf);

                         if (port==1)
                           ser_init(SER_COM1, baud, SER_LCR_8BITS | SER_LCR_1STOPBIT | SER_LCR_NOPARITY);
                         if (port==2)
                           ser_init(SER_COM2, baud, SER_LCR_8BITS | SER_LCR_1STOPBIT | SER_LCR_NOPARITY);

                         SetItemText_input(win_terminal, terminal_ipt_status,"Initialized.");
                       }
                       break;
            }
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_terminal, WF_STOP_OUTPUT, 0);
            set_wflag(win_terminal, WF_MSG_GUI_REDRAW, 0);
            terminal_redraw();
            break;
     }

  }

}

void terminal_init_app()
{
  if (!valid_app(win_terminal))
    CreateKernelTask(terminal_task,"terminal",NP,0);
}