/************************************************************************/
/* Sky Operating System V2
/* Copyright (c)1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\daemon\ftpd\fptd.c
/* Last Update: 21.12.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*  File Transfer Protocol Daemon
/************************************************************************/
#include "net.h"
#include "socket.h"
#include "sched.h"
#include "error.h"
#include "netdev.h"
#include "list.h"
#include "newgui.h"

// #define FTP_DEBUG 1
struct window_t *ftp_win;

struct list *ftp_content = NULL;
unsigned int ftp_content_index = 0;

extern struct s_net_device *active_device;
extern unsigned int scheduler_debug_wait;

unsigned int ftp_wincount = 0;
unsigned int ftp_dest_addr;
unsigned int ftp_local_port;

static unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

void ftp_redraw(void)
{
  int i = 0;
  int j = 0;

  unsigned char *li = (unsigned char*)get_item(ftp_content,i++);

  redraw_window(ftp_win);


  while (li!=NULL)
  {
     win_outs(ftp_win, 10, 40+j*10, 8,li);
     j++;

     li=(unsigned char*)get_item(ftp_content,i++);
  }
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

    ftp_redraw();
#ifdef FTP_DEBUG
    printk("content done");
#endif

    ps = str;
    i = 0;
  }
}

void ftp_task(void)
{
        int sock;
        int ret;
        struct s_sockaddr_in saddr;
        struct s_sockaddr_in source;
        unsigned char buffer[1024];
        unsigned char str[255];
        int j;

        ret = sys_socket(AF_INET, SOCK_STREAM, 0);

        if (ret < 0)
        {
          alert("File: inet.c   Function: tftp()\n\n%s",
           "Open socket failed.");
          destroy_window(ftp_win);
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
            destroy_window(ftp_win);
            exit(0);
        }

        source.sa_family = AF_INET;
        source.sa_port = htons(21);
        source.sa_addr = ftp_dest_addr;

        if (sys_connect(sock, &source) == ERROR_TIMEOUT)
        {
             alert("File: ftpd.c   Function: ftp()\n\n%s",
            "Connection reply timed out...");
            destroy_window(ftp_win);
            exit(0);
        }

#ifdef FTP_DEBUG
        printk("Connected to %s",in_ntoa(source.sa_addr));
#endif

        while (1)
        {
          memset(buffer,0, 512);

          ret = sys_recvfrom(sock, buffer, 512, &source);

          ftp_parse(buffer, ret);
        }
}

void ftp(unsigned char *dest, unsigned char* port)
{
  ftp_dest_addr = in_aton(dest);
  ftp_local_port = atoi(port);

  ftp_win = (struct window_t*)create_app(ftp_task,"ftp", 100, 100, 700, 300);
}

