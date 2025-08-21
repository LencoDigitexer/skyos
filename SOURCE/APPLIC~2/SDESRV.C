/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applic~2\sdesrv.c
/* Last Update: 07.10.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Server
/************************************************************************/
#include "newgui.h"
#include "controls.h"
#include "sched.h"
#include "net.h"
#include "socket.h"

static ctl_text_t *txt_port;
static ctl_text_t *txt_info_2;
static ctl_text_t *txt_info_3;

static ctl_input_t *ipt_port;
static ctl_input_t *ipt_data;

static ctl_button_t *but_connect;
static ctl_button_t *but_exit;

static window_t *win_sdesrv;

#define ID_PORT 10
#define ID_CONNECT 100
#define ID_EXIT    101

static unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

void sdesrv_layout(void)
{
  char str[255];

  txt_port = create_text(win_sdesrv,10,10,80,12,"Port:",TEXT_ALIGN_LEFT,1);

  ipt_port = create_input(win_sdesrv,100,10,80,10,ID_PORT);

  but_connect  = create_button(win_sdesrv, 10, 130, 70, 18, "Start", ID_CONNECT);
  but_exit     = create_button(win_sdesrv, 100, 130, 70, 18, "Exit", ID_EXIT);
}

void sdesrv_start(void)
{
   int sock;
   int ret;
   struct s_sockaddr_in saddr;
   struct s_sockaddr_in source;
   unsigned char buffer[512];
   unsigned char str[512];

   ret = sys_socket(AF_INET, SOCK_DGRAM, 0);

   if (ret < 0)
   {
     show_msgf("sdesrv.c: Failed to open socket!");
     return 0;
   }

   sock = ret;

   GetItemText_input(win_sdesrv, ipt_port, str);

   saddr.sa_family = AF_INET;
   saddr.sa_port = htons(atoi(str));
   saddr.sa_addr = in_aton("0.0.0.0"); // accept any

   ret = sys_bind(sock, &saddr, sizeof(struct s_sockaddr_in));
   if (ret < 0)
   {
     show_msgf("sdesrv.c: Bind FAILED");
     return 0;
   }

   show_msgf("sdesrv.c: Socket created and binded to port %d",atoi(str));

   return 1;
}


void sdesrv_shutdown(void)
{
    destroy_app(win_sdesrv);
}

void sdesrv_task()
{
  struct ipc_message *m;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_sdesrv = create_window("SkyOS DE-Server",100,100,300,200,WF_STANDARD);

  sdesrv_layout();

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m->MSGT_GUI_CTL_ID == ID_EXIT)
              sdesrv_shutdown();
            if (m->MSGT_GUI_CTL_ID == ID_CONNECT)
              sdesrv_start();
            break;

       case MSG_GUI_WINDOW_CLOSE:
            sdesrv_shutdown();
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_sdesrv, WF_STOP_OUTPUT, 0);
            set_wflag(win_sdesrv, WF_MSG_GUI_REDRAW, 0);
            break;
     }
  }
}

void sdesrv_init_app(void)
{
  if (!valid_app(win_sdesrv))
   CreateKernelTask(sdesrv_task,"sdesrv",NP,0);

}
