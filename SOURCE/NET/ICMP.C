/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\icmp.c
/* Last Update: 02.12.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux
/*              ICMP: rfc777.txt
/************************************************************************/
/* Definition:
/*   Internet Control Message Protocol Driver.
/************************************************************************/
#include "netdev.h"
#include "ip.h"

extern unsigned int ip_count;           // IP-ID value
unsigned int protman_icmp_debug;

static unsigned short int ntohs(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

static unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

void icmp_echo_reply(struct s_net_buff *buff, struct s_net_device *dev)
{
  struct s_net_buff *send_buff;

  struct s_iphdr *iphdr_source;
  struct s_iphdr *iphdr_dest;
  struct s_ethhdr *ethhdr_source;
  struct s_ethhdr *ethhdr_dest;
  struct s_icmphdr *icmphdr_source;
  struct s_icmphdr *icmphdr_dest;
  unsigned short oldcheck, checksum;
  unsigned int len = 0;
  unsigned int i;

  ethhdr_source = (struct s_ethhdr*)(buff->data);
  iphdr_source = (struct s_iphdr*)(buff->data + sizeof(struct s_ethhdr));
  icmphdr_source = (struct s_icmphdr*)(buff->data + sizeof(struct s_ethhdr) +
    20);

  send_buff = (struct s_net_buff*)net_buff_alloc(htons(iphdr_source->tot_len) +
    sizeof(struct s_ethhdr));

  // copy received packet to send packet (ignore Ethernet and IP header)
  memcpy(send_buff->data, buff->data, htons(iphdr_source->tot_len)
   + sizeof(struct s_ethhdr));

  ethhdr_dest = (struct s_ethhdr*)(send_buff->data);
  iphdr_dest = (struct s_iphdr*)(send_buff->data + sizeof(struct s_ethhdr));
  icmphdr_dest = (struct s_icmphdr*)(send_buff->data + sizeof(struct s_ethhdr)
   + 20);

  // Build MAC header
  memcpy(ethhdr_dest->h_dest, ethhdr_source->h_source, 6);
  memcpy(ethhdr_dest->h_source, ethhdr_source->h_dest, 6);
  ethhdr_dest->h_proto = ethhdr_source->h_proto;

  // Build IP header
  iphdr_dest->version  = 4;
  iphdr_dest->tos      = 0;
  iphdr_dest->frag_off = 0;
  iphdr_dest->ttl      = 32;
  iphdr_dest->daddr    = iphdr_source->saddr;
  iphdr_dest->saddr    = iphdr_source->daddr;
  iphdr_dest->protocol = 0x01;                  //ICMP Message
  iphdr_dest->ihl      = 5;
  iphdr_dest->id       = htons(ip_count++);
  iphdr_dest->tot_len  = iphdr_source->tot_len;

  ip_send_check(iphdr_dest);

  // Build ICMP Header
  icmphdr_dest->type = 0;
  icmphdr_dest->code = 0;
  icmphdr_dest->checksum = 0;

  icmphdr_dest->checksum = ip_compute_csum((unsigned char *)icmphdr_dest,
                         htons(iphdr_source->tot_len) - 20);

/*  mem_dump(buff->data, 100);
  ip_dump(iphdr_source);
  icmp_dump(icmphdr_source);

  printk("Sending data:");
  mem_dump(send_buff->data, 100);
  ip_dump(iphdr_dest);
  icmp_dump(icmphdr_dest);*/

  el3_start_xmit(dev, send_buff);
}

void icmp_echo_request(unsigned char *da, struct s_net_device *dev, unsigned int checksum, unsigned int deb)
{
  struct s_net_buff *send_buff;
  struct s_icmphdr *icmphdr;
  unsigned int daddr;

  daddr = in_aton(da);

  printk("ip.c: icmp_echo_request() sending to %s",in_ntoa(daddr));

  send_buff = (struct s_net_buff*)ip_build_header(dev->pa_addr, daddr,
		dev, 0x01, sizeof(struct s_icmphdr) + 32, 0, 32);

  icmphdr = (struct s_icmphdr*)(send_buff->data + sizeof(struct s_ethhdr) +
    20);

  icmphdr->type = 8;
  icmphdr->code = 0;
  icmphdr->checksum =  ip_compute_csum((unsigned char *)icmphdr,
                         32 +  sizeof(struct s_icmphdr));

  el3_start_xmit(dev, send_buff);
}

void icmp_rcv(struct s_net_buff *buff, struct s_net_device *dev)
{
  struct s_icmphdr *icmphdr;
  struct s_iphdr   *iphdr;

  iphdr = (struct s_iphdr*)(buff->data + sizeof(struct s_ethhdr));
  icmphdr = (struct s_icmphdr*)(buff->data + sizeof(struct s_ethhdr) +
    iphdr->ihl * 4);

  switch (icmphdr->type)
  {
    case ICMP_ECHO:
     {
       if (protman_icmp_debug)
         printk("ip.c: icmp_rcv() ECHO REQUEST received.");

       icmp_echo_reply(buff, dev);
       break;
     }
    case ICMP_ECHOREPLY:
     {
       printk("ip.c: echo reply received.");
       break;
     }
     default:
     {
       printk("ip.c: icmp_rcv() ICMP Type 0x%X not supported in this kernel.",icmphdr->type);
       break;
     }
  }
}
