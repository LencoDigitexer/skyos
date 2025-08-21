/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\net\3c509.c
/* Last Update: 01.12.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux, Etherlink III Parallel Tasking
/*              Adapter Drivers Technical Reference from
/*              Therry Murphy (and Donald Becker) (orig. 3com)
/************************************************************************/
/* Definition:
/*   Netcard Driver. Supports the Etherlink III 3c509 Netcard.
/* To Do:
/*   - Support Etherlink III 3c509 Netcards with Plug and Play.
/************************************************************************/
#include "devman.h"
#include "sched.h"
#include "head.h"
#include "netdev.h"
#include "twm.h"

// #define EL3_DEBUG 1

extern void do_irq_el3();

unsigned int net_eth_debug;

unsigned short packet_buffer[2000] = {0};

#define EL3_DATA 0x00
#define EL3_CMD 0x0e
#define EL3_STATUS 0x0e
#define ID_PORT 0x100
#define	EEPROM_READ 0x80

#define EL3WINDOW(win_num) outportw(ioaddr + EL3_CMD,0x0800+(win_num))

/* Register window 1 offsets, the window used in normal operation. */
#define TX_FIFO		0x00
#define RX_FIFO		0x00
#define RX_STATUS 	0x08
#define TX_STATUS 	0x0B
#define TX_FREE		0x0C		/* Remaining free bytes in Tx buffer. */

#define WN4_MEDIA	0x0A		/* Window 4: Various transceiver/media bits. */
#define  MEDIA_TP	0x00C0		/* Enable link beat and jabber for 10baseT. */

unsigned short id_read_eeprom(int index);
unsigned short read_eeprom(unsigned short ioaddr, int index);
static void el3_update_stats(int ioaddr, struct s_net_device *dev);

unsigned char dev_addr[7];

struct twm_window *netmonitor = NULL;

struct s_net_device *el3dev;
unsigned int el3_pid = 0;

struct s_net_device *active_device;

/* routine to swap two bytes of a word */
static __inline__ unsigned short int htons(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=q" (x)
		:  "0" (x));
	return x;
}

int el3_probe(struct s_net_device *dev)
{
  unsigned short lrs_state = 0xff;
  short i;
  unsigned short ioaddr, irq, if_port;
  static int current_tag = 0;
  int EISA_bus = 0;
  unsigned char str[255];
  short *phys_addr = (short *)dev_addr;
  unsigned int timeout = 0;
  int id;

  /* First check for a board on the EISA bus. */
  if (EISA_bus)
  {
	for (ioaddr = 0x1000; ioaddr < 0x9000; ioaddr += 0x1000)
	{
	   /* Check the standard EISA ID register for an encoded '3Com'. */
	   if (inportw(ioaddr + 0xC80) != 0x6d50)
				continue;

	   /* Change the register set to the configuration window 0. */
	   outportw(ioaddr + 0xC80 + EL3_CMD,0x0800);

	   irq = inportw(ioaddr + 8) >> 12;
	   if_port = inportw(ioaddr + 6)>>14;

	   goto el3_found;

  	   /* Restore the "Product ID" to the EEPROM read register. */
	   read_eeprom(ioaddr, 3);

	   /* Was the EISA code an add-on hack?  Nahhhhh... */
	}
        printk("3c509.c: No EISA Card found. Maybe no EISA Bus?");
   }

	/* Send the ID sequence to the ID_PORT. */
	outportb(ID_PORT, 0x00);
	outportb(ID_PORT, 0x00);
	for(i = 0; i < 255; i++)
	{
		outportb(ID_PORT, (unsigned char)lrs_state);
		lrs_state <<= 1;
		lrs_state = lrs_state & 0x100 ? lrs_state ^ 0xcf : lrs_state;
	}

	/* For the first probe, clear all board's tag registers. */
	if (current_tag == 0)
		outportb(ID_PORT, 0xd0);
	else				/* Otherwise kill off already-found boards. */
		outportb(ID_PORT, 0xd8);

	if (id_read_eeprom(7) != 0x6d50)
	{
           printk("3c509.c: No 3c509 adapter found.");
           return -1;
	}

	for (i = 0; i < 3; i++)
	  phys_addr[i] = htons(id_read_eeprom(i));

	{
	   unsigned short iobase = id_read_eeprom(8);
	   if_port = iobase >> 14;
	   ioaddr = 0x200 + ((iobase & 0x1f) << 4);

	}
	irq = id_read_eeprom(9) >> 12;

	/* Set the adaptor tag so that the next card can be found. */
	outportb(ID_PORT, 0xd0 + ++current_tag);

	/* Activate the adaptor at the EEPROM location. */
	outportb(ID_PORT, 0xff);

	EL3WINDOW(0);
        for (timeout = 0;timeout < 50000; timeout++);
	if (inportw(ioaddr) != 0x6d50)
        {
          printk("3c509.c: no adaptor found. Port: 0x%X  Manufacturer ID: 0x%X", ioaddr, inportw(ioaddr));
          printk("         Maybe your card is configured with Plug and Play.");
  	  return -1;
        }

el3_found:
	{
	  char *if_names[] = {"10baseT", "AUI", "undefined", "BNC"};

	  printk("3c509.c: 3Com Etherlink III 3C509 at 0x%x, IRQ: %d, %s port",ioaddr,irq,
	    if_names[if_port]);

          // print station address
	  sprintf(str,"%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x",
		dev_addr[0],
		dev_addr[1],
		dev_addr[2],
		dev_addr[3],
		dev_addr[4],
		dev_addr[5]);

          printk("         Station address: %s",str);

          sprintf(str,"3Com Etherlink III 3C509 %s",if_names[if_port]);
          id = register_hardware(DEVICE_IDENT_NETWORK, str, 0);
          register_irq(id, irq);
          register_port(id, ioaddr, ioaddr + 0xe);


      	 dev->ioaddr = ioaddr;
	 dev->irq = irq;
	 dev->if_port = if_port;
	 memcpy(dev->dev_addr, dev_addr, 7);

	}
	return 0;
}

unsigned short read_eeprom(unsigned short ioaddr, int index)
{
	int timer;

	outportw(ioaddr + 10, EEPROM_READ + index);
	/* Pause for at least 162 us. for the read to take place. */
	for (timer = 0; timer < 11620*4 + 400; timer++);
	return inportw(ioaddr + 12);
}

/* Read a word from the EEPROM when in the ISA ID probe state. */
unsigned short id_read_eeprom(int index)
{
	int bit, word = 0;
	int timer;

	/* Issue read command, and pause for at least 162 us. for it to complete.
	   Assume extra-fast 16Mhz bus. */
	outportb(ID_PORT,EEPROM_READ + index);

	/* This should really be done by looking at one of the timer channels. */
	for (timer = 0; timer < 3000000; timer++);

	for (bit = 15; bit >= 0; bit--)
		word = (word << 1) + (inportb(ID_PORT) & 0x01);

	return word;
}

void el3_open(unsigned int mode)
{
  int ioaddr = el3dev->ioaddr;
  int i;
  unsigned long timeout;
  unsigned short status;

  EL3WINDOW(0);

  // install IRQ request vector
  set_intr_gate(0x20+el3dev->irq, do_irq_el3);

  el3dev->tx_lock = 0;

  //printk("3c509.c: Opened device irq %d, window: %d  status: %x",dev->irq,
//    inportw(ioaddr + EL3_STATUS) >> 13,
//    inportw(ioaddr + EL3_STATUS) & 255);

  // activate board
  outportw(ioaddr + 4, 0x0001);

  // set irq line
  outportw(ioaddr + 8, (el3dev->irq << 12) | 0x0f00);

  EL3WINDOW(2);

  // set station address
  for (i=0;i<6;i++)
    outportb(ioaddr+i, el3dev->dev_addr[i]);

  if (el3dev->if_port == 3)
  {
    // start the thinnet transceiver.
    outportw(ioaddr + EL3_CMD, 0x1000);
  }
  else if (el3dev->if_port == 0)
  {
    // ethernet
    EL3WINDOW(4);
    outportw(ioaddr + WN4_MEDIA, inportw(ioaddr + WN4_MEDIA) | MEDIA_TP);
  }

  EL3WINDOW(1);

  outportw(ioaddr + EL3_CMD, 0x8005); // accept b-case and phys addr only
  outportw(ioaddr + EL3_CMD, 0xA800); // turn on statistics
  outportw(ioaddr + EL3_CMD, 0x2000); // enable receiver
  outportw(ioaddr + EL3_CMD, 0x4800); // enable transmitter
  outportw(ioaddr + EL3_CMD, 0x78ff); // allow all status bit to be seen
  outportw(ioaddr + EL3_CMD, 0x7098); // set interrupt mask

  // set interface in loopback mode
  el3dev->mode = mode;

  if (mode == 0x4000)  // enable ENDEC loopback, disable link beat
  {
    EL3WINDOW(4);

    status = inportw(ioaddr + 0x0A);
    status = status ^ 0x80;
    outportw(ioaddr + 0x0A, status);

    status = inportw(ioaddr + 0x06);
    status |= mode;
    outportw(ioaddr + 0x06, status);
  }
  else if (mode > 0)            // enable loopback mode
  {
    EL3WINDOW(4);

    status = inportw(ioaddr + 0x06);
    status |= mode;
    outportw(ioaddr + 0x06, status);
  }

  el3dev->ha_len = 6;
  el3dev->pa_len = 4;

  EL3WINDOW(1);
/*  printk("3c509.c: Opened Etherlink III device irq %d, window: %d  status: %x",el3dev->irq,
    inportw(ioaddr + EL3_STATUS) >> 13,
    inportw(ioaddr + EL3_STATUS) & 255);*/

/*   netmonitor = (struct twm_window *)valloc(sizeof(struct twm_window));
   netmonitor->x = 500;
   netmonitor->y = 100;
   netmonitor->length = 297;
   netmonitor->heigth = 300;
   netmonitor->acty = 10;
   strcpy(netmonitor->title, "Etherlink Netmonitor");
   draw_window(netmonitor);*/
}

int el3_rx(struct s_net_device *dev)
{
  unsigned int ioaddr = dev->ioaddr;

  unsigned short status;
  unsigned short rx_status;
  unsigned char str[255];
  struct s_net_buff *buff;

//  while ((rx_status = inportw(ioaddr + RX_STATUS)) > 0)
//  {
     rx_status = inportw(ioaddr + RX_STATUS);
     if (rx_status & 0x4000)                    /* Error, update stats. */
     {
	short error = rx_status & 0x3800;
	dev->stats.rx_errors++;

#ifdef EL3_DEBUG
        printk("3c509.c: rx_error occured. Error code: 0x%0004X.",error);
#endif

	switch (error)
	{
/*  	       case 0x0000:		lp->stats.rx_over_errors++; break;
	       case 0x0800:		lp->stats.rx_length_errors++; break;
	       case 0x1000:		lp->stats.rx_frame_errors++; break;
	       case 0x1800:		lp->stats.rx_length_errors++; break;
	       case 0x2000:		lp->stats.rx_frame_errors++; break;
	       case 0x2800:		lp->stats.rx_crc_errors++; break;*/
     	}
     }

     { // receive paket

       short pkt_len = rx_status & 0x7ff;
#ifdef EL3_DEBUG
       printk("3c509.c: el3_rx() receiving paket. Length: %dbytes.",pkt_len);
       printk("3c509.c: paket is %s, rx_status: %x.",(rx_status >> 15)?"incomplete":"complete",rx_status);
#endif

// read out RXFIFO data and add to net buffer
// First, allocate a net buffer entry
       buff = (struct s_net_buff*)net_buff_alloc(pkt_len);

// Read data from RXFIFO
       insl(ioaddr + RX_FIFO, buff->data, (pkt_len+3) >> 2);

       buff->eth = (struct s_ethhdr*)buff->data;
       buff->dev = dev;
       buff->size = pkt_len;
       net_buff_add(buff);

       if (net_eth_debug)
         mem_dump(buff->data, pkt_len);

/*       {
         unsigned char *pointer;
         unsigned short* p2;

         pointer = (unsigned char*)buff->data;
         pointer += 6;

         str[0] = *pointer;
         pointer++;
         str[1] = *pointer;
         pointer++;
         str[2] = *pointer;
         pointer++;
         str[3] = *pointer;
         pointer++;
         str[4] = *pointer;
         pointer++;
         str[5] = *pointer;
         pointer++;

         p2 = (unsigned short*)pointer;
         typ = *p2;

         sprintf(str,"%02X-%02X-%02X-%02X-%02X-%02X  %0004X  %d",str[0],str[1], str[2],
           str[3],str[4], str[5], htons(typ), pkt_len);
         out_window(netmonitor, str);
       }
       */

       outportw(ioaddr + EL3_CMD, 0x4000); /* Rx discard */

       while (inportw(ioaddr + EL3_STATUS) & 0x1000)
	  printk("3c509.c: Waiting for 3c509 to discard packet, status %x.\n",
							 inportw(ioaddr + EL3_STATUS) );
     }

  return 0;
}

/************************************************************************/
/* Interrupt Service Routine
/************************************************************************/
void el3_interrupt(void)
{
  unsigned int ioaddr;
  unsigned short status; // 16bit
  unsigned int ret = 0;

  ioaddr = el3dev->ioaddr;

  EL3WINDOW(1);

//  outportw(ioaddr + EL3_CMD, 0x7000); // command to disable all interrupts

//  outportw(ioaddr + EL3_CMD, 0x6801); // acknowledge Interrupt Latch

  if (el3dev->irq > 7)           // acknowledge IC (EOI)
    outportb(0xA0,0x20);

  else outportb(0x20,0x20);

  while ((status = inportw(ioaddr + EL3_STATUS)) & 0x01)
  {

#ifdef EL3_DEBUG
    printk("3c509.c: interrupt received. Status: 0x%0004X.",status);
#endif

    if (status & 0x10)            // RxComplete received
    {
#ifdef EL3_DEBUG
      printk("3c509.c: interrupt reason: RxComplete.");
#endif

      ret = el3_rx(el3dev);
    }

    if (status & 0x80)				/* Statistics full. */
    {
       // printk("3c509.c: updating statistics...");
       el3_update_stats(ioaddr, el3dev);
    }

    if (status & 0x04)                          /* TX Complete. */
    {
    		unsigned char tx_status = inportb(ioaddr + TX_STATUS);
#ifdef EL3_DEBUG
		printk("Complete: status %0004X, txstatus %02x.\n", status, tx_status);
#endif
    }

    outportw(ioaddr + EL3_CMD, 0x6891); /* Ack IRQ */

  }
}

static void el3_update_stats(int ioaddr, struct s_net_device *dev)
{
	/* Turn off statistics updates while reading. */
	outportw(ioaddr + EL3_CMD, 0xB000);

	/* Switch to the stats window, and read everything. */
	EL3WINDOW(6);
	dev->stats.tx_carrier_errors 	+= inportb(ioaddr + 0);
	dev->stats.tx_heartbeat_errors	+= inportb(ioaddr + 1);
	/* Multiple collisions. */	   	inportb(ioaddr + 2);
	dev->stats.collisions			+= inportb(ioaddr + 3);
	dev->stats.tx_window_errors		+= inportb(ioaddr + 4);
	dev->stats.rx_fifo_errors		+= inportb(ioaddr + 5);
	dev->stats.tx_packets			+= inportb(ioaddr + 6);
	dev->stats.rx_packets			+= inportb(ioaddr + 7);

	/* Tx deferrals */
	inportb(ioaddr + 8);
	inportw(ioaddr + 10);	/* Total Rx and Tx octets. */
	inportw(ioaddr + 12);

	/* Back to window 1, and turn statistics back on. */
	EL3WINDOW(1);
	outportw(ioaddr + EL3_CMD, 0xA800);
	return;
}


int el3_start_xmit(struct s_net_device *dev, struct s_net_buff* buff)
{
        unsigned short *p;
        int i;
        unsigned short status;
        int length;
        int pad;


	int ioaddr = dev->ioaddr;

        EL3WINDOW(1);

        if (dev->tx_lock == 1)
        {
           alert("File: 3c509.c  Function: el3_start_xmit\n\n%s%s",
                 "Out of Resources.\n",
                 "EL3 is already transmitting...");
           return;
        }

        dev->tx_lock = 1;

	/* Put out the doubleword header... */
	outportw(ioaddr + TX_FIFO, buff->data_length);
	outportw(ioaddr + TX_FIFO, 0x00);
	/* ... and the packet rounded to a doubleword. */

#ifdef EL3_DEBUG
       printk("3c509.c: transmitting %d bytes.",(buff->data_length +3) >>2 <<2);
#endif

       /* write data to TX_FIFO */
       outsl(ioaddr + TX_FIFO, buff->data, (buff->data_length + 3) >> 2);

#ifdef EL3_DEBUG
       printk("3c509.c: transmit done...");
#endif

       if (inportw(ioaddr + TX_FREE) > 1536)
       {
       }
       else
 	 outportw(ioaddr + EL3_CMD,0x9000 + 1536);

       /* Clear the Tx status stack. */
       {
		short tx_status;
		int i = 4;

		tx_status = inportb(ioaddr + TX_STATUS);
                status = inportw(ioaddr + 0x0E);
#ifdef EL3_DEBUG
		printk("el3_start_xmit(), status %0004X, txstatus %02x.\n", status, tx_status);
#endif
       }

       dev->tx_lock = 0;
}


void el3_close(void)
{
  int ioaddr = el3dev->ioaddr;


  /* Turn off statistics. */
//  printk("3c509.c: shuting down etherlink III netcard...");

  outportw(ioaddr + EL3_CMD, 0xB000);

  /* Disable the receiver and transmitter. */
  outportw(ioaddr + EL3_CMD, 0x1800);
  outportw(ioaddr + EL3_CMD, 0x5000);

  if (el3dev->if_port == 3)
	/* Turn off thinnet power. */
	outportw(ioaddr + EL3_CMD, 0xb800);

  else if (el3dev->if_port == 0)
  {
	/* Disable link beat and jabber, if_port may change ere next open(). */
        EL3WINDOW(4);
	outportw(ioaddr + WN4_MEDIA, inportw(ioaddr + WN4_MEDIA) & ~MEDIA_TP);
  }

  /* Switching back to window 0 disables the IRQ. */
  EL3WINDOW(0);

  /* But we explicitly zero the IRQ line select anyway. */
  outportw(ioaddr + 8, 0x0f00);

  if (netmonitor) vfree(netmonitor);
}

void el3_task(void)
{
  struct ipc_message *m;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
        case MSG_WRITE_BLOCK:
             {
               el3_start_xmit(el3dev, (struct s_net_buff*)m->MSGT_WRITE_BLOCK_BUFFER);
               break;
             }
     }
  }
}

void el3_init(void)
{
  int found = 0;

  el3dev = (struct s_net_device*)valloc(sizeof(struct s_net_device));
  if (!el3_probe(el3dev))
  {
    found = 1;
    el3_pid = ((struct task_struct*)CreateKernelTask((unsigned int)el3_task, "el3", RTP,0))->pid;
    el3dev->major = register_netdev("el3",el3_pid);
    el3_open(0);
  }

/* This must be done because other functions uses the active_device structure.
   If this structure isn't initialized, uuuhhh */

  active_device = el3dev;
  // if (!found) vfree(el3dev);
}

void el3_output(unsigned int bytes)
{
}

void el3_config(void)
{

  ///!!!!!
  ///OLD
  ///!!!!!
  struct twm_window w;
  unsigned short reg16;
  unsigned char reg8;
  unsigned int flags;
  struct s_net_device *dev;
  unsigned char str[255];
  unsigned short ioaddr;

  w.x = 10;
  w.y = 70;
  w.length = 500;
  w.heigth = 400;
  w.acty = 10;
  strcpy(w.title, "Etherlink III Netcard Configuration");
  draw_window(&w);

  save_flags(flags);
  cli();

  ioaddr = el3dev->ioaddr;

  switch (el3dev->mode)
  {
    case 0: sprintf(str,"Loopback Mode: Disabled"); break;
    case 0x1000: sprintf(str,"Loopback Mode: FIFO loopback"); break;
    case 0x2000: sprintf(str,"Loopback Mode: Ethernet controller loopback"); break;
    case 0x4000: sprintf(str,"Loopback Mode: ENDEC loopback"); break;
    case 0x8000: sprintf(str,"Loopback Mode: External loopback"); break;
    default: sprintf(str,"Loopback Mode: unknown?!?!"); break;
  }

  out_window(&w, str);

  // FIFO RX STATUS
  EL3WINDOW(1);
  reg16 = inportw(el3dev->ioaddr + 0x08);

  out_window(&w, "FIFO RX Status:");

  sprintf(str,"Complete : %s",(reg16 & 0x8000)?"RX packet incomplete or RX FIFO empty":"Complete");
  out_window(&w, str);
  sprintf(str,"Error    : %s",(reg16 & 0x4000)?"Error in RX packet":"no error or incomplete");
  out_window(&w, str);

  if (reg16 & 0x4000)
  {
   switch ((reg16 & 0x3800)>>11)
   {
    case 0x0: sprintf(str,"Error    : Overrun occured"); break;
    case 0x3: sprintf(str,"Error    : Runt Packer Error"); break;
    case 0x4: sprintf(str,"Error    : Alignment (Framing) Error"); break;
    case 0x5: sprintf(str,"Error    : CRC Error"); break;
    case 0x1: sprintf(str,"Error    : Oversized packet"); break;
    case 0x2: {
                if (reg16 & 0x4000)
                  sprintf(str,"Error    : Dribble Bit information"); break;
                break;
              }
    default: sprintf(str,"Error    : No error");
   }

   out_window(&w, str);
  }

  sprintf(str,"RX bytes : %d", reg16 & 0x07FF);
  out_window(&w, str);

  // FIFO TX STATUS
  reg8 = inportb(el3dev->ioaddr+ 0x0B);

  out_window(&w, "");
  out_window(&w, "FIFO TX Status:");

  sprintf(str,"Complete : %s",(reg8 & 0x80)?"Transmit Complete":"Transmit Incomplete");
  out_window(&w, str);
  sprintf(str,"Interrupt on successful transmit: %s",(reg8 & 0x40)?"Set":"Clear");
  out_window(&w, str);
  sprintf(str,"Jabber   : %s",(reg8 & 0x20)?"Set":"Clear");
  out_window(&w, str);
  sprintf(str,"Underrun : %s",(reg8 & 0x10)?"Set":"Clear");
  out_window(&w, str);
  sprintf(str,"Max. Collisions    : %s",(reg8 & 0x8)?"Set":"Clear");
  out_window(&w, str);
  sprintf(str,"TX Status Overflow : %s",(reg8 & 0x4)?"Set":"Clear");
  out_window(&w, str);
  sprintf(str,"TX Reclaim         : %s",(reg8 & 0x2)?"Set":"Clear");
  out_window(&w, str);

  // Show Net Diagnostic Register

  EL3WINDOW(4);
  reg16 = inportw(ioaddr + 0x0A);
  out_window(&w,"");
  out_window(&w,"Media Type and Status:");

  sprintf(str,"10Base-T        : %s",(reg16 & 0x8000)?"Enabled":"Disabled");
  out_window(&w, str);
  sprintf(str,"Coax trans.     : %s",(reg16 & 0x4000)?"Enabled":"Disabled");
  out_window(&w, str);
  sprintf(str,"SQE             : %s",(reg16 & 0x1000)?"Present":"Not present");
  out_window(&w, str);
  sprintf(str,"Valid link beat : %s",(reg16 & 0x0800)?"Detected":"not detected");
  out_window(&w, str);
  sprintf(str,"Polarity rev.   : %s",(reg16 & 0x0400)?"Detected":"not detected");
  out_window(&w, str);
  sprintf(str,"Jabber          : %s",(reg16 & 0x0200)?"Detected":"not detected");
  out_window(&w, str);
  sprintf(str,"Unsquelch       : %s",(reg16 & 0x0100)?"Set":"Clear");
  out_window(&w, str);
  sprintf(str,"Carrier Sense   : %s",(reg16 & 0x0020)?"Set":"Clear");
  out_window(&w, str);
  sprintf(str,"Collision       : %s",(reg16 & 0x0010)?"Set":"Clear");
  out_window(&w, str);

  out_window(&w,"");
  out_window(&w,"Statistics:");

  sprintf(str,"TX Carrier Errors   : %d", dev->stats.tx_carrier_errors);
  out_window(&w,str);
  sprintf(str,"TX Heartbeat Errors : %d", dev->stats.tx_heartbeat_errors);
  out_window(&w,str);
  sprintf(str,"Collisions          : %d", dev->stats.collisions);
  out_window(&w,str);
  sprintf(str,"TX Window Errors    : %d", dev->stats.tx_window_errors);
  out_window(&w,str);
  sprintf(str,"RX Fifo Errors      : %d", dev->stats.rx_fifo_errors);
  out_window(&w,str);
  sprintf(str,"TX Packets          : %d", dev->stats.tx_packets);
  out_window(&w,str);
  sprintf(str,"RX Packets          : %d", dev->stats.rx_packets);
  out_window(&w,str);
  EL3WINDOW(1);

  restore_flags(flags);
}

