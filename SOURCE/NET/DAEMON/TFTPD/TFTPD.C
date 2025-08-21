/************************************************************************/
/* Sky Operating System V2
/* Copyright (c)1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\daemon\tftpd\tfptd.c
/* Last Update: 15.12.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*  Trivial File Transfer Daemon
/************************************************************************/
#include "net.h"
#include "socket.h"
#include "sched.h"

#define NULL (void*)0

#define TFTP_RRQ   1
#define TFTP_WRQ   2
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TFTP_ERROR 5

//#define TFTPD_DEBUG 1

unsigned int tftpd_monitor_debug = 1;
struct twm_window *tftpd_monitor = NULL;

static unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

struct s_tftp_rw
{
  unsigned short opcode;
  unsigned short block;
};

struct s_tftp_data
{
  unsigned short opcode;
  unsigned short block;
};

struct s_tftp_connection
{
  unsigned char* filename;
  unsigned char* buffer;
  unsigned int size;
};

struct s_tftp_connection *connection;

void tftpd_debug(unsigned char* str)
{
  if (tftpd_monitor_debug)
    out_window(tftpd_monitor, str);
}

void tftpd_ack(int sock, struct s_sockaddr_in *source, unsigned int block)
{
  unsigned char buffer[20]={0};
  unsigned char str[255];

  struct s_tftp_data opcode;

  opcode.opcode = htons(TFTP_ACK);
  opcode.block  = htons(block);

  sys_sendto(sock, &opcode, 4, source);

#ifdef TFTPD_DEBUG
  sprintf(str, "<4> Sending ACK for block %d to %s/%d",htons(block), in_ntoa(source->sa_addr),htons(source->sa_port));
  tftpd_debug(str);
#endif
}


void tftpd_data(unsigned char*buffer, int size, int sock, struct s_sockaddr_in* source)
{
  unsigned char str[255];
  struct s_tftp_data *opcode;

#ifdef TFTPD_DEBUG
  sprintf(str,"(3) Data received from %s at port %d, %d bytes",
     in_ntoa(source->sa_addr),htons(source->sa_port),size);

  tftpd_debug(str);
#endif

  memcpy(((connection->buffer)+connection->size), buffer+4, size-4);
  connection->size += size;

  opcode = (struct s_tftp_data*)buffer;

  tftpd_ack(sock, source, htons(opcode->block));

  if (size < 512)
  {
#ifdef TFTPD_DEBUG
    sprintf(str,"(3) Transmission finished. Closing connection...");
    tftpd_debug(str);
#endif

    if (tftpd_monitor_debug)
      mem_dump(connection->buffer, connection->size);

    if (connection->filename != NULL)
      vfree(connection->filename);
    if (connection->buffer != NULL)
      vfree(connection->buffer);
    if (connection != NULL)
      vfree(connection);
  }
}

void tftpd_task(void)
{
   int sock;
   int ret;
   struct s_sockaddr_in saddr;
   struct s_sockaddr_in source;
   unsigned char buffer[512];
   unsigned char str[512];
   struct s_tftp_data *opcode;
   unsigned char *p;

   ret = sys_socket(AF_INET, SOCK_DGRAM, 0);

   if (ret < 0)
   {
     alert("File: tftpd.c   Function: tftpd_task()\n\n%s",
           "Open socket failed.");
     exit(0);
   }

   sock = ret;

   saddr.sa_family = AF_INET;
   saddr.sa_port = htons(69);   // 69 == TFTP port
   saddr.sa_addr = in_aton("0.0.0.0"); // accept any

   ret = sys_bind(sock, &saddr, sizeof(struct s_sockaddr_in));
   if (ret < 0)
   {
     alert("File: tftpd.c   Function: tftpd_task()\n\n%s",
           "Bind to socket failed.");
     exit(0);
   }

   tftpd_debug("<1> Socket created and binded to port 69");

   while(1)
   {
     memset(buffer,0,512);
     ret = sys_recvfrom(sock, buffer, 512, &source);

     opcode = (struct s_tftp_data*)buffer;

     if (htons(opcode->opcode) == TFTP_DATA)
     {
        tftpd_data(buffer, ret, sock, &source);
     }
     else
     {
        if (htons(opcode->opcode) == TFTP_RRQ)
        {
#ifdef TFTPD_DEBUG
          sprintf(str,"(2) RRQ opcode for file %s",buffer+2);
          tftpd_debug(str);
#endif
        }

        if (htons(opcode->opcode) == TFTP_WRQ)
        {
#ifdef TFTPD_DEBUG
          sprintf(str,"(2) WRQ opcode for file %s",buffer+2);
          tftpd_debug(str);
#endif
        }

        // establish connection to remote host
        p = (unsigned char*)(buffer+2);
        connection = (struct s_tftp_connection*)valloc(sizeof (struct s_tftp_connection));

        connection->filename = (unsigned char*)valloc(strlen(p)+1);
        memcpy(connection->filename, p, strlen(p));

        connection->buffer = (unsigned char*)valloc(1024*1024);
        connection->size = 0;

        opcode->opcode = htons(TFTP_ACK);
        opcode->block  = htons(0);

        sys_sendto(sock, buffer, 4, &source);
#ifdef TFTPD_DEBUG
        sprintf(str, "<2> Connection to %s at port %d established",in_ntoa(source.sa_addr),htons(source.sa_port));
        tftpd_debug(str);
#endif
     }
   }
}

unsigned char* tftp(unsigned int addr, unsigned char* file)
{
        int sock;
        int ret;
        struct s_sockaddr_in saddr;
        struct s_sockaddr_in source;
        unsigned char buffer[512];

        ret = sys_socket(AF_INET, SOCK_DGRAM, 0);

        if (ret < 0)
        {
          alert("File: inet.c   Function: tftp()\n\n%s",
           "Open socket failed.");
        }

        sock = ret;

        saddr.sa_family = AF_INET;
        saddr.sa_port = htons(2000);
        saddr.sa_addr = in_aton("0.0.0.0");

        ret = sys_bind(sock, &saddr, sizeof(struct s_sockaddr_in));
        if (ret < 0)
        {
             alert("File: inet.c   Function: tftpd()\n\n%s",
            "Bind to socket failed.");
        }

        source.sa_family = AF_INET;
        source.sa_port = htons(69);
        source.sa_addr = addr;

        buffer[0] = 0x00;
        buffer[1] = 0x01;
        memcpy(buffer+2,"test\0",5);
        memcpy(buffer+7,"netascii\0",9);

        sys_sendto(sock, buffer, 16, &source);
        ret = sys_recvfrom(sock, buffer, 512, &source);

        mem_dump_char(buffer,ret);
}

void tftpd_init(void)
{
  printk("tftpd.c: Starting TFTP Deamon...");

//  CreateKernelTask((unsigned int)tftpd_task , "tftpd", HP,0);

/*  tftpd_monitor = (struct twm_window*)valloc(sizeof(struct twm_window));

  tftpd_monitor->x = 520;
  tftpd_monitor->y = 180;
  tftpd_monitor->length = 500;
  tftpd_monitor->heigth = 200;
  tftpd_monitor->acty = 10;
  strcpy(tftpd_monitor->title, "tftpd Connection Monitor");
  draw_window(tftpd_monitor);*/
}

