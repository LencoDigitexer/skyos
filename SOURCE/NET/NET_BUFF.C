/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\net_buff.c
/* Last Update: 20.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Linux
/************************************************************************/
/* Definition:
/*   Network packet buffer.
/* To Do:
/************************************************************************/
#include "sched.h"
#include "netdev.h"

#define NULL (void*)0

/* These are the defined Ethernet Protocol ID's. */
#define ETH_P_LOOP	0x0060		/* Ethernet Loopback packet	*/
#define ETH_P_ECHO	0x0200		/* Ethernet Echo packet		*/
#define ETH_P_PUP	0x0400		/* Xerox PUP packet		*/
#define ETH_P_IP	0x0800		/* Internet Protocol packet	*/
#define ETH_P_ARP	0x0806		/* Address Resolution packet	*/
#define ETH_P_RARP      0x8035		/* Reverse Addr Res packet	*/
#define ETH_P_X25	0x0805		/* CCITT X.25			*/
#define ETH_P_IPX	0x8137		/* IPX over DIX			*/
#define ETH_P_802_3	0x0001		/* Dummy type for 802.3 frames  */
#define ETH_P_AX25	0x0002		/* Dummy protocol id for AX.25  */
#define ETH_P_ALL	0x0003		/* Every packet (be careful!!!) */

struct s_net_buff *net_buff = NULL;
extern int net_pid;

static __inline__ unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

int init_net_buff(void)
{
  printk("net_buff.c: Initiailizing network packet buffering...");
  net_buff = NULL;
}

struct s_net_buff* net_buff_alloc(unsigned int size)
{
  struct s_net_buff *buff;

  buff = (struct s_net_buff*)valloc(sizeof(struct s_net_buff));
  memset(buff,0,sizeof(struct s_net_buff));
  buff->data = (unsigned char*)valloc(size+1);
  buff->data_length = size;

  return buff;
}

int net_buff_add(struct s_net_buff *buff)
{
  struct s_net_buff *p;
  struct ipc_message *m;
  unsigned int flags;

  save_flags(flags);
  cli();

  if ((net_buff) == NULL)               // first packet in queue
  {
    net_buff = buff;
    net_buff->next = NULL;
  }

  else                                  // insert at tail
  {
    p = net_buff;

    while (p->next)
      p = p->next;

    p->next = buff;
    p->next->next = NULL;
  }

  // send a message to protocol manager

  m = (struct ipc_message*)valloc(sizeof(struct ipc_message));
  m->type = MSG_NET_RECEIVED;

  send_msg(net_pid, m);
  vfree(m);

  restore_flags(flags);
}

struct s_net_buff *net_buff_get(void)
{
  struct s_net_buff *p;
  unsigned int flags;

  save_flags(flags);
  cli();

  if ((net_buff) == NULL)               // first packet in queue
  {
    restore_flags(flags);
    return NULL;
  }

  p = net_buff;

  net_buff = net_buff->next;

  restore_flags(flags);

  return p;
}

void net_buff_debug(void)
{
  struct s_net_buff *p;
  unsigned char ch=0;
  unsigned char str[255];

  console_clear();

  p = net_buff;

  if (!p)
  {
    printk("net_buff.c: net_buff_debug() no packets.");
    return;
  }

  while (ch!=27)
  {
    if (p)
    {
      sprintf(str, "Source Address     : %02X:%02X:%02X:%02X:%02X:%02X",p->eth->h_source[0],p->eth->h_source[1],
      p->eth->h_source[2],p->eth->h_source[3],p->eth->h_source[4],p->eth->h_source[5]);
      printk(str);

      sprintf(str, "Destination Address: %02X:%02X:%02X:%02X:%02X:%02X",p->eth->h_dest[0],p->eth->h_dest[1],
      p->eth->h_dest[2],p->eth->h_dest[3],p->eth->h_dest[4],p->eth->h_dest[5]);
      printk(str);

      switch (htons(p->eth->h_proto))
      {
        case ETH_P_LOOP	 : sprintf(str,"Ethernet Loopback packet"); break;
        case ETH_P_ECHO	 : sprintf(str,"Ethernet Echo packet"); break;
        case ETH_P_PUP 	 : sprintf(str,"Xerox PUP packet"); break;
        case ETH_P_IP  	 : sprintf(str,"Internet Protocol packet"); break;
        case ETH_P_ARP 	 : sprintf(str,"Address Resolution packet"); break;
        case ETH_P_RARP  : sprintf(str,"Reverse Address Resolution packet"); break;
        case ETH_P_X25 	 : sprintf(str,"CCITT X.25"); break;
        case ETH_P_IPX   : sprintf(str,"IPX over DIX"); break;
        case ETH_P_802_3 : sprintf(str,"Dummy type for 802.3 frames"); break;
        case ETH_P_AX25  : sprintf(str,"Dummy protocol ID for AX.25"); break;
        case ETH_P_ALL   : sprintf(str,"Every packet"); break;
        default: sprintf(str,"Unknown ethernet packet"); break;

      }
      printk("Ethernet type: %0004X %s",p->eth->h_proto,str);
      printk("Ethernet datas with header:");

      mem_dump(p->data, p->data_length);
      printk("");

      if (htons(p->eth->h_proto) == ETH_P_ARP)
      {
        arp_debug(p->data+sizeof(struct s_ethhdr));
      }
      printk("");
      ch = getch();
      p = p->next;
    }
    else ch = 27;
  }
}


