/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 -1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : net\ip\udp.c
/* Last Update: 11.12.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux, Etherlink III Parallel Tasking
/************************************************************************/
/* Definition:
/*   User Datagram Protocol Interface.
/************************************************************************/
#include "netdev.h"
#include "net.h"
#include "net_buff.h"
#include "ip.h"
#include "udp.h"
#include "socket.h"

unsigned int net_udp_debug =0;

struct s_udpfakehdr
{
	unsigned int saddr;
	unsigned int daddr;
	struct s_udphdr uh;
	char *data;
        unsigned int wcheck;
};

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

void udp_dump(struct s_udphdr *udp)
{
    printk("UDP Header:");
    printk("Sourceport: %d  Destinationport: %d",htons(udp->source), htons(udp->dest));
    printk("Length: %d   Checksum: %0004X",htons(udp->len), udp->check);
}

/*
 * computes a partial checksum, e.g. for TCP/UDP fragments
 */

unsigned int csum_partial(const unsigned char * buff, int len, unsigned int sum) {
	  /*
	   * Experiments with ethernet and slip connections show that buff
	   * is aligned on either a 2-byte or 4-byte boundary.  We get at
	   * least a 2x speedup on 486 and Pentium if it is 4-byte aligned.
	   * Fortunately, it is easy to convert 2-byte alignment to 4-byte
	   * alignment for the unrolled loop.
	   */
	__asm__("
	    testl $2, %%esi		# Check alignment.
	    jz 2f			# Jump if alignment is ok.
	    subl $2, %%ecx		# Alignment uses up two bytes.
	    jae 1f			# Jump if we had at least two bytes.
	    addl $2, %%ecx		# ecx was < 2.  Deal with it.
	    jmp 4f
1:	    movw (%%esi), %%bx
	    addl $2, %%esi
	    addw %%bx, %%ax
	    adcl $0, %%eax
2:
	    movl %%ecx, %%edx
	    shrl $5, %%ecx
	    jz 2f
	    testl %%esi, %%esi
1:	    movl (%%esi), %%ebx
	    adcl %%ebx, %%eax
	    movl 4(%%esi), %%ebx
	    adcl %%ebx, %%eax
	    movl 8(%%esi), %%ebx
	    adcl %%ebx, %%eax
	    movl 12(%%esi), %%ebx
	    adcl %%ebx, %%eax
	    movl 16(%%esi), %%ebx
	    adcl %%ebx, %%eax
	    movl 20(%%esi), %%ebx
	    adcl %%ebx, %%eax
	    movl 24(%%esi), %%ebx
	    adcl %%ebx, %%eax
	    movl 28(%%esi), %%ebx
	    adcl %%ebx, %%eax
	    lea 32(%%esi), %%esi
	    dec %%ecx
	    jne 1b
	    adcl $0, %%eax
2:	    movl %%edx, %%ecx
	    andl $0x1c, %%edx
	    je 4f
	    shrl $2, %%edx
	    testl %%esi, %%esi
3:	    adcl (%%esi), %%eax
	    lea 4(%%esi), %%esi
	    dec %%edx
	    jne 3b
	    adcl $0, %%eax
4:	    andl $3, %%ecx
	    jz 7f
	    cmpl $2, %%ecx
	    jb 5f
	    movw (%%esi),%%cx
	    leal 2(%%esi),%%esi
	    je 6f
	    shll $16,%%ecx
5:	    movb (%%esi),%%cl
6:	    addl %%ecx,%%eax
	    adcl $0, %%eax
7:	    "
	: "=a"(sum)
	: "0"(sum), "c"(len), "S"(buff)
	: "bx", "cx", "dx", "si");
	return(sum);
}

/*
 *	Fold a partial checksum
 */

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
 
/*
 * computes the checksum of the TCP/UDP pseudo-header
 * returns a 16-bit checksum, already complemented
 */

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

void udp_send(struct s_net_device *device, unsigned char* buffer, unsigned int size, struct s_socket *socket, struct s_sockaddr_in *daddr)
{
  struct s_net_buff *buff;
  struct s_udphdr *udphdr;
  struct s_udpfakehdr ufh;
  int check;

  buff = (struct s_net_buff*)ip_build_header(device->pa_addr, daddr->sa_addr,
		device, 17, (sizeof(struct s_udphdr)) + size, 0, 128);

  // Build udp header
  udphdr = (struct s_udphdr*)(buff->data + sizeof(struct s_ethhdr) + 20);

  udphdr->source = socket->port;
  udphdr->dest = daddr->sa_port;
  udphdr->len = htons(size + sizeof(struct s_udphdr));

  // Insert data to send
  memcpy((buff->data + sizeof(struct s_ethhdr) + 20 + sizeof(struct s_udphdr)), buffer, size);

  udphdr->check = 0;

  /* Calculate checksum of data and udpheader */
  check = 0;
  check = csum_partial((unsigned char*)udphdr, htons(udphdr->len),
 				   check);

  /* Calculate checksum of fake haeder and add data/udpheader csum */
  check = csum_tcpudp_magic(device->pa_addr, daddr->sa_addr,
					  htons(udphdr->len),
					  17, check);
  if (check == 0) check = -1;

  udphdr->check = check;

  if (net_udp_debug)
  {
    udp_dump(udphdr);
    mem_dump(udphdr, size + 8);
  }

  write_netdev(device->major, buff);
}

int udp_rcv(struct s_net_buff *buff, struct s_net_device *dev)
{
  struct s_iphdr   *iphdr;
  struct s_udphdr  *udphdr;
  struct s_sockaddr_in daddr;
  struct s_sockaddr_in saddr;
  struct s_udpfakehdr ufh;
  unsigned char*buffer;
  int checksum;
  int check;
  int size;
  unsigned int check2;

  iphdr = (struct s_iphdr*)(buff->data + sizeof(struct s_ethhdr));
  udphdr = (struct s_udphdr*)(buff->data + sizeof(struct s_ethhdr) +
        iphdr->ihl * 4);

  if (net_udp_debug)
    udp_dump(udphdr);

  checksum = udphdr->check;
  udphdr->check = 0;

  /* Calculate checksum of data and udpheader */
  check = 0;
  check = csum_partial((unsigned char*)udphdr, htons(udphdr->len),
 				   check);

  /* Calculate checksum of fake haeder and add data/udpheader csum */
  check = csum_tcpudp_magic(iphdr->saddr, iphdr->daddr,
					  htons(udphdr->len),
					  17, check);
  
  if (check != checksum)
  {
    alert("File: udp.c   Fucntion: udp_rcv      \n\n%s%0004X\n%s%0004X\n",
          "Received Checksum  : ", checksum,
          "Calculated Checksum: ", check);
    return 0;
  }

  udphdr->check = check; /* restore old checksum */

  daddr.sa_family = AF_INET;
  daddr.sa_port = udphdr->dest;
  daddr.sa_addr = iphdr->saddr;

  saddr.sa_family = AF_INET;
  saddr.sa_port = udphdr->source;
  saddr.sa_addr = iphdr->saddr;

  socket_received(&saddr, &daddr, (buff->data + sizeof(struct s_ethhdr) +
        iphdr->ihl * 4 + sizeof(struct s_udphdr)),htons(udphdr->len) - sizeof(struct s_udphdr));
}

void udp_init(void)
{
  printk("udp.c: Initializing UDP Protocol...");
}
