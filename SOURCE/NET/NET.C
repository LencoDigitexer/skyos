/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\net.c
/* Last Update: 25.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/************************************************************************/
/* Definition:
/*   Network Protocol Manager.
/************************************************************************/
#include "net.h"
#include "sched.h"
#include "netdev.h"
#include "ip.h"
#include "error.h"
#include "socket.h"

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

unsigned int net_pid;
unsigned int net_arp_debug =0;

extern struct s_net_device *active_device;

struct list *send_queue = NULL;          // send queue

/* routine to swap two bytes of a word */
static __inline__ unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

/***************************************************************************/
/* This is the entry point for every received packet.
/* The packet is then handled by the corrospondending protocol
/* receive function.
/***************************************************************************/
void protocol_received(struct s_net_buff *buff)
{
  struct s_ethhdr *eth;
  struct s_net_device *dev;
  struct s_iphdr *iphdr;
  struct s_icmphdr *icmphdr;

  dev = (struct s_net_device*)buff->dev;
  eth = (struct s_ethhdr*) buff->data;

  switch (htons(eth->h_proto))
  {
    case ETH_P_ARP:                     // Address Resolution packet
       {
           if (net_arp_debug)
             arp_debug(buff->data + sizeof(struct s_ethhdr));

           arp_rcv(buff , dev);

           vfree(buff->data);
           vfree(buff);
           break;
       }
    case ETH_P_IP:	            	// Internet Protocol packet
       {
          ip_rcv(buff, dev);

          vfree(buff->data);
          vfree(buff);
          break;
       }

    default:                            // Protocol Type isn't handled
       {
         vfree(buff->data);
         vfree(buff);
         break;
       }
  }
}

/***************************************************************************/
/* This function sends a packet to a specified hardware address.
/* If the hardware address is unknown, an arp request is sended,
/* and the packet will be stored in a buffer.
/***************************************************************************/
void net_queue_buff(struct s_net_buff *buff)
{
  struct s_net_device *device;
  unsigned char ha_addr[7];     // destination hardware address

  device = active_device;       // actual device

  if (buff->arp.need)          // we have to send an ARP request before
  {
    if (!arp_get_mac(buff->arp.addr, ha_addr))
    {
      /* add our buffer to send_queue for later sending */
      send_queue = (struct list*)add_item(send_queue,buff,sizeof(struct s_net_buff) + buff->data_length + sizeof(struct s_sockaddr_in));

      /* now, send a ARP Request message, the real IP packet is already stored
         in the send queue, so we leave the protocol_send function then. */
      arp_request(buff->arp.addr, device);
      return;
    }
  }

  /* ok, we now the destination hardware address, so send the packet */
  write_netdev(device->major, buff);
//  el3_start_xmit(device, buff);
}

/* This function is called by the net task. It checks if there are packets
/* to transmit in the send queue. */
void net_transmit(void)
{
  struct s_net_buff *p;
  int count = 0;
  unsigned char str[255];

  p = (struct s_net_buff*)get_item(send_queue, count++);

  while (p != NULL)
  {
    // packet waits for ARP reply
    if (p->arp.need)
    {
       if (arp_get_mac(p->arp.addr, str))   // if already replied
       {
         net_queue_buff(p);                  // send the packet

         count--;

         send_queue = (struct list*)del_item(send_queue, count);
        }
    }
    else
    {
       net_queue_buff(p);                  // send the packet
       count--;
       send_queue = (struct list*)del_item(send_queue, count);
    }

   p = (struct s_net_buff*)get_item(send_queue, count++);
  }
}

void net_task()
{
  struct ipc_message *m;
  struct s_net_buff *buff;
  unsigned char str[255];
  struct s_net_buff *p;

  int ret;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     net_transmit();

     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_NET_SOCKET_SOCKET:
            {
              ret = socket(m->MSGT_NET_SOCKET_SOCKET_FAMILY,
                           m->MSGT_NET_SOCKET_SOCKET_TYPE,
                           m->MSGT_NET_SOCKET_SOCKET_PROTOCOL,
                           m->MSGT_NET_SOCKET_SOCKET_PID);

              m->type = MSG_NET_SOCKET_SOCKET_REPLY;
              m->MSGT_NET_SOCKET_SOCKET_RESULT = ret;

              send_msg(m->source->pid, m);
              break;
            }
       case MSG_NET_SOCKET_BIND:
            {
              ret = socket_bind(m->MSGT_NET_SOCKET_BIND_SOCKET,
                           m->MSGT_NET_SOCKET_BIND_SADDR,
                           m->MSGT_NET_SOCKET_BIND_SIZE);

              m->type = MSG_NET_SOCKET_BIND_REPLY;
              m->MSGT_NET_SOCKET_BIND_RESULT = ret;

              send_msg(m->source->pid, m);
              break;
            }

       case MSG_NET_SOCKET_READ:
            {
              ret = socket_recvfrom(m->MSGT_NET_SOCKET_READ_PID,
                           m->MSGT_NET_SOCKET_READ_SOCKET,
                           m->MSGT_NET_SOCKET_READ_BUFFER,
                           m->MSGT_NET_SOCKET_READ_SIZE,
                           m->MSGT_NET_SOCKET_READ_SOURCE);

              if (ret != 0)
              {
                m->type = MSG_NET_SOCKET_READ_REPLY;
                m->MSGT_NET_SOCKET_READ_RESULT = ret;

                send_msg(m->source->pid, m);
              }
              break;
            }

       case MSG_NET_SOCKET_WRITE:
            {
              ret = socket_sendto(m->MSGT_NET_SOCKET_WRITE_SOCKET,
                           m->MSGT_NET_SOCKET_WRITE_BUFFER,
                           m->MSGT_NET_SOCKET_WRITE_SIZE,
                           m->MSGT_NET_SOCKET_WRITE_SOURCE);

              m->type = MSG_NET_SOCKET_WRITE_REPLY;
              m->MSGT_NET_SOCKET_WRITE_RESULT = ret;

              send_msg(m->source->pid, m);

              break;
            }

       case MSG_NET_SOCKET_CONNECT:
            {
              ret = socket_connect(m->MSGT_NET_SOCKET_CONNECT_SOCKET,
                           m->MSGT_NET_SOCKET_CONNECT_SOURCE);

              break;
            }

       case MSG_NET_RECEIVED:
            {
               buff = (struct s_net_buff*)net_buff_get();

               if (buff == NULL)
               {
                   alert("File: net.c  Function: net_task\n\n%s",
                         "No packets received?!?!");
               }

               else
               {
                 protocol_received(buff);
               }
               break;
            }
     }

  }
}

int net_init(void)
{
  printk("net.c: Initializing protocol manager...");
  net_pid = ((struct task_struct*)CreateKernelTask((unsigned int)net_task, "net", HP,0))->pid;

  return 0;
}

