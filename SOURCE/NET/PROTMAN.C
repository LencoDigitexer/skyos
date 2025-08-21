/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\protman.c
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
#include "list.h"
#include "error.h"

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

unsigned int protman_pid;
unsigned int protman_arp_debug =0;

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
           if (protman_arp_debug)
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

void protocol_send(unsigned char* buffer, unsigned int size, unsigned int pa_addr )
{
  struct s_net_buff *buff;
  struct s_net_device *device;
  struct s_iphdr *iphdr;
  struct s_ethhdr *ethhdr;
  unsigned char ha_addr[7];     // destination hardware address
  unsigned int daddr;           // destination protocol address

  device = active_device;       // actual device

  daddr = pa_addr;              // destination IP address

  // check if we already know destination IP address
  /* If we are using a standard gateway, first ask for gateway address */
  if (device->pa_gateway)
  {
    if (!arp_get_mac(device->pa_gateway, ha_addr))
    {
      // add send buffer to send_queue for later sending
      buff = (struct s_net_buff*)net_buff_alloc(size);
      memcpy(buff->data, buffer, size);
      buff->pa_addr = daddr;
      buff->arp = 1;

      send_queue = (struct list*)add_item(send_queue,buff,sizeof(struct s_net_buff) + size);

    // now, send a ARP Request message, the real IP packet is already stored
    // in the send queue, so we can leave the protocol_send function after
    // sending the ARP Request.
      arp_request(device->pa_gateway, device);
      return;
    }
  }
  else
  {
    if (!arp_get_mac(device->pa_addr, ha_addr))
    {
      // add send buffer to send_queue for later sending
      buff = (struct s_net_buff*)net_buff_alloc(size);
      memcpy(buff->data, buffer, size);
      buff->pa_addr = daddr;
      buff->arp = 1;

      send_queue = (struct list*)add_item(send_queue,buff,sizeof(struct s_net_buff) + size);

    // now, send a ARP Request message, the real IP packet is already stored
    // in the send queue, so we leave the protocol_send function then.
      arp_request(pa_addr, device);
      return;
    }
  }

  // Ok, we know the destination IP and hardware address. Now build the IP
  // and Ethernet header, fill in the data and send the IP packet.

  buff = (struct s_net_buff*)ip_build_header(device->pa_addr, daddr,
		device, 0x01, size, 0, 128);

  // Fill data
  memcpy((buff->data + sizeof(struct s_ethhdr) + sizeof(struct s_iphdr)),
    buffer, size);

  iphdr = (struct s_iphdr*)(buff->data + sizeof(struct s_ethhdr));
  mem_dump(buff->data, 100);
  ip_dump(iphdr);

  el3_start_xmit(device, buff);
}

/* This function is called by the protman task. It checks if there are packets
/* to transmit in the send queue. */
void protman_transmit(void)
{
  struct s_net_buff *p;
  int count = 0;
  unsigned char str[255];

  p = (struct s_net_buff*)get_item(send_queue, count++);

  while (p != NULL)
  {
    // packet waits for ARP reply
    if (p->arp)
    {
       if (arp_get_mac(p->pa_addr, str))   // if already replied
       {
         protocol_send(p->data, p->data_length, p->pa_addr);

         count--;

         send_queue = (struct list*)del_item(send_queue, count);
        }
    }

    protocol_send(p->data, p->data_length, p->pa_addr);

    p = (struct s_net_buff*)get_item(send_queue, count++);
  }
}

void protman_task()
{
  struct ipc_message *m;
  struct s_net_buff *buff;
  unsigned char str[255];
  struct s_net_buff *p;

  int ret;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     protman_transmit();

     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_PROTMAN_RECEIVED:
            {
               buff = (struct s_net_buff*)net_buff_get();

               if (buff == NULL)
               {
                   alert("File: protman.c  Function: protman_task\n\n%s",
                         "No packets received?!?!");
               }

               else
               {
                 protocol_received(buff);
               }
               break;
            }
       case MSG_PROTMAN_SEND:
            {
               protocol_send(m->MSGT_PROTMAN_SEND_BUFFER,
                    m->MSGT_PROTMAN_SEND_SIZE,
                    in_aton(m->MSGT_PROTMAN_SEND_DEST));

               m->type = MSG_PROTMAN_SEND_REPLY;
               send_msg(m->source->pid, m);

               break;
            }
     }

  }
}

int protman_init(void)
{
  printk("protman.c: Initializing protocol manager...");
  protman_pid = ((struct task_struct*)CreateKernelTask((unsigned int)protman_task, "protman", HP,0))->pid;

  return 0;
}

