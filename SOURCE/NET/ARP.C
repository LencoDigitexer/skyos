/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 -1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\arp.c
/* Last Update: 01.12.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux, Etherlink III Parallel Tasking
/*              RFC 826
/************************************************************************/
/* Definition:
/*   Address Resolution Protocol Driver.
/************************************************************************/
#include "system.h"
#include "netdev.h"
#include "list.h"

#define NULL (void*)0

/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_NETROM	0		/* from KA9Q: NET/ROM pseudo	*/
#define ARPHRD_ETHER 	1		/* Ethernet 10Mbps		*/
#define	ARPHRD_EETHER	2		/* Experimental Ethernet	*/
#define	ARPHRD_AX25	3		/* AX.25 Level 2		*/
#define	ARPHRD_PRONET	4		/* PROnet token ring		*/
#define	ARPHRD_CHAOS	5		/* Chaosnet			*/
#define	ARPHRD_IEEE802	6		/* IEEE 802.2 Ethernet- huh?	*/
#define	ARPHRD_ARCNET	7		/* ARCnet			*/
#define	ARPHRD_APPLETLK	8		/* APPLEtalk			*/

/* ARP protocol opcodes. */
#define	ARPOP_REQUEST	1		/* ARP request			*/
#define	ARPOP_REPLY	2		/* ARP reply			*/
#define	ARPOP_RREQUEST	3		/* RARP request			*/
#define	ARPOP_RREPLY	4		/* RARP reply			*/

/*
 * Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  ARP packets are variable
 * in size; the arphdr structure defines the fixed-length portion.
 * Protocol type values are the same as those for 10 Mb/s Ethernet.
 * It is followed by the variable-sized fields ar_sha, arp_spa,
 * arp_tha and arp_tpa in that order, according to the lengths
 * specified.  Field names used correspond to RFC 826.
 */

extern struct s_net_device *active_device;

struct s_arp_statistics
{
   unsigned int sended;
   unsigned int sended_requests;
   unsigned int sended_replies;

   unsigned int received;
   unsigned int received_requests;
   unsigned int received_replies;
};

struct s_arphdr
{
  unsigned short	ar_hrd;		/* format of hardware address	*/
  unsigned short	ar_pro;		/* format of protocol address	*/
  unsigned char		ar_hln;		/* length of hardware address	*/
  unsigned char		ar_pln;		/* length of protocol address	*/
  unsigned short	ar_op;		/* ARP opcode (command)		*/
};

struct s_arpcache
{
  unsigned int pa_addr;                 /* protocol address */
  unsigned char ha_addr[7];             /* hardware address */
  struct s_arpcache *next;
};

struct list *arp_cache = NULL;          /* arp cache buffer */

struct s_arp_statistics arp_statistics = {0};

/* ARP Flag values. */
#define	ATF_INUSE	0x01		/* entry in use			*/
#define ATF_COM		0x02		/* completed entry (ha valid)	*/
#define	ATF_PERM	0x04		/* permanent entry		*/
#define	ATF_PUBL	0x08		/* publish entry		*/
#define	ATF_USETRAILERS	0x10		/* has requested trailers	*/

static char* unk_print(unsigned char *ptr, int len);
static char* eth_aprint(unsigned char *ptr, int len);

static struct
{
  char	*name;
  char	*(*print)(unsigned char *ptr, int len);
} arp_types[] = {
  { "0x%04X",			unk_print	},
  { "10 Mbps Ethernet", 	eth_aprint	},
  { "3 Mbps Ethernet",		eth_aprint	},
  { "AX.25",			unk_print	},
  { "Pronet",			unk_print	},
  { "Chaos",			unk_print	},
  { "IEEE 802.2 Ethernet (?)",	eth_aprint	},
  { "Arcnet",			unk_print	},
  { "AppleTalk",		unk_print	},
  { NULL,			NULL		}
};
#define	ARP_MAX_TYPE	(sizeof(arp_types) / sizeof(arp_types[0]))

static char *arp_cmds[] = {
  "0x%04X",
  "REQUEST",
  "REPLY",
  "REVERSE REQUEST",
  "REVERSE REPLY",
  NULL
};
#define	ARP_MAX_CMDS	(sizeof(arp_cmds) / sizeof(arp_cmds[0]))

/* routine to swap two bytes of a word */
static __inline__ unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

/* Dump the ADDRESS bytes of an unknown hardware type. */
static char *
unk_print(unsigned char *ptr, int len)
{
  static char buff[32];
  char *bufp = buff;
  int i;

  for (i = 0; i < len; i++)
	bufp += sprintf(bufp, "%02X ", (*ptr++ & 0377));
  return(buff);
}

/* Dump the ADDRESS bytes of an Ethernet hardware type. */
static char *
eth_aprint(unsigned char *ptr, int len)
{
  if (len != ETH_ALEN) return("");
  return((char*)eth_print(ptr));
}

/* Display an IP address in readable format. */
char *in_ntoa(unsigned long in)
{
  static char buff[18];
  register char *p;

  p = (char *) &in;
  sprintf(buff, "%d.%d.%d.%d",
	(p[0] & 255), (p[1] & 255), (p[2] & 255), (p[3] & 255));
  return(buff);
}

void arp_debug(struct s_arphdr *arp)
{
  int len, idx;
  unsigned char *ptr;

  printk("ARP Header: ");
  if (arp == NULL)
  {
	printk("(null)\n");
	return;
  }

  /* Print the opcode name. */
  len = htons(arp->ar_op);
  if (len < ARP_MAX_CMDS) idx = len;
    else idx = 0;
  printk("Opcode: %s",arp_cmds[idx]);

  /* Print the ARP header. */
  len = htons(arp->ar_hrd);
  if (len < ARP_MAX_TYPE) idx = len;
    else idx = 0;

  printk("   hrd = %s", arp_types[idx].name);
  printk("   pro = 0x%04X\n", htons(arp->ar_pro));
  printk("   hlen = %d plen = %d\n", arp->ar_hln, arp->ar_pln);

  /*
   * Print the variable data.
   * When ARP gets redone (after the formal introduction of NET-2),
   * this part will be redone.  ARP will then be a multi-family address
   * resolver, and the code below will be made more general. -FvK
   */
  ptr = ((unsigned char *) &arp->ar_op) + sizeof(unsigned short);
  printk("   sender HA = %s ", arp_types[idx].print(ptr, arp->ar_hln));
  ptr += arp->ar_hln;
  printk("  PA = %s\n", in_ntoa(*(unsigned long *) ptr));
  ptr += arp->ar_pln;
  printk("   target HA = %s ", arp_types[idx].print(ptr, arp->ar_hln));
  ptr += arp->ar_hln;
  printk("  PA = %s\n", in_ntoa(*(unsigned long *) ptr));
}

/* Create and send our response to an ARP request. */
int arp_response(struct s_arphdr *arp1, struct s_net_device *dev)
{
  struct s_arphdr *arp2;
  struct s_net_buff *buff;
  unsigned long src, dst;
  unsigned char *ptr1, *ptr2;
  int hlen;
  unsigned char str[255];

  /* Decode the source (REQUEST) message. */
  ptr1 = ((unsigned char *) &arp1->ar_op) + sizeof(unsigned short);
  src = *((unsigned long *) (ptr1 + arp1->ar_hln));
  dst = *((unsigned long *) (ptr1 + (arp1->ar_hln * 2) + arp1->ar_pln));

  // check if my protocol address
  if (dst != dev->pa_addr)
    return;

  /* Get some mem and initialize it for the return trip. */
  buff = (struct s_net_buff*)net_buff_alloc( sizeof(struct s_net_buff) +
  		sizeof(struct s_arphdr) +
		(2 * arp1->ar_hln) + (2 * arp1->ar_pln) +
                sizeof( struct s_ethhdr));

  eth_build_header(buff, dev, 1, 0x0608);

  /*
   * Fill in the ARP REPLY packet.
   */
  arp2 = (struct s_arphdr *)buff->rowdata;
  ptr2 = ((unsigned char *) &arp2->ar_op) + sizeof(unsigned short);
  arp2->ar_hrd = arp1->ar_hrd;
  arp2->ar_pro = arp1->ar_pro;
  arp2->ar_hln = arp1->ar_hln;
  arp2->ar_pln = arp1->ar_pln;

  arp2->ar_op = htons(ARPOP_REPLY);

  memcpy(ptr2, dev->dev_addr, arp2->ar_hln);

  ptr2 += arp2->ar_hln;
  memcpy(ptr2, ptr1 + (arp1->ar_hln * 2) + arp1->ar_pln, arp2->ar_pln);

  ptr2 += arp2->ar_pln;
  memcpy(ptr2, ptr1, arp2->ar_hln);

  ptr2 += arp2->ar_hln;
  memcpy(ptr2, ptr1 + arp1->ar_hln, arp2->ar_pln);

  // transmit packet
  buff->eth = (struct s_ethhdr*)buff->data;

  arp_statistics.sended++;
  arp_statistics.sended_replies++;

  el3_start_xmit(dev, buff);
}

/***************************************************************************/
/* send an arp request
/***************************************************************************/
/* Input:  unsigned int daddr (IP address of destination (int))
/*         struct s_net_device *dev (Pointer to Device)
/***************************************************************************/
void arp_request(unsigned int daddr, struct s_net_device *dev)
{
  struct s_net_buff *buff;
  struct s_arphdr *arphdr;

  unsigned char *ptr1, *ptr2;


  /* Get some mem and initialize it for the return trip. */
  buff = (struct s_net_buff*)net_buff_alloc( sizeof(struct s_net_buff) +
  		sizeof(struct s_arphdr) +
		(2 * dev->ha_len) + (2 * dev->pa_len) +
                sizeof( struct s_ethhdr));

  eth_build_header(buff, dev, 0, 0x0608);

  /*
   * Fill in the ARP REQUEST packet.
   */

  arphdr = (struct s_arphdr*)(buff->data + sizeof(struct s_ethhdr));
  arphdr->ar_hrd = htons(ARPHRD_ETHER);
  arphdr->ar_pro = 0x0008;
  arphdr->ar_hln = dev->ha_len;
  arphdr->ar_pln = dev->pa_len;

  arphdr->ar_op = htons(ARPOP_REQUEST);

  ptr1 = ((unsigned char *) &arphdr->ar_op) + sizeof(unsigned short);

  // Insert Source Hardware Address
  memcpy(ptr1, dev->dev_addr, dev->ha_len);

  // Insert Source IP Address
  ptr1 += dev->ha_len;
  ptr2 = (unsigned char*)&dev->pa_addr;

  memcpy(ptr1, ptr2, dev->pa_len);

  // Insert Destination Hardware Address
  ptr1 += dev->pa_len;
  memset(ptr1, 0, dev->pa_len);

  // Insert Destination IP Address
  ptr2 = (unsigned char *) &daddr;
  ptr1 += dev->ha_len;
  memcpy(ptr1, ptr2, dev->pa_len);

  // transmit packet
  buff->eth = (struct s_ethhdr*)buff->data;

  arp_statistics.sended++;
  arp_statistics.sended_requests++;

  el3_start_xmit(dev, buff);
}

void arp_update_cache(struct s_net_buff *buff)
{
  int i;
  struct s_arpcache *arp_entry;
  struct s_arphdr *arp;
  unsigned char *ptr1, *ptr2;

  arp_entry = (struct s_arpcache*)valloc(sizeof(struct s_arpcache));
  if (arp_entry == NULL)
  {
    printk("arp.c: No mem left to add arp-cache entry.");
    return;
  }

  arp = (struct s_arphdr *)(buff->data + sizeof(struct s_ethhdr));
  ptr1 = ((unsigned char *) &arp->ar_op) + sizeof(unsigned short);

  // Insert Source Hardware address
  memcpy(arp_entry->ha_addr, ptr1, arp->ar_hln);
  ptr1 += arp->ar_hln;

  // Insert Source IP address
  memcpy(&arp_entry->pa_addr, ptr1, arp->ar_pln);

  // add entry to arp-cache-list
  arp_cache = (struct list*)add_item(arp_cache,arp_entry,sizeof(struct s_arpcache));

  arp_dump();
}

int arp_get_mac(unsigned int daddr, unsigned char *ha_addr)
{
  int i=0;

  struct s_arpcache *arp_entry=(struct s_arpcache*)get_item(arp_cache,i++);

  while (arp_entry != NULL)
  {
    if (daddr == arp_entry->pa_addr)
    {
      memcpy(ha_addr, arp_entry->ha_addr, 6);
      return 1;
    }
    arp_entry = (struct s_arpcache*)get_item(arp_cache,i++);
  }
  return 0;
}

void arp_rcv(struct s_net_buff *buff, struct s_net_device *dev)
{
  struct s_arphdr *arphdr;

  arp_statistics.received++;

  arphdr = (struct s_arphdr*)(buff->data + sizeof(struct s_ethhdr));

  switch (htons(arphdr->ar_op))
  {
    case ARPOP_REQUEST:
       {
          arp_statistics.received_requests++;
          arp_response(arphdr, dev);
          break;
       }
    case ARPOP_REPLY:
       {
          arp_statistics.received_replies++;
          arp_update_cache(buff);
          break;
       }
    default:
       {
          printk("arp.c: ARP Opcode %d not supported in this kernel.",htons(arphdr->ar_op));
          break;
       }
  }
}

void arp_init(void)
{
}

void arp_dump(void)
{
  unsigned int flags;
  int i=0;
  struct s_arpcache *arp_entry;

  save_flags(flags);
  cli();

  printk("Total ARP packets transmitted: %d", arp_statistics.sended);
  printk("   - ARP requests transmitted: %d", arp_statistics.sended_requests);
  printk("   - ARP replies  transmitted: %d", arp_statistics.sended_replies);
  printk("Total ARP packets received   : %d", arp_statistics.received);
  printk("   - ARP requests received   : %d", arp_statistics.received_requests);
  printk("   - ARP replies  received   : %d", arp_statistics.received_replies);
  printk("Arp Cache: (Hardware Address   IP Address)");
  arp_entry = (struct s_arpcache*)get_item(arp_cache,i++);

  while (arp_entry != NULL)
  {
    printk("%02X:%02X:%02X:%02X:%02X:%02X      %s",
      arp_entry->ha_addr[0],
      arp_entry->ha_addr[1],
      arp_entry->ha_addr[2],
      arp_entry->ha_addr[3],
      arp_entry->ha_addr[4],
      arp_entry->ha_addr[5],
      in_ntoa(arp_entry->pa_addr));
      arp_entry = (struct s_arpcache*)get_item(arp_cache,i++);
  }

  restore_flags(flags);
}
