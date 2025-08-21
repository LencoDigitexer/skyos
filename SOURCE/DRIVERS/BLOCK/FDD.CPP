/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\fdd.cpp
/* Last Update: 15.04.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the floppy disk device object.
/************************************************************************/
#include "devman.h"
#include "vdm.h"
#include "stdlib.h"
#include "devices.h"
#include "sched.h"
#include "fdd.h"

extern "C" void floppy_interrupt(void);

extern CVDM vdm;

#define MOTOR_OFF_DELAY 100*4            // 4 seconds
#define FD_STATUS 0x3f4
#define FD_DATA   0x3f5
#define FD_DOR    0x3f2
#define FD_DIR    0x3f7
#define FD_DCR    0x3f7

/* Bits of main status register */
#define STATUS_BUSYMASK         0x0F            /* drive busy mask */
#define STATUS_BUSY             0x10            /* FDC busy */
#define STATUS_DMA              0x20            /* 0- DMA mode */
#define STATUS_DIR              0x40            /* 0- cpu->fdc */
#define STATUS_READY            0x80            /* Data reg ready */

/* Values for FD_COMMAND */
#define FD_RECALIBRATE     0x07 /* move to track 0 */
#define FD_SEEK            0x0F /* seek track */
#define FD_READ            0xE6 /* read with MT, MFM, SKip deleted */
#define FD_WRITE           0xC5    /* write with MT, MFM */
#define FD_SENSEI          0x08     /* Sense Interrupt Status */
#define FD_SPECIFY         0x03     /* specify HUT etc */
#define FD_FORMAT          0x4D       /* format one track */
#define FD_VERSION         0x10      /* get version code */
#define FD_CONFIGURE       0x13   /* configure FIFO operation */
#define FD_PERPENDICULAR   0x12   /* perpendicular r/w mode */

#define NO_TRACK 255
#define MAX_REPLIES 7

/* drive geometries */
#define DG144_HEADS       2     /* heads per drive (1.44M) */
#define DG144_TRACKS     80     /* number of tracks (1.44M) */
#define DG144_SPT        18     /* sectors per track (1.44M) */
#define DG144_GAP3FMT  0x54     /* gap3 while formatting (1.44M) */
#define DG144_GAP3RW   0x1b     /* gap3 while reading/writing (1.44M) */

#define DG168_HEADS       2     /* heads per drive (1.68M) */
#define DG168_TRACKS     80     /* number of tracks (1.68M) */
#define DG168_SPT        21     /* sectors per track (1.68M) */
#define DG168_GAP3FMT  0x0c     /* gap3 while formatting (1.68M) */
#define DG168_GAP3RW   0x1c     /* gap3 while reading/writing (1.68M) */

/* IO ports */
#define FDC_DOR  (0x3f2)   /* Digital Output Register */
#define FDC_MSR  (0x3f4)   /* Main Status Register (input) */
#define FDC_DRS  (0x3f4)   /* Data Rate Select Register (output) */
#define FDC_DATA (0x3f5)   /* Data Register */
#define FDC_DIR  (0x3f7)   /* Digital Input Register (input) */
#define FDC_CCR  (0x3f7)   /* Configuration Control Register (output) */

/* command bytes (these are 765 commands + options such as MFM, etc) */
#define CMD_SPECIFY (0x03)  /* specify drive timings */
#define CMD_WRITE   (0xc5)  /* write data (+ MT,MFM) */
#define CMD_READ    (0xe6)  /* read data (+ MT,MFM,SK) */
#define CMD_RECAL   (0x07)  /* recalibrate */
#define CMD_SENSEI  (0x08)  /* sense interrupt status */
#define CMD_FORMAT  (0x4d)  /* format track (+ MFM) */
#define CMD_SEEK    (0x0f)  /* seek track */
#define CMD_VERSION (0x10)  /* FDC version */

char tmpbuf[512];
char reply_buffer[MAX_REPLIES] = {0};
int statsz = 0;
int dchange = 0;
int fdc_track = 0;
char status[MAX_REPLIES] = {0};
int reset = 0;
int mtick = 0;
int motor = 0;
int sr0 = 0;
char *buffer;
char current_track = 0;
char current_DOR   = 0x0C;
static volatile int done = 0;
static volatile int timed_out = 0;

unsigned int motor_off_timer = 0;

int floppy_pid = -1;

static volatile int intcount = 0;

#define NULL (void*)0
int fd_read_block(int blocknr, unsigned char *buffer);
int fd_write_block(int blocknr, unsigned char *buffer);

void CDiskFloppy::dma_setup(int write)
{
  unsigned int flags;

  save_flags(flags);
  cli();


  buffer = (unsigned char*)0x9000;

  disable_dma(2);
  clear_dma_ff(2);

  if (!write)
    set_dma_mode(2, 0x44);
  else
    set_dma_mode(2, 0x48);

  set_dma_addr(2, buffer);
  set_dma_count(2, 512);

  enable_dma(2);

  restore_flags(flags);
}

void CDiskFloppy::output_byte( char byte)
{
  int counter;
  unsigned char stat;

  if (reset)
    return;

  for (counter = 0; counter < 10000; counter++)
     {
      stat = inportb(FD_STATUS) & (STATUS_READY | STATUS_DIR);
      if (stat == STATUS_READY)
        {
         outportb(FD_DATA, byte);
         return;
        }
     }

  current_track = NO_TRACK;
  reset = 1;
  alert("File: floppy.c  Function: output_byte\n\n%s",
        "Unable to send byte to FDC.");
}

int CDiskFloppy::result(void)
{
        int i = 0, counter, status;

        if (reset)
          return -1;

        for (counter = 0 ; counter < 10000 ; counter++)
        {
           status = inportb(FD_STATUS)&(STATUS_DIR|STATUS_READY|STATUS_BUSY);

           if (status == STATUS_READY)
           {
              return i;
           }

           if (status == (STATUS_DIR|STATUS_READY|STATUS_BUSY)) {
             if (i >= MAX_REPLIES)
             {
               alert("File: floppy.c  Function: result\n\n%s",
                     "floppy_stat reply overrun\n");
               break;
           }
           reply_buffer[i++] = inportb(FD_DATA);
          }
        }
        reset = 1;
        current_track = NO_TRACK;
        alert("File: floppy.c  Function: result\n\n%s",
              "Getstatus times out\n");
        return -1;
}

int CDiskFloppy::getbyte()
{
    volatile int msr;
    int tmo;

    for (tmo = 0;tmo < 128;tmo++)
    {
      msr = inportb(FDC_MSR);
      if ((msr & 0xd0) == 0xd0)
      {
         return inportb(FDC_DATA);
      }
      inportb(0x80);   /* delay */
    }
    return -1;   /* read timeout */
}

int CDiskFloppy::waitfdc(int sensei)
{
    /* wait for IRQ6 handler to signal command finished */
    int tmout;

    if (!done)
    {
      /* sleep_and_wake_up(100ms) */
    }

//    if (!done)

    tmout = 1000000;
    while (!done && tmout)
      tmout--;

    /* read in command result bytes */
    statsz = 0;
    while ((statsz < 7) && (inportb(FDC_MSR) & (1<<4)))
    {
       status[statsz++] = getbyte();
    }

    if (sensei)
    {
      /* send a "sense interrupt status" command */
      output_byte(CMD_SENSEI);
      sr0 = getbyte();
      fdc_track = getbyte();
    }

    done = 0;
    if (!tmout)
    {
      /* timed out! */
      alert("File: floppy.c  Function: waitfdc\n\n%s",
            "timeout...");

      if (inportb(FDC_DIR) & 0x80)  /* check for diskchange */
        {
          alert("File: floppy.c  Function: waitfdc\n\n%s",
                "Diskchange detected. Reinitializing...");
          dchange = 1;
        }
       return 0;
    }
    else
      return 1;
}

void CDiskFloppy::motoron(void)
{
  int i;

  if (!motor)
  {
    mtick = -1;
    outportb(FDC_DOR, 0x1c);
    for (i=0;i<=100000;i++) asm("nop");
    motor = 1;
  }
}

void CDiskFloppy::motoroff(void)
{
  outportb(FDC_DOR, 0x0C);
  motor = 0;
}

void CDiskFloppy::prepare_motoroff(void)
{
   if (!motor_off_timer)
   {
     settimer(MOTOR_OFF_DELAY);
     motor_off_timer = 1;
   }
}

void CDiskFloppy::recalibrate(void)
{
   int rep = MAX_REPLIES;

   motoron();

   output_byte(CMD_RECAL);
   output_byte(0);

   done = 0;

   while (!(waitfdc(1)) && (rep--));

   prepare_motoroff();

}

int CDiskFloppy::seek(int track)
{
  int i;
  int sr0;

  motoron();

  output_byte(CMD_SENSEI);
  result();

  output_byte(CMD_SEEK);
  output_byte(0);
  output_byte(track);

  cli();
  if (!done)
    task_control(floppy_pid, TASK_SLEEPING);
  sti();

  while (!done);
  done = 0;

  output_byte(CMD_SENSEI);
  result();

  if (reply_buffer[0] != 0x20)
    return 0;
  else return 1;
}

void CDiskFloppy::resetcontroller(void)
{
  int rep = MAX_REPLIES;

  /* stop the motor, disable IRQ/DMA */
  outportb(FD_DOR, 0);

  mtick = 0;
  motor = 0;

  /* set data rate to 500K/s*/
  outportb(FDC_DRS, 0);

  /* re-enable interrupt */
  outportb(FDC_DOR, 0x0C);
  done = 0;

  while (!(waitfdc(1)) && (rep--));

  done = 0;
  output_byte(CMD_SPECIFY);
  output_byte(0xdf);
  output_byte(0x02);

  seek(1);
  recalibrate();

  done = 0;
}


void CDiskFloppy::block2hts(int block,int *head,int *track,int *sector)
{
    block--;
    *head = (block % (18 * 2)) / (18);
    *track = block / (18 * 2);
    *sector = block % 18 + 1;
}

unsigned int CDiskFloppy::fdc_rw(unsigned int block, char* blockbuff, int read)
{
  int head, track, sector, tries, i;
  unsigned char* dmabuffer;

  block2hts(block, &head, &track, &sector);

//  printk("Block %d = head %d at track %d and sector %d\n",block, head, track, sector);
  if (block == 0)
  {
      alert("File: floppy.c  Function: fdc_rw\n\n%s",
            "Invalid block nummer 0.");
      return -1;
  }
  motoron();

  for (tries = 0; tries < 3; tries ++)
  {
    if (inportb(FDC_DIR) & 0x80)
      {
       dchange = 1;
       seek(1);
       recalibrate();
/*       motoroff();
       printk("Drive change detected...\n");
       return 0;*/
       continue;
      }

    if (!seek(track))
      {
       prepare_motoroff();
       printk("File: floppy.c  Function: fdc_rw\n\n%s",
            "Seek error occured.");
       return 0;
      }

    outportb(FDC_CCR, 0); /* data rate to 500K/s */

    dmabuffer = (unsigned char*)0x9000;

    for (i=0;i<512;i++)
    {
      *dmabuffer = blockbuff[i];
      dmabuffer++;
    }


    if (read)
    {
       dma_setup(0);
       output_byte(CMD_READ);
    }
    else
    {
       dma_setup(1);
       output_byte(CMD_WRITE);
    }

    done = 0;
    output_byte(head << 2);
    output_byte(track);
    output_byte(head);
    output_byte(sector);
    output_byte(2);
    output_byte(18);
    output_byte(0x1b);
    output_byte(0xff);

    cli();
    if (!done)
      task_control(floppy_pid, TASK_SLEEPING);
    sti();

    while(!done);
    done = 0;
    result();

    if ((reply_buffer[0] & 0xc0) == 0) break; /* OK */

    recalibrate();
  }

  prepare_motoroff();

  dmabuffer = (unsigned char*)0x9000;

  for (i=0;i<512;i++)
     {
      blockbuff[i] = *dmabuffer;
      dmabuffer++;
     }

  return tries;
}

unsigned int CDiskFloppy::ReadBlock(unsigned int blocknr, char *buffer)
{
  if (fdc_rw(blocknr, buffer, 1) == 3)
    return -1;

  else return 1;
}

unsigned int CDiskFloppy::WriteBlock(unsigned int blocknr, char *buffer)
{
  memcpy(tmpbuf,buffer,512);

  if (fdc_rw(blocknr, tmpbuf, 0) == 3) return -1;
  else return 1;
}

void floppy_interrupt(void)
{
  if (floppy_pid != -1)
  {
    done = 1;

    task_control(floppy_pid, TASK_READY);
  }

  outportb(0x20,0x20);

}


void CDiskFloppy::Thread(void)
{
  struct ipc_message *m;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  
  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
        case MSG_READ_BLOCK:
             {
//                printk("fdd.cpp: MSG_READ_BLOCK received.");
				
				m->MSGT_READ_BLOCK_RESULT = ReadBlock(m->MSGT_READ_BLOCK_BLOCKNUMMER, m->MSGT_READ_BLOCK_BUFFER);
                m->type = MSG_READ_BLOCK_REPLY;
                send_msg(m->source->pid, m);
                break;
             }
        case MSG_WRITE_BLOCK:
             {
                m->MSGT_READ_BLOCK_RESULT = WriteBlock(m->MSGT_READ_BLOCK_BLOCKNUMMER, m->MSGT_READ_BLOCK_BUFFER);
                m->type = MSG_WRITE_BLOCK_REPLY;
                send_msg(m->sender, m);
                break;
             }
        case MSG_TIMER:
               cleartimer();
               motor_off_timer = 0;
               motoroff();
               break;

     }
   }
}

void fdd_thread(void)
{
	CDevice *d;

	d = vdm.GetRegistrationDevice("fd0001");		/* return the CDevice object from the virtual device manager */

	if (d == NULL)
	{
		printk("fdd.cpp: No fd0 device found. Ooops!!");
		printk("         fdd thread already created. Please unregister device");
		printk("         and kill thread.");
		while(1);
	}
	d->Thread();
}


CDiskFloppy::CDiskFloppy() : CBlockDevice()
{
}


/************************************************************************************/
/* This function is called from the virtual device manager to initialize the object.
/************************************************************************************/
/* What this fucntion must do:
/*   1. Create a Thread for the device
/*   2. Set the name of the device object
/************************************************************************************/
unsigned int CDiskFloppy::Initialize(void)
{
  int id;

  printk("fdd.cpp: Initializing Floppy device...");

  // C: register_blkdev(DEVICE_FLOPPY_MAJOR, DEVICE_FLOPPY_NAME, floppy_);

  outportb(FD_DOR, current_DOR);

  /* get the floppy controller version */
  output_byte(FD_VERSION);
  int res = result();
  if (res != 1)
    {
       printk("File: fdd.cpp  Function: fdc_rw, FDC failed to return version byte.");
    }
    else
    {
      printk("fdd.cpp: FDC version 0x%x\n",reply_buffer[0]);
      floppy_pid = ((struct task_struct*)CreateKernelTask(fdd_thread, "fdd", RTP,0))->pid;

      if (reply_buffer[0] == 0x80)
      {
        printk("fdd.cpp: NEC765 controller found.\n");
        id = register_hardware(DEVICE_IDENT_STORAGE_DEVICE, "Floppy Controller NEC765", floppy_pid);
     }
      else
      {
        printk("fdd.cpp: enhanced controller found.\n");
        id = register_hardware(DEVICE_IDENT_STORAGE_DEVICE, "Floppy Enhanced Controller", floppy_pid);
      }

	  
	  
	  SetPID(floppy_pid);
	  SetRegistrationName("fd0001");
          vdm.RegisterDevice(this);

	  vdm.AddName(this, "fd0",0);

      register_irq(id, 6);
      register_dma(id, 2);
      register_port(id, 0x3F2, 0x3F7);

      register_blkdev(DEVICE_FLOPPY_MAJOR, "fd0", floppy_pid);

    }

  mtick = motor = 0;
  done = 0;
}
