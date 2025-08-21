/************************************************************************/
/* Sky Operating System V2
/* Copyright (c)1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\daemon\ftpd\fptd.c
/* Last Update: 26.12.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*  File Transfer Protocol Daemon and FTP Task
/************************************************************************/
#include "newgui.h"
#include "controls.h"
#include "net.h"
#include "socket.h"
#include "sched.h"
#include "error.h"
#include "netdev.h"
#include "module.h"

#define FTP_FAILED     -1
#define FTP_START      0
#define FTP_CONNECTED  1
#define FTP_CONNECTING 2

#define MSG_FTP_CONNECTED 10000

struct task_struct *ftp_connect_task;
struct task_struct *ftp_receive_task;

#define ID_INPUT_IP 10
#define ID_INPUT_PORT 11
#define ID_INPUT_COMMAND 12
#define ID_BUTTON_CONNECT 13

ctl_input_t *input_command;
ctl_input_t *input_ip;
ctl_input_t *input_port;
ctl_button_t *button_connect;

#define FTP_DEBUG 1
struct window_t *ftp_win;
unsigned int ftp_status;

struct list *ftp_content = NULL;
unsigned int ftp_content_index = 0;

extern struct s_net_device *active_device;
extern unsigned int scheduler_debug_wait;

unsigned int ftp_wincount = 0;
extern unsigned int ftp_dest_addr;
extern unsigned int ftp_local_port;

unsigned char ftp_command_line[255] = {0};

int sock;
struct s_sockaddr_in source;

#define win_out(a,b,c,d) win_outs(ftp_win, a, b, c, d, COLOR_LIGHTGRAY)

static unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

void ftp_redraw_command(void)
{
  unsigned char str[255];

  win_out(20, 380, "FTP Command:",COLOR_BLACK);

  if (ftp_status == FTP_CONNECTING)
    sprintf(str,"Status: Connecting to %s.", in_ntoa(ftp_dest_addr));
  else if (ftp_status == FTP_CONNECTED)
    sprintf(str,"Status: Connected to %s.", in_ntoa(ftp_dest_addr));
  else  sprintf(str,"Status: not connected.");
  win_out(20, 420, str,COLOR_BLACK);
}

void ftp_redraw(void)
{
  int i = 0;
  int j = 0;

  unsigned char *li = (unsigned char*)get_item(ftp_content,i++);

  win_draw_fill_rect(ftp_win,20,80,400,370,7);

  while (li!=NULL)
  {
     win_out(20, 70+j*10, li,COLOR_BLACK);
     j++;

     li=(unsigned char*)get_item(ftp_content,i++);
  }
  ftp_redraw_command();
}

void ftpd_init(void)
{
  printk("ftpd.c: Starting FTP Deamon...");

  ftp_content = NULL;
  ftp_content_index = 0;

  //CreateKernelTask((unsigned int)ftpd_task , "ftpd", HP,0);
}

void ftp_parse(unsigned char* buffer, int size)
{
  unsigned char str[255];

  unsigned char *ps;
  unsigned char *pb;
  int i=0;

  pb = buffer;
  ps = str;

  while (size>0)
  {
    while (*pb != 0x0d)
    {
      size--;
      *ps = *pb;
      ps++;
      pb++;
      i++;

      if (i>250)
        alert("File: ftpd.c   Function: ftp_parse()\n\n%s",
              "Buffer overflow while parsing.");

      if (size<0) break;
    }
    size-=2;
    pb+=2;

    *ps = '\0';

#ifdef FTP_DEBUG
    printk("content");
#endif
    ftp_content_index++;
    if (ftp_content_index > 10)
    {
        ftp_content=(struct list*)del_item(ftp_content,0);
        ftp_content_index--;
    }

    ftp_content= (struct list*)add_item(ftp_content, str, 255);

#ifdef FTP_DEBUG
    printk("content done");
#endif

    ps = str;
    i = 0;
  }
  ftp_redraw();
}

void ftp_receive(void)
{
   int ret;
   unsigned char buffer[1024]={0};

   ret = 0;

   while (1)
   {
     memset(buffer,0, 1024);

     ret = sys_recvfrom(sock, buffer, 1024, &source);

     ftp_parse(buffer, ret);
   }
}

void ftp_connect(void)
{
        int ret;
        struct s_sockaddr_in saddr;
        unsigned char buffer[1024];
        unsigned char str[255];
        int j;

        ftp_status = FTP_CONNECTING;

        GetItemText_input(ftp_win, input_ip, str);
        ftp_dest_addr = in_aton(str);

        GetItemText_input(ftp_win, input_port, str);
        ftp_local_port = atoi(str);

        ret = sys_socket(AF_INET, SOCK_STREAM, 0);

        if (ret < 0)
        {
          alert("File: inet.c   Function: tftp()\n\n%s",
           "Open socket failed.");
          pass_msg(ftp_win, MSG_GUI_WINDOW_CLOSE);
          exit(0);
        }

        sock = ret;

#ifdef FTP_DEBUG
        printk("sock = %d",sock);
#endif

        saddr.sa_family = AF_INET;
        saddr.sa_port = htons(ftp_local_port);
        saddr.sa_addr = active_device->pa_addr;

        ret = sys_bind(sock, &saddr, sizeof(struct s_sockaddr_in));
        if (ret < 0)
        {
             alert("File: ftpd.c   Function: ftp()\n\n%s",
            "Bind to socket failed.");
            pass_msg(ftp_win, MSG_GUI_WINDOW_CLOSE);
            exit(0);
        }

        source.sa_family = AF_INET;
        source.sa_port = htons(21);
        source.sa_addr = ftp_dest_addr;

        if (sys_connect(sock, &source) == ERROR_TIMEOUT)
        {
             alert("File: ftpd.c   Function: ftp()\n\n%s",
            "Connection reply timed out...");
            ftp_status = FTP_FAILED;
            pass_msg(ftp_win, MSG_GUI_WINDOW_CLOSE);

            exit(0);
        }

        ftp_receive_task = (struct task_struct*)CreateKernelTask(ftp_receive, "ftp receive", NP,0);

        ftp_status = FTP_CONNECTED;

        exit(0);
}

void ftp_task(void)
{
  struct ipc_message *m;
  unsigned int ret;
  int i=0;
  unsigned char ftp_para1[255] = {0};
  unsigned char ftp_para2[255] = {0};

  ftp_win = create_window("SkyOS FTP Client",200,400,500,300,WF_STANDARD);
  input_ip       = create_input(ftp_win, 100, 20, 200, 14, ID_INPUT_IP);
  input_port     = create_input(ftp_win, 100, 40, 200, 14, ID_INPUT_PORT);
  input_command  = create_input(ftp_win, 20, 400, 300, 14, ID_INPUT_COMMAND);
  button_connect  = create_button(ftp_win,350,40,80,25,"Connect",ID_BUTTON_CONNECT);

  SetItemText_input(ftp_win, input_ip,"193.170.156.4");
  SetItemText_input(ftp_win, input_port,"10000");

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            switch (m->MSGT_GUI_CTL_ID)
            {
               case ID_BUTTON_CONNECT:
                      ftp_connect_task = (struct task_struct*)CreateKernelTask(ftp_connect, "ftp connect", NP,0);
                      break;
            }
            break;

       case MSG_MOUSE_BUT1_PRESSED:
            break;

       case MSG_MOUSE_BUT1_RELEASED:
            break;

       case MSG_READ_CHAR:
            if (m->MSGT_READ_CHAR_CHAR == 0x08)
            {
               if (i>0)
               {
                 i--;
                 ftp_command_line[i] = '\0';
                 ftp_redraw_command();
               }
            }
            else if (m->MSGT_READ_CHAR_CHAR == 13)
            {
               parse(ftp_command_line,ftp_para1,0);
               parse(ftp_command_line,ftp_para2,1);

               memset(ftp_command_line,0,255);
               i=0;

               if (!strcmp(ftp_para1,"quit"))
               {
                 pass_msg(ftp_win, MSG_GUI_WINDOW_CLOSE);
               }
            }
            else
            {
               ftp_command_line[i] = m->MSGT_READ_CHAR_CHAR;
               i++;
               ftp_redraw_command();
            }
            break;

       case MSG_GUI_REDRAW:
            set_wflag(ftp_win, WF_STOP_OUTPUT, 0);
            set_wflag(ftp_win, WF_MSG_GUI_REDRAW, 0);
            ftp_redraw();
            break;

       case MSG_GUI_WINDOW_CREATED:
            break;

       case MSG_GUI_WINDOW_CLOSE:
            ftp_connect_task->state = TASK_DEAD;
            ftp_receive_task->state = TASK_DEAD;
            destroy_app(ftp_win);
            break;
     }
  }
}

void ftp(void)
{
  ftp_local_port++;
  memset(ftp_command_line, 0, 255);

  ftp_status = FTP_START;

  CreateKernelTask(ftp_task,"ftpclt",NP,0);

}
#if (MODULE_FTP == 1)
void init(void)
{
  ftp_dest_addr = in_aton("193.170.156.1");
  ftp();
  while(1);
}
#endif
