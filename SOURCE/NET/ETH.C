/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\eth.c
/* Last Update: 02.12.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux, Etherlink III Parallel Tasking
/*              Documentation from Stoettinger
/************************************************************************/
/* Definition:
/*   Ethernet driver. Support all functions to communicate over an
/*   ethernet net.
/************************************************************************/
#include "netdev.h"
#include "net_buffer.h"

#define NULL (void*)0

/* Display an Ethernet address in readable format. */
char *eth_print(unsigned char *ptr)
{
  static char buff[64];

  if (ptr == NULL) return("[NONE]");
  sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X",
	(ptr[0] & 255), (ptr[1] & 255), (ptr[2] & 255),
	(ptr[3] & 255), (ptr[4] & 255), (ptr[5] & 255)
  );
  return(buff);
}

void eth_build_header(struct s_net_buff *buff, struct s_net_device *dev,
   unsigned char* daddr, unsigned short proto)
{
  struct s_ethhdr eth;
  unsigned short *phys_addr;
  unsigned short *pointer;
  int i;

  buff->dev = dev;
  // insert active interface source address
  phys_addr = (unsigned short*)&(eth.h_source);
  pointer = (unsigned short*)dev->dev_addr;

  for (i = 0; i < 3; i++)
	  phys_addr[i] = pointer[i];

  // destination is FF:FF:FF:FF:FF:FF (broadcast)
  phys_addr = (unsigned short*)&(eth.h_dest);
  for (i = 0; i < 3; i++)
	  phys_addr[i] = 0xFFFF;

  // no protocol used
  eth.h_proto = proto;

  // set ethernet header pointer
  memcpy(buff->data, &eth, sizeof(struct s_ethhdr));

  buff->rowdata = buff->data + sizeof(struct s_ethhdr);


}











