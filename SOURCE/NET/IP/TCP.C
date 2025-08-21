/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\ip\tcp.c
/* Last Update: 15.12.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux, RFC 761, RFC 793
/************************************************************************/
/* Definition:
/*   Transmission Control Protocol Interface
/************************************************************************/
#include "netdev.h"
#include "net.h"
#include "net_buff.h"
#include "ip.h"
#include "tcp.h"
#include "socket.h"
#include "sched.h"
#include "list.h"
#include "error.h"

#define NULL (void*)0

#define RT_TIMEOUT 10
#define MAX_TIMEOUTS 1

struct list* rtq = NULL; // retransmitqueue

unsigned int net_tcp_debug = 0;

extern struct s_net_device *active_device;

struct window_t *tcp_monitor_win;
struct list *tcp_monitor_content = NULL;

#define PACKED __attribute__((packed))

struct s_mss
{
  unsigned char typ PACKED;
  unsigned char length PACKED;
  unsigned char data PACKED;
  unsigned char fill PACKED;
};


/************************************************************************/
/*   Print a message to screen in tcp_debug activated.
/************************************************************************/
void tcp_debug(unsigned char* str)
{
  unsigned int flags;

  save_flags(flags);
  cli();

  if (net_tcp_debug)
  {
    tcp_monitor_content = (struct list*)add_item(
      tcp_monitor_content,str,strlen(str));

    pass_msg(tcp_monitor_win, MSG_GUI_REDRAW);
  }

  restore_flags(flags);
}
/************************************************************************/
/* Functions to swap bytes of a short integer
/************************************************************************/
static unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

static unsigned short int ntohs(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}


/***************************************************************************/
/* Dump a tcp header to screen
/***************************************************************************/
void tcp_dump(struct s_tcphdr *tcp)
{
   printk("TCP Header:");
   printk("Source  : %d   Destination : %d",htons(tcp->source),htons(tcp->dest));
   printk("Sequence: %d   ACK Sequence: %d",ntohl(tcp->seq), ntohl(tcp->ack_seq));
   printk("Offset  : %d   U: %d  A: %d  P: %d  R: %d  S: %d  F: %d",
     tcp->doff, tcp->urg, tcp->ack, tcp->psh, tcp->rst, tcp->syn, tcp->fin);
   printk("Window  : %d   Check: %0004X   Urg: %d",htons(tcp->window), tcp->check, tcp->urg_ptr);
}

/***************************************************************************/
/* Checksum calculation for TCP packets
/***************************************************************************/
static inline unsigned int csum_fold(unsigned int sum)
{
	__asm__("
		addl %1, %0
		adcl $0xffff, %0
		"
		: "=r" (sum)
		: "r" (sum << 16), "0" (sum & 0xffff0000)
	);
	return (~sum) >> 16;
}
 
/***************************************************************************/
/* computes the checksum of the TCP/UDP pseudo-header
/* returns a 16-bit checksum, already complemented
/***************************************************************************/
static inline unsigned short int csum_tcpudp_magic(unsigned long saddr,
						   unsigned long daddr,
						   unsigned short len,
						   unsigned short proto,
						   unsigned int sum) {
    __asm__("
	addl %1, %0
	adcl %2, %0
	adcl %3, %0
	adcl $0, %0
	"
	: "=r" (sum)
	: "g" (daddr), "g"(saddr), "g"((ntohs(len)<<16)+proto*256), "0"(sum));
	return csum_fold(sum);
}

/***************************************************************************/
/* Add a packet to the retransmitqueue
/***************************************************************************/
void rtq_add(struct s_net_buff *buff, struct s_socket *socket)
{
  struct s_net_buff *b;

  b = (struct s_net_buff*)net_buff_alloc(buff->data_length);

  memcpy(b, buff, sizeof (struct s_net_buff) + buff->data_length);
  b->retransmittimer = RT_TIMEOUT;
  b->retransmits = 0;
  b->socket = socket;

  rtq=(struct list*)add_item(rtq,b,sizeof(struct s_net_buff) + buff->data_length);
}

/***************************************************************************/
/* Add a packet to the retransmitqueue
/***************************************************************************/
void rtq_update(void)
{
  int i=0;
  struct s_net_buff *li;

  li=(struct s_net_buff*)get_item(rtq,i++);

  while (li!=NULL)
  {
    li->retransmittimer--;

    if (li->retransmittimer == 0)
    {
       if (li->retransmits >= MAX_TIMEOUTS)
       {
          rtq=(struct list*)del_item(rtq,i-1);
          socket_connected(li->socket, ERROR_TIMEOUT);
          return;
       }
      net_queue_buff(li);
      li->retransmittimer = RT_TIMEOUT;
      li->retransmits++;
    }
  li=(struct s_net_buff*)get_item(rtq,i++);
  }
}

/***************************************************************************/
/* Check for a packet in the retransmitqueue
/***************************************************************************/
void rtq_check(struct s_net_buff *buff)
{
  int i=0;
  struct s_net_buff *li = NULL;
  struct s_iphdr *iphdr;
  struct s_tcphdr *tcphdr;

  struct s_iphdr *r_iphdr;
  struct s_tcphdr *r_tcphdr;

  r_iphdr = (struct s_iphdr*)(buff->data + sizeof(struct s_ethhdr));
  r_tcphdr = (struct s_tcphdr*)(buff->data + sizeof(struct s_ethhdr) + iphdr->ihl*4);

  li=(struct s_net_buff*)get_item(rtq,i++);

  while (li!=NULL)
  {
    iphdr = (struct s_iphdr*)(li->data + sizeof(struct s_ethhdr));
    tcphdr = (struct s_tcphdr*)(li->data + sizeof(struct s_ethhdr) + iphdr->ihl*4);

    if ((tcphdr->syn) && (!tcphdr->ack))           // we sent a connection request
    {
      if (tcphdr->seq == htonl(htonl(r_tcphdr->ack_seq)-1))
      {
        rtq=(struct list*)del_item(rtq,i-1);
        return;
      }
    }
    li=(struct s_net_buff*)get_item(rtq,i++);
  }
}

/***************************************************************************/
/* Send an ACK and SYN packet.
/***************************************************************************/
void tcp_send_ack(struct s_net_device *device,
                  struct s_tcphdr *r_tcphdr,
                  struct s_sockaddr_in *saddr,
                  struct s_sockaddr_in *daddr,
                  struct s_socket *socket,
                  unsigned int syn_flag)
{
  struct s_iphdr *iphdr;
  struct s_net_buff *buff;
  struct s_tcphdr *tcphdr;
  int check;

  buff = (struct s_net_buff*)ip_build_header(device->pa_addr, daddr->sa_addr,
		device, 6, (sizeof(struct s_tcphdr)), 0, 128);

  // Build tcp header
  tcphdr = (struct s_tcphdr*)(buff->data + sizeof(struct s_ethhdr) + 20);

  tcphdr->source = saddr->sa_port;
  tcphdr->dest = daddr->sa_port;

  tcphdr->ack_seq = ntohl(ntohl(r_tcphdr->seq)+1);

  socket->seq = htonl(htonl(socket->seq) + 1);
  tcphdr->seq = socket->seq;

  tcphdr->urg = tcphdr->psh = tcphdr->rst = tcphdr->fin = 0;
  tcphdr->syn = syn_flag;
  tcphdr->ack = 1;
  tcphdr->doff = 5;

  tcphdr->window = r_tcphdr->window;

  // Insert data to send
//  memcpy((buff->data + sizeof(struct s_ethhdr) + 20 + sizeof(struct s_udphdr)), buffer, size);

  tcphdr->check = 0;

  iphdr = (struct s_iphdr*)(buff->data + sizeof(struct s_ethhdr));
  /* Calculate checksum of data and tcpheader */
  check = 0;
  check = csum_partial((unsigned char*)tcphdr, htons(iphdr->tot_len) - (iphdr->ihl*4),
 				   check);

  /* Calculate checksum of fake haeder and add data/udpheader csum */
  check = csum_tcpudp_magic(device->pa_addr, daddr->sa_addr,
					  htons(iphdr->tot_len) - (iphdr->ihl*4),
					  6, check);
  if (check == 0) check = -1;

  tcphdr->check = check;

  if (net_tcp_debug)
  {
    printk("TCP Output:");
    tcp_dump(tcphdr);
    mem_dump(tcphdr, htons(iphdr->tot_len) - (iphdr->ihl*4));
  }

  net_queue_buff(buff);
}

/***************************************************************************/
/* Send an ACK packet.
/***************************************************************************/
void tcp_ack(struct s_net_device *device,
                  struct s_net_buff *r_buff,
                  struct s_sockaddr_in *saddr,
                  struct s_sockaddr_in *daddr,
                  struct s_socket *socket)
{
  struct s_iphdr *iphdr;
  struct s_net_buff *buff;
  struct s_tcphdr *tcphdr;
  int check;
  struct s_iphdr *r_iphdr;
  struct s_tcphdr *r_tcphdr;
  int size;

  r_iphdr = (struct s_iphdr*)(r_buff->data + sizeof(struct s_ethhdr));
  r_tcphdr = (struct s_tcphdr*)(r_buff->data + sizeof(struct s_ethhdr) + r_iphdr->ihl * 4);

  buff = (struct s_net_buff*)ip_build_header(device->pa_addr, daddr->sa_addr,
		device, 6, (sizeof(struct s_tcphdr)), 0, 128);

  // Build tcp header
  tcphdr = (struct s_tcphdr*)(buff->data + sizeof(struct s_ethhdr) + 20);

  tcphdr->source = saddr->sa_port;
  tcphdr->dest = daddr->sa_port;

  // calculate size of tcp segment

  size = htons(r_iphdr->tot_len) - r_iphdr->ihl * 4 - sizeof(struct s_tcphdr);

  tcphdr->seq = r_tcphdr->ack_seq;
  tcphdr->ack_seq = ntohl(ntohl(r_tcphdr->seq) + size);
  socket->seq = tcphdr->ack_seq;

  tcphdr->urg = tcphdr->psh = tcphdr->rst = tcphdr->fin = 0;
  tcphdr->syn = 0;
  tcphdr->ack = 1;
  tcphdr->doff = 5;

  tcphdr->window = r_tcphdr->window;

  // Insert data to send
//  memcpy((buff->data + sizeof(struct s_ethhdr) + 20 + sizeof(struct s_udphdr)), buffer, size);

  tcphdr->check = 0;

  iphdr = (struct s_iphdr*)(buff->data + sizeof(struct s_ethhdr));
  /* Calculate checksum of data and tcpheader */
  check = 0;
  check = csum_partial((unsigned char*)tcphdr, htons(iphdr->tot_len) - (iphdr->ihl*4),
 				   check);

  /* Calculate checksum of fake haeder and add data/udpheader csum */
  check = csum_tcpudp_magic(device->pa_addr, daddr->sa_addr,
					  htons(iphdr->tot_len) - (iphdr->ihl*4),
					  6, check);
  if (check == 0) check = -1;

  tcphdr->check = check;

  if (net_tcp_debug)
  {
    printk("TCP Output:");
    tcp_dump(tcphdr);
    mem_dump(tcphdr, htons(iphdr->tot_len) - (iphdr->ihl*4));
  }

  net_queue_buff(buff);
}

/***************************************************************************/
/* Send an SYN packet (To open a connection)
/***************************************************************************/
void tcp_send_syn(struct s_net_device *device,
                  struct s_sockaddr_in *saddr,
                  struct s_sockaddr_in *daddr,
                  struct s_socket *socket)
{
  struct s_iphdr *iphdr;
  struct s_net_buff *buff;
  struct s_tcphdr *tcphdr;
  struct s_mss *mss;
  int check;


  buff = (struct s_net_buff*)ip_build_header(saddr->sa_addr, daddr->sa_addr,
		device, 6, (sizeof(struct s_tcphdr)) + 4, 0, 128);

  // Build tcp header
  tcphdr = (struct s_tcphdr*)(buff->data + sizeof(struct s_ethhdr) + 20);

  tcphdr->source = saddr->sa_port;
  tcphdr->dest = daddr->sa_port;

  tcphdr->seq = socket->seq;
  socket->seq = htonl(htonl(socket->seq) + 1);

  tcphdr->urg = tcphdr->ack = tcphdr->psh = tcphdr->rst = tcphdr->fin = 0;
  tcphdr->syn = 1;
  tcphdr->doff = 6;

  tcphdr->window = htons(8192);

  mss = (struct s_mss*)(buff->data + sizeof(struct s_ethhdr) + 40);
  mss->typ = 2;
  mss->length = 4;
  mss->data = 5;
  mss->fill = 0xb4;

  tcphdr->check = 0;

  iphdr = (struct s_iphdr*)(buff->data + sizeof(struct s_ethhdr));

  /* Calculate checksum of data and tcpheader */
  check = 0;
  check = csum_partial((unsigned char*)tcphdr, htons(iphdr->tot_len) - (iphdr->ihl*4),
 				   check);

  /* Calculate checksum of fake haeder and add data/udpheader csum */
  check = csum_tcpudp_magic(device->pa_addr, daddr->sa_addr,
					  htons(iphdr->tot_len) - (iphdr->ihl*4),
					  6, check);
  if (check == 0) check = -1;

  tcphdr->check = check;

  if (net_tcp_debug)
  {
    printk("TCP Output:");
    tcp_dump(tcphdr);
    mem_dump(tcphdr, htons(iphdr->tot_len) - (iphdr->ihl*4));
  }

  socket->state = SOCKET_W_SYN_ACK;

  rtq_add(buff, socket);
  net_queue_buff(buff);
}

void tcp_connect(struct s_socket *socket, struct s_sockaddr_in *daddr)
{
  struct s_sockaddr_in saddr;
  struct s_net_device *dev;

  dev = active_device;

  saddr.sa_port = socket->port;
  saddr.sa_addr = socket->pa_addr;

  socket->state = SOCKET_W_SYN_ACK;
  tcp_send_syn(dev, &saddr, daddr, socket);

}

/***************************************************************************/
/* Entry point for a received TCP packet.
/***************************************************************************/
int tcp_rcv(struct s_net_buff *buff, struct s_net_device *dev)
{
  struct s_iphdr   *iphdr;
  struct s_tcphdr  *tcphdr;
  struct s_sockaddr_in saddr;
  struct s_sockaddr_in daddr;
  int checksum;
  int check;
  int sock;
  unsigned char str[255];
  struct s_socket *socket;

  iphdr = (struct s_iphdr*)(buff->data + sizeof(struct s_ethhdr));
  tcphdr = (struct s_tcphdr*)(buff->data + sizeof(struct s_ethhdr) + iphdr->ihl*4);

  if (net_tcp_debug)
    tcp_dump(tcphdr);

  /* RFC says, we MUST discard any packet with a wrong checksum */

  checksum = tcphdr->check;
  tcphdr->check = 0;

  /* Calculate checksum of data and tcpheader */
  check = 0;

  check = csum_partial((unsigned char*)tcphdr, htons(iphdr->tot_len) - (iphdr->ihl*4),
 				   check);

  /* Calculate checksum of fake haeder and add data/udpheader csum */
  check = csum_tcpudp_magic(iphdr->saddr, iphdr->daddr,
					  htons(iphdr->tot_len) - (iphdr->ihl*4),
					  6, check);

  if (check != checksum)
  {
    alert("File: tcp.c   Fucntion: tcp_rcv      \n\n%s%0004X\n%s%0004X\n",
          "Received Checksum  : ", checksum,
          "Calculated Checksum: ", check);
    return 0;
  }

  tcphdr->check = check; /* restore old checksum */

  daddr.sa_family = AF_INET;
  daddr.sa_port   = tcphdr->dest;
  daddr.sa_addr   = iphdr->daddr;

  sock = socket_get_fd(&daddr);

  if (sock == -1)
  {
    alert("File: tcp.c  Function: tcp_rcv\n\n%s",
           "Requested socket not registered!");
    return;
  }

  /* Here we check, if someone is in connecting state with us. */
  if (tcphdr->syn)
  {
     /* If SYN flag is set and ACK zero, someone wants to connect to us*/
     if (!tcphdr->ack)
     {
       sprintf(str,"<1> Request for connection from %s at %d",
         in_ntoa(iphdr->saddr), htons(tcphdr->source));
       tcp_debug(str);

       sprintf(str,"    to %s at %d",
         in_ntoa(iphdr->daddr), htons(tcphdr->dest));
       tcp_debug(str);

       sprintf(str,"<1> Sending ACK,SYN to foreign host");
       tcp_debug(str);

       socket = (struct s_socket*)socket_get(sock);

       daddr.sa_addr = iphdr->saddr;

       saddr.sa_port = socket->port;
       saddr.sa_addr = dev->pa_addr;

       daddr.sa_port = tcphdr->source;
       daddr.sa_addr = iphdr->saddr;

       tcp_send_ack(dev, tcphdr, &saddr, &daddr, socket, 1);
     }
     if (tcphdr->ack)   // reply for our connecting request
     {
       sprintf(str,"<1> Connected to %s at %d",
         in_ntoa(iphdr->saddr), htons(tcphdr->source));
       tcp_debug(str);

       sprintf(str,"    from %s at %d",
         in_ntoa(iphdr->daddr), htons(tcphdr->dest));
       tcp_debug(str);

       socket = (struct s_socket*)socket_get(sock);

       if (socket->state != SOCKET_W_SYN_ACK)
         return;

       saddr.sa_port = socket->port;
       saddr.sa_addr = dev->pa_addr;

       daddr.sa_port = tcphdr->source;
       daddr.sa_addr = iphdr->saddr;

       rtq_check(buff);

       tcp_send_ack(dev, tcphdr, &saddr, &daddr, socket, 0);

       socket->state = SOCKET_CONNECTED;
       socket_connected(socket, 1);
     }
  }

  /* check if someone sends data to us */
  if (tcphdr->ack && tcphdr->psh)
  {
       sprintf(str,"<1> ACK/PSH from %s at %d",
         in_ntoa(iphdr->saddr), htons(tcphdr->source));
       tcp_debug(str);

       sprintf(str,"    to %s at %d",
         in_ntoa(iphdr->daddr), htons(tcphdr->dest));
       tcp_debug(str);

       sprintf(str,"<1> Sending ACK to foreign host");
       tcp_debug(str);

       socket = (struct s_socket*)socket_get(sock);
       daddr.sa_addr = iphdr->saddr;

       saddr.sa_port = socket->port;
       saddr.sa_addr = dev->pa_addr;

       daddr.sa_port = tcphdr->source;
       daddr.sa_addr = iphdr->saddr;


       tcp_ack(dev, buff, &saddr, &daddr, socket);

       daddr.sa_family = AF_INET;
       daddr.sa_port = tcphdr->dest;
       daddr.sa_addr = iphdr->daddr;

       saddr.sa_family = AF_INET;
       saddr.sa_port = tcphdr->source;
       saddr.sa_addr = iphdr->saddr;

       socket_received(&saddr, &daddr, (buff->data + sizeof(struct s_ethhdr) +
        iphdr->ihl * 4 + sizeof(struct s_tcphdr)), htons(iphdr->tot_len) - iphdr->ihl*4 - sizeof(struct s_tcphdr));
  }
}

void tcp_monitor_redraw(void)
{
  int i=0;
  unsigned char* fi;
  int j=0;

  fi=(unsigned char*)get_item(tcp_monitor_content,i++);

  while (fi!=NULL)
  {
    win_outs(tcp_monitor_win, 10, 40+j*10, 8, fi);
    j++;
    fi=(unsigned char*)get_item(tcp_monitor_content,i++);
  }
}

void tcp_monitor_task()
{
  struct ipc_message *m;
  unsigned int ret;
  int i=0;

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
            tcp_monitor_redraw();
            show_cursor();
            break;
     }
  }
}

void tcp_init(void)
{
  printk("tcp.c: Initializing TCP Protocol...");

/*  tcp_monitor_win = (struct window_t*)create_app(tcp_monitor_task,"TCP-Connections",
    100, 100, 300, 200);*/
}



