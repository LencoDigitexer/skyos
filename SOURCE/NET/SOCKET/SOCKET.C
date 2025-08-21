/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 -1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\socket\socket.c
/* Last Update: 11.12.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux
/************************************************************************/
/* Definition:
/*   The socket protocol interface.
/************************************************************************/
#include "error.h"
#include "list.h"
#include "net.h"
#include "netdev.h"
#include "socket.h"
#include "sched.h"

#define NULL (void*)0

#define SOCKET_LISTEN 1
#define SOCKET_RECEIVING 2

int socket_count = 0;
unsigned int socket_nr = 5000;

extern unsigned int timerticks;

struct list *protocols = NULL;            /* list of all installed protocols */

struct list *sockets = NULL;              /* list of open sockets */

/* Window of Socket Monitor */
struct window_t *socket_monitor_win;

struct s_family
{
  int family;
};

static unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

/* register a socket family */
int register_family(unsigned int new_family)
{
  int i=0;
  struct s_family *fi;

  fi=(struct s_family*)get_item(protocols,i++);

  // protocol already registered?
  while (fi!=NULL)
  {
    if (fi->family == new_family) return ERROR_PROTOCOL_REGISTERED;
    fi=(struct s_family*)get_item(protocols,i++);
  }

  fi=(struct s_family*)valloc(sizeof(struct s_family));

  fi->family = new_family;

  protocols = (struct list*)add_item(protocols,fi,sizeof(struct s_family));
  return SUCCESS;
}

/* check if a socketfamily is registered */
int check_family(unsigned int family)
{
  int i=0;
  struct s_family *fi;

  fi=(struct s_family*)get_item(protocols,i++);

  // protocol already registered?
  while (fi!=NULL)
  {
    if (fi->family == family) return SUCCESS;
    fi=(struct s_family*)get_item(protocols,i++);
  }

  return ERROR_PROTOCOL_NOT_REGISTERED;
}

/* Allocate memory for a socket */
struct s_socket* socket_alloc(void)
{
  struct s_socket *socket;

  socket = (struct s_socket*)valloc(sizeof(struct s_socket));
  return socket;
}

/* Register a open socket */
int socket_register(struct s_socket *socket)
{
  socket_count++;

  socket->fd = socket_count;

  sockets = (struct list*)add_item(sockets,socket,sizeof(struct s_socket));

  return socket_count;
}

/* get a socket by his fd */
struct s_socket* socket_get(unsigned int fd)
{
  int i=0;
  struct s_socket *li;

  li=(struct s_socket*)get_item(sockets,i++);

  while (li!=NULL)
  {
    if (li->fd == fd)
      return li;

    li=(struct s_socket*)get_item(sockets,i++);
  }
  return NULL;
}

/* get a socket by his datas */
int socket_get_fd(struct s_sockaddr_in *saddr)
{
  int i=0;
  struct s_socket *li;

  li=(struct s_socket*)get_item(sockets,i++);

  while (li!=NULL)
  {
    if (li->pa_addr == 0)            // accept from any pa_addr
    {
      if ((saddr->sa_family == li->family) &&
          (saddr->sa_port   == li->port  ))
             return li->fd;
    }
    else
    {
      if ((saddr->sa_family == li->family) &&
          (saddr->sa_port   == li->port  ) &&
             (saddr->sa_addr   == li->pa_addr))
             return li->fd;
    }

    li=(struct s_socket*)get_item(sockets,i++);
  }
  return -1;
}

unsigned long int ntohl(unsigned long int x)
{
	__asm__("bswap %0" : "=r" (x) : "0" (x));
	return x;
}

/* Create a socket for a process */
int socket(unsigned int family, unsigned int type, unsigned int protocol, unsigned int pid)
{
  struct s_socket *socket;

  /* check if family is registered */
  if (check_family(family) != SUCCESS)
  {
    alert("File: socket.c  Function: sys_socket\n\n%s",
            "Family not registered.");
    return ERROR_PROTOCOL_NOT_REGISTERED;
  }

  /* check if correct type */
  if ((type != SOCK_STREAM && type != SOCK_DGRAM) || protocol < 0)
  {
    alert("File: socket.c  Function: sys_socket\n\n%s",
            "Invalid Type specificated.");
    return(-1);
  }

  socket = socket_alloc();

  if (socket == NULL)
  {
    alert("File: socket.c  Function: sys_socket\n\n%s\n%s",
            "Out of memory.",
            "Unable to allocate memory for socket.");
    return ERROR_NO_MEM;
  }

  socket->family = family;
  socket->type = type;
  memset(socket->pa_addr, 0, 8);
  socket->port = 0;

  socket->receive_pid = -1;
  socket->owner = pid;

  socket->data = NULL;
  socket->state = 0;

  socket->seq = ntohl(2000000000);

  return (socket_register(socket));
}

int socket_bind(unsigned int sock, struct s_sockaddr_in *saddr, int size)
{
  struct s_socket *socket;

  socket = socket_get(sock);
  if (socket == NULL) return ERROR_FILE_NOT_OPEN;

  socket->pa_addr = saddr->sa_addr;

  if (saddr->sa_port == 0)              // get a portnummer for us
  {
    socket->port = htons(socket_nr);
    socket_nr++;
  }
  else
    socket->port    = saddr->sa_port;

  socket->family  = saddr->sa_family;

  return SUCCESS;
}

int socket_received(struct s_sockaddr_in *saddr, struct s_sockaddr_in *daddr, unsigned char* buffer, unsigned int size)
{
  int sock;
  struct list *data;

  struct s_socket *socket;
  struct s_net_buff *buff;
  struct s_socket_packet *packet;

  unsigned int flags;
  struct ipc_message m;

  sock = socket_get_fd(daddr);

  if (sock == -1)
  {
/*    alert("File: socket.c  Function: socket_received\n\n%s",
          "Requested socket not registered");*/
    return;
  }
  else
  {
/*    alert("File: socket.c  Function: socket_received\n\n%s",
          "Data for a registered socket received!             ");*/

    // Copy received data to socket buffer
    packet = (struct s_socket_packet*)valloc(sizeof(struct s_socket_packet));
    packet->data = (struct s_net_buff*)net_buff_alloc(size);
    memcpy(packet->data->data, buffer, size);

    // Copy received address data to socket buffer
    memcpy(&(packet->saddr), saddr, sizeof(struct s_sockaddr_in));

    socket = socket_get(sock);
    if (!socket)
    {
      alert("File: socket.c  Function: socket_received\n\n%s",
            "Socket not found?!?!");
            return;
    }

    save_flags(flags);
    cli();

    if (socket->state == SOCKET_RECEIVING)
    {
      socket->state = SOCKET_CONNECTED;
      // wake up task
      task_control(socket->receive_pid, TASK_READY);
      m.type = MSG_NET_SOCKET_READ_REPLY;
      m.MSGT_NET_SOCKET_READ_RESULT = size;

      memcpy(socket->buffer, buffer, size);

      send_msg(socket->receive_pid, &m);
    }
    else socket->data = (struct list*)add_item(socket->data ,packet,sizeof(struct s_socket_packet));


    restore_flags(flags);
  }
}

int socket_recvfrom(unsigned int pid, unsigned int sock, unsigned char*buffer, unsigned int size, struct s_sockaddr_in *saddr)
{
  struct s_socket *socket;
  struct s_socket_packet *li = NULL;
  int i = 0;
  int rcv_size;
  unsigned int flags;

socket_recv:

  socket = socket_get(sock);
  if (socket == NULL) return ERROR_FILE_NOT_OPEN;

  li=(struct s_socket_packet*)get_item(socket->data,i++);
  while (li!=NULL)
  {
    memcpy(buffer, li->data->data, li->data->data_length);  // copy received data from
                                                            // socket to user memory

    // copy received socket address to user memory

    memcpy(saddr, &(li->saddr), sizeof (struct s_sockaddr_in));
    rcv_size = li->data->data_length;

    save_flags(flags);
    cli();
    socket->data = (struct list*)del_item(socket->data,i-1);
    restore_flags(flags);

    return rcv_size;

    li=(struct s_socket_packet*)get_item(socket->data,i++);
  }

  socket->state = SOCKET_RECEIVING;
  socket->buffer = buffer;
  socket->receive_pid = pid;

  task_control(pid, TASK_SLEEPING);

  return 0;
}

int socket_sendto(unsigned int sock, unsigned char*buffer, unsigned int size, struct s_sockaddr_in *daddr)
{
  struct s_socket *socket;
  struct s_net_buff *li = NULL;
  int i = 0;
  int rcv_size;
  unsigned int flags;

  socket = socket_get(sock);
  if (socket == NULL) return ERROR_FILE_NOT_OPEN;

  return 0;
}

int socket_connect(unsigned int sock, struct s_sockaddr_in *daddr)
{
  struct s_socket *socket;
  struct s_net_buff *li = NULL;
  int i = 0;
  int rcv_size;
  unsigned int flags;

  socket = socket_get(sock);
  if (socket == NULL) return ERROR_FILE_NOT_OPEN;

  tcp_connect(socket, daddr);

  return 0;
}

int socket_connected(struct s_socket *socket, int result)
{
  struct ipc_message *m;

  m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

  m->type = MSG_NET_SOCKET_CONNECT_REPLY;
  m->MSGT_NET_SOCKET_CONNECT_RESULT = result;

  send_msg(socket->owner, m);

  vfree(m);
}

void socket_monitor_redraw(int t)
{
  unsigned int flags;
  unsigned char str[255];
  struct s_socket *li;
  int i=0;
  int j=0;

  save_flags(flags);
  cli();

  li=(struct s_socket*)get_item(sockets,i++);

  while (li!=NULL)
  {
    sprintf(str,"Socket %02d, family %s, type %s",li->fd,
      (li->family == AF_INET)?"Internet":"Unknown", (li->type==SOCK_STREAM)?"TCP":"UDP");
    win_outs(socket_monitor_win, 10, 40+j*10, 8,str);
    j++;
    sprintf(str,"           port: %d   pa_addr: %s",htons(li->port), in_ntoa(li->pa_addr));
    win_outs(socket_monitor_win, 10, 40+j*10, 8,str);
    j++;

    li=(struct s_socket*)get_item(sockets,i++);
  }

  if (t)
    {
      sprintf(str,"Timer %d",timerticks);
      win_outs(socket_monitor_win, 10, 40+j*10, 8,str);
    }

  restore_flags(flags);
}

void socket_monitor_task()
{
  struct ipc_message *m;
  unsigned int ret;
  int i=0;
  unsigned char str[255] = {0};


  settimer(18);

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_MOUSE_BUT1_PRESSED:
            break;
       case MSG_MOUSE_BUT1_RELEASED:
            break;
       case MSG_GUI_REDRAW:
            hide_cursor();
            socket_monitor_redraw(0);
            show_cursor();
            break;
       case MSG_TIMER:
            socket_monitor_redraw(1);

     }
  }
}

void socket_init(void)
{
  printk("socket.c: Initializing socket interface...");

/*  socket_monitor_win = (struct window_t*)create_app(socket_monitor_task,"Socket monitor",
  100, 100, 300, 200);*/
}






