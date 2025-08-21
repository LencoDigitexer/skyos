/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\net\ne.c
/* Last Update: 04.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus from : Acrobat NIC Documentation, Linux
/************************************************************************/
/* Definition:
/*   Netcard Driver. Supports all NE2000 compatible netcards.
/************************************************************************/
#define E8390_TX_IRQ_MASK 0xa	/* For register EN0_ISR */
#define E8390_RX_IRQ_MASK  0x5
#define E8390_RXCONFIG 0x4	/* EN0_RXCR: broadcasts, no multicast,errors */
#define E8390_RXOFF 0x20	/* EN0_RXCR: Accept no packets */
#define E8390_TXCONFIG 0x00	/* EN0_TXCR: Normal transmit mode */
#define E8390_TXOFF 0x02	/* EN0_TXCR: Transmitter off */

/*  Register accessed at EN_CMD, the 8390 base addr.  */
#define E8390_STOP	0x01	/* Stop and reset the chip */
#define E8390_START	0x02	/* Start the chip, clear reset */
#define E8390_TRANS	0x04	/* Transmit a frame */
#define E8390_RREAD	0x08	/* Remote read */
#define E8390_RWRITE	0x10	/* Remote write  */
#define E8390_NODMA	0x20	/* Remote DMA */
#define E8390_PAGE0	0x00	/* Select page chip registers */
#define E8390_PAGE1	0x40	/* using the two high-order bits */
#define E8390_PAGE2	0x80	/* Page 3 is invalid. */

#define E8390_CMD	0x00	/* The command register (for all pages) */
/* Page 0 register offsets. */
#define EN0_CLDALO	0x01	/* Low byte of current local dma addr  RD */
#define EN0_STARTPG	0x01	/* Starting page of ring bfr WR */
#define EN0_CLDAHI	0x02	/* High byte of current local dma addr  RD */
#define EN0_STOPPG	0x02	/* Ending page +1 of ring bfr WR */
#define EN0_BOUNDARY	0x03	/* Boundary page of ring bfr RD WR */
#define EN0_TSR		0x04	/* Transmit status reg RD */
#define EN0_TPSR	0x04	/* Transmit starting page WR */
#define EN0_NCR		0x05	/* Number of collision reg RD */
#define EN0_TCNTLO	0x05	/* Low  byte of tx byte count WR */
#define EN0_FIFO	0x06	/* FIFO RD */
#define EN0_TCNTHI	0x06	/* High byte of tx byte count WR */
#define EN0_ISR		0x07	/* Interrupt status reg RD WR */
#define EN0_CRDALO	0x08	/* low byte of current remote dma address RD */
#define EN0_RSARLO	0x08	/* Remote start address reg 0 */
#define EN0_CRDAHI	0x09	/* high byte, current remote dma address RD */
#define EN0_RSARHI	0x09	/* Remote start address reg 1 */
#define EN0_RCNTLO	0x0a	/* Remote byte count reg WR */
#define EN0_RCNTHI	0x0b	/* Remote byte count reg WR */
#define EN0_RSR		0x0c	/* rx status reg RD */
#define EN0_RXCR	0x0c	/* RX configuration reg WR */
#define EN0_TXCR	0x0d	/* TX configuration reg WR */
#define EN0_COUNTER0	0x0d	/* Rcv alignment error counter RD */
#define EN0_DCFG	0x0e	/* Data configuration reg WR */
#define EN0_COUNTER1	0x0e	/* Rcv CRC error counter RD */
#define EN0_IMR		0x0f	/* Interrupt mask reg WR */
#define EN0_COUNTER2	0x0f	/* Rcv missed frame error counter RD */

/* Bits in EN0_ISR - Interrupt status register */
#define ENISR_RX	0x01	/* Receiver, no error */
#define ENISR_TX	0x02	/* Transmitter, no error */
#define ENISR_RX_ERR	0x04	/* Receiver, with error */
#define ENISR_TX_ERR	0x08	/* Transmitter, with error */
#define ENISR_OVER	0x10	/* Receiver overwrote the ring */
#define ENISR_COUNTERS	0x20	/* Counters need emptying */
#define ENISR_RDC	0x40	/* remote dma complete */
#define ENISR_RESET	0x80	/* Reset completed */
#define ENISR_ALL	0x3f	/* Interrupts we will enable */

/* Bits in EN0_DCFG - Data config register */
#define ENDCFG_WTS	0x01	/* word transfer mode selection */

/* Page 1 register offsets. */
#define EN1_PHYS   0x01	/* This board's physical enet addr RD WR */
#define EN1_CURPAG 0x07	/* Current memory page RD WR */
#define EN1_MULT   0x08	/* Multicast filter mask array (8 bytes) RD WR */

/* Bits in received packet status byte and EN0_RSR*/
#define ENRSR_RXOK	0x01	/* Received a good packet */
#define ENRSR_CRC	0x02	/* CRC error */
#define ENRSR_FAE	0x04	/* frame alignment error */
#define ENRSR_FO	0x08	/* FIFO overrun */
#define ENRSR_MPA	0x10	/* missed pkt */
#define ENRSR_PHY	0x20	/* physical/multicase address */
#define ENRSR_DIS	0x40	/* receiver disable. set in monitor mode */
#define ENRSR_DEF	0x80	/* deferring */

/* Transmitted packet status, EN0_TSR. */
#define ENTSR_PTX 0x01	/* Packet transmitted without error */
#define ENTSR_ND  0x02	/* The transmit wasn't deferred. */
#define ENTSR_COL 0x04	/* The transmit collided at least once. */
#define ENTSR_ABT 0x08  /* The transmit collided 16 times, and was deferred. */
#define ENTSR_CRS 0x10	/* The carrier sense was lost. */
#define ENTSR_FU  0x20  /* A "FIFO underrun" occured during transmit. */
#define ENTSR_CDH 0x40	/* The collision detect "heartbeat" signal was lost. */
#define ENTSR_OWC 0x80  /* There was an out-of-window collision. */

#define NE_CMD	 	0x00
#define NE_DATAPORT	0x10	/* NatSemi-defined port window offset. */
#define NE_RESET	0x1f	/* Issue a read to reset, a write to clear. */

#define NE1SM_START_PG	0x20	/* First page of TX buffer */
#define NE1SM_STOP_PG 	0x40	/* Last page +1 of RX ring */
#define NESM_START_PG	0x40	/* First page of TX buffer */
#define NESM_STOP_PG	0x80	/* Last page +1 of RX ring */

extern int timerticks;

int ne_probe(unsigned int ioaddr)
{
  unsigned int i;
  unsigned int regd;
  unsigned int reg0;

  char *name;

  int start_page, stop_page;
  unsigned char SA_prom[32];
  int wordlength = 2;
  int neX000, ctron, dlink, dfi;
  unsigned int irq;


  //printk("Probing for NE*000 card at 0x%0x....\n",ioaddr);

  reg0 = inportb(ioaddr);
  if (reg0 == 0xFF)
    return -1;

  /* Do a quick preliminary check that we have a 8390. */

  outportb(ioaddr + E8390_CMD, E8390_NODMA+E8390_PAGE1+E8390_STOP);
  regd = inportb(ioaddr + 0x0d);
  outportb(ioaddr + 0x0d,0xff);
  outportb(ioaddr + E8390_CMD,E8390_NODMA+E8390_PAGE0);

  inportb(ioaddr + EN0_COUNTER0); /* Clear the counter by reading. */

  if (inportb(ioaddr + EN0_COUNTER0) != 0)
  {
      outportb(ioaddr,reg0);
      outportb(ioaddr + 0x0d, regd);	/* Restore the old values. */
      return -1;
  }

  /* Read the 16 bytes of station address prom, returning 1 for
  an eight-bit interface and 2 for a 16-bit interface.
  We must first initialize registers, similar to NS8390_init(eifdev, 0).
  We can't reliably read the SAPROM address without this.
  (I learned the hard way!). */
    {
	struct {unsigned char value, offset; } program_seq[] = {
	    {E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD}, /* Select page 0*/
	    {0x48,	EN0_DCFG},	/* Set byte-wide (0x48) access. */
	    {0x00,	EN0_RCNTLO},	/* Clear the count regs. */
	    {0x00,	EN0_RCNTHI},
	    {0x00,	EN0_IMR},	/* Mask completion irq. */
	    {0xFF,	EN0_ISR},
	    {E8390_RXOFF, EN0_RXCR},	/* 0x20  Set to monitor */
	    {E8390_TXOFF, EN0_TXCR},	/* 0x02  and loopback mode. */
	    {32,	EN0_RCNTLO},
	    {0x00,	EN0_RCNTHI},
	    {0x00,	EN0_RSARLO},	/* DMA starting at 0x0000. */
	    {0x00,	EN0_RSARHI},
	    {E8390_RREAD+E8390_START, E8390_CMD},
	};
	for (i = 0; i < sizeof(program_seq)/sizeof(program_seq[0]); i++)
	    outportb(ioaddr + program_seq[i].offset, program_seq[i].value);
    }
    for(i = 0; i < 32 /*sizeof(SA_prom)*/; i+=2) {
	SA_prom[i] = inportb(ioaddr + NE_DATAPORT);
	SA_prom[i+1] = inportb(ioaddr + NE_DATAPORT);
	if (SA_prom[i] != SA_prom[i+1])
	    wordlength = 1;
    }

    if (wordlength == 2) {
	/* We must set the 8390 for word mode. */
	outportb(ioaddr + EN0_DCFG, 0x49);
	/* We used to reset the ethercard here, but it doesn't seem
	   to be necessary. */
	/* Un-double the SA_prom values. */
	for (i = 0; i < 16; i++)
	    SA_prom[i] = SA_prom[i+i];
    }

    neX000 = (SA_prom[14] == 0x57  &&  SA_prom[15] == 0x57);
    ctron =  (SA_prom[0] == 0x00 && SA_prom[1] == 0x00 && SA_prom[2] == 0x1d);
    dlink =  (SA_prom[0] == 0x00 && SA_prom[1] == 0xDE && SA_prom[2] == 0x01);
    dfi   =  (SA_prom[0] == 'D' && SA_prom[1] == 'F' && SA_prom[2] == 'I');

    /* Set up the rest of the parameters. */
    if (neX000 || dlink || dfi) {
	if (wordlength == 2) {
	    name = dlink ? "DE200" : "NE2000";
	    start_page = NESM_START_PG;
	    stop_page = NESM_STOP_PG;
	} else {
	    name = dlink ? "DE100" : "NE1000";
	    start_page = NE1SM_START_PG;
	    stop_page = NE1SM_STOP_PG;
	}
    } else if (ctron) {
	name = "Cabletron";
	start_page = 0x01;
	stop_page = (wordlength == 2) ? 0x40 : 0x20;
    } else {
	printk(" not found.\n");
	return -1;
    }

  printk("ne.c: %s compatible using %dbit\n",name,wordlength * 8);

  // detect irq line

  detect_irq();
  outportb(ioaddr + EN0_IMR, 0x50);	/* Enable one interrupt. */
  outportb(ioaddr + EN0_RCNTLO, 0x00);
  outportb(ioaddr + EN0_RCNTHI, 0x00);
  outportb(ioaddr, E8390_RREAD+E8390_START ); /* Trigger it... */
  outportb(ioaddr + EN0_IMR, 0x00); 		/* Mask it again. */
  for (i=0;i<=100000;i++);

  irq = request_irq();
  printk("      Automatic IRQ detection. IRQ: %d",irq);

  register_hardware("NE2000 compatible Netcard",irq);

  return 0;
}

static void
ne_reset_8390(unsigned int ioaddr)
{
    int tmp = inportb(ioaddr + NE_RESET);
    int reset_start_time = timerticks;

    outportb(ioaddr + NE_RESET, tmp);
    /* This check _should_not_ be necessary, omit eventually. */
    while ((inportb(ioaddr+EN0_ISR) & ENISR_RESET) == 0)
	if (timerticks - reset_start_time > 2)
	{
	    printk("ne.c: ne_reset_8390() did not complete.");
	    break;
	}
}

void ne_init(void)
{
  unsigned int port = 0x200;

  for (port = 0x200; port <= 0x300; port+=0x10)
  {
    if (!ne_probe(port))
    {
      ne_reset_8390(port);
      break; // ne*000 found
    }
  }

}


