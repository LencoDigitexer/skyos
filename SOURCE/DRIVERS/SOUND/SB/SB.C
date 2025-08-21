/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 -1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\sound\sb.c
/* Last Update: 10.08.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux, Soundblaster Book, PC Underground
/************************************************************************/
/* Definition:
/*   A device driver for Sound Blaster compatible cards.
/* TODO:
/*   If an application plays sound and the applications gets terminated
/*   all allocated memory is freed.
/*   Because of this the sb driver has lost the buffer with the data
/*   to play. (DMA routine.)
/************************************************************************/
#include "devman.h"
#include "sched.h"
#include "head.h"

#define DSP_MINADDR        0x200
#define DSP_MAXADDR        0x280
#define DSP_RESET           6

#define MAX_DMA_SIZE       1000

#define DEBUG_ON 0
#define DEBUG if (DEBUG_ON == 1)

void sb_init();
int sb_probe();
void sb_write_dsp(int dsp_addr, unsigned char byte);
unsigned char sb_read_dsp(int dsp_addr);
void sb_get_dspversion(int dsp_addr, unsigned char *version);

static unsigned int sb_base_addr = 0;
static unsigned int sb_dma = 3;

static unsigned int sb_sound_size;            // size of current sound-file
static unsigned int sb_sound_offset;
static unsigned char* sb_sound_buffer;
static unsigned int sb_sound_currentsize;

static volatile unsigned int sb_intr = 0;

unsigned int sb_pid = 0;
static unsigned int sb_sound_pid = 0;

extern void do_irq_sb();

int sb_probe(unsigned int dma, unsigned int irq)
{
  int i, status, count = 0;
  int dsp_addr = DSP_MINADDR;
  int found = 0;
  unsigned char dsp_version[10];
  int id;

  while ((dsp_addr <= DSP_MAXADDR) && (!found))
  {
    outportb(dsp_addr+DSP_RESET, 1);

    for (i=0;i<100000;i++);

    outportb(dsp_addr+DSP_RESET, 0);

    i = 0;
    while ((status != 0xAA) && (i<100000))
    {
      status = inportb(dsp_addr+0x0E);
      status = inportb(dsp_addr+0x0A);
      i++;
    }

    if (status == 0xAA)
    {
      printk("sb.c: Soundblaster card found on port 0x%.3X", dsp_addr);
      sb_base_addr = dsp_addr;
      count++;
      found = 1;

      // register card, detect irq, dma channel 1?
      id = register_hardware(DEVICE_IDENT_MULTIMEDIA, "Soundblaster compatible sound card", 0);
      register_irq(id, irq);
      register_irq(id, dma);
      register_port(id, dsp_addr, dsp_addr + 0xF);

    }
    else
    {
      dsp_addr += 0x10;
    }
  }
  if (!count)
  {
     printk("sb.c: No Soundblaster card found");
     return;
  }

  sb_get_dspversion(dsp_addr, dsp_version);

  // volume to max

  outportb(sb_base_addr + 4, 0);
  for (i=0;i<=100000;i++);
  outportb(sb_base_addr + 5, 0);

  outportb(sb_base_addr + 4, 2);
  outportb(sb_base_addr + 5, 255);

  outportb(sb_base_addr + 4, 0x0E);
  outportb(sb_base_addr + 5, 255);

  outportb(sb_base_addr + 4, 0x22);
  outportb(sb_base_addr + 5, 255);

  outportb(sb_base_addr + 4, 0x26);
  outportb(sb_base_addr + 5, 255);

  outportb(sb_base_addr + 4, 0x2E);
  outportb(sb_base_addr + 5, 255);

  outportb(sb_base_addr + 4, 67);
  outportb(sb_base_addr + 5, 1);
  outportb(sb_base_addr + 4, 65);
  outportb(sb_base_addr + 5, 255);
  outportb(sb_base_addr + 4, 66);
  outportb(sb_base_addr + 5, 255);


  sb_write_dsp(sb_base_addr, 0xD1);

  sb_dma = dma;
  set_intr_gate(0x20 + irq,do_irq_sb);

  return count;
}

void sb_write_dsp(int dsp_addr, unsigned char byte)
{
  unsigned char status;
  int count = 0;

  // wait until DSP is ready for writing
  do
  {
    status = inportb(dsp_addr+0x0C);
    count++;
    if (count==100000)
       show_msgf("sb.c: DSP not ready for write.");
  } while (status >= 128);

  // write byte to DSP
  outportb(dsp_addr+0x0C, byte);
}

unsigned char sb_read_dsp(int dsp_addr)
{
  unsigned char byte;
  unsigned int count = 0;
  // wait until DSP is ready for reading
  do
  {
    byte = inportb(dsp_addr+0x0E);
    count++;
    if (count==100000)
       show_msgf("sb.c: DSP not ready for read.");
  }
  while (byte < 128);

  // read byte from DSP
  byte = inportb(dsp_addr+0x0A);

  return byte;
}


void sb_get_dspversion(int dsp_addr, unsigned char *version)
{
  unsigned int ver_major =0, ver_minor=0;

  sb_write_dsp(dsp_addr, 0xE1);           // 0xE1 .. version query

  ver_major = sb_read_dsp(dsp_addr);
  ver_minor = sb_read_dsp(dsp_addr);

  printk("sb.c: Base: 0x%x DSP Version is %d.%d", dsp_addr, ver_major, ver_minor);
  version[0] = ver_major;
  version[1] = ver_minor;
  version[2] = '\0';
}

void sb_write_direct(unsigned char value)
{
   sb_write_dsp(sb_base_addr, 0x10);
   sb_write_dsp(sb_base_addr, value);
}

void sb_play_click(void)
{
   unsigned char count;
   unsigned int i,j;

   return;
   if (sb_base_addr == 0) return;

     for (i=0;i<255;i++)
       sb_write_direct(i);
}


void sb_playblock_dma(unsigned char *data, unsigned int size)
{
  unsigned char *buffer;
  unsigned int i;

  // programm dma

  disable_dma(sb_dma);
  clear_dma_ff(sb_dma);

  set_dma_mode(sb_dma, 0x48);

  set_dma_addr(sb_dma, 0x8000);
  set_dma_count(sb_dma, size);

  // insert data
  buffer = (unsigned char*)0x8000;

  memcpy(buffer, data, size);

  sb_write_dsp(sb_base_addr, 0x14);
  sb_write_dsp(sb_base_addr,  (size-1)       & 0xFF);
  sb_write_dsp(sb_base_addr, ((size-1) >> 8) & 0xFF);

  enable_dma(sb_dma);
}

void sb_play_buffer(unsigned char *buf, unsigned int size)
{
   unsigned char *buffer;
   buffer = (unsigned char*)valloc(size);
   memcpy(buffer, buf, size);

   sb_sound_size   = size;
   sb_sound_buffer = buffer;
   sb_sound_offset = 0;

   sb_intr = 0;

   if (size < MAX_DMA_SIZE)
   {
     sb_playblock_dma(buffer, size);
     sb_sound_currentsize = size;
   }
   else
   {
     sb_playblock_dma(buffer, MAX_DMA_SIZE);
     sb_sound_currentsize = MAX_DMA_SIZE;
   }
}

void sb_interrupt(void)
{
   unsigned char status;
   struct ipc_message m;

   DEBUG show_msgf("sb.c: Intr. received.");
   DEBUG show_msgf("sb.c: Last block size: %d",sb_sound_currentsize);
   DEBUG show_msgf("sb.c: Offset: %d",sb_sound_offset);

   sb_sound_offset += sb_sound_currentsize;

   /* Acknowledge interrupt */
   status = inportb(sb_base_addr + 0x0E);

   if (sb_sound_offset != sb_sound_size)
   {
     if ((sb_sound_size - sb_sound_offset)  < MAX_DMA_SIZE)
     {
       sb_playblock_dma(sb_sound_buffer + sb_sound_offset, sb_sound_size - sb_sound_offset);
       sb_sound_currentsize = sb_sound_size - sb_sound_offset;
     }
     else
     {
       sb_playblock_dma(sb_sound_buffer + sb_sound_offset, MAX_DMA_SIZE);
       sb_sound_currentsize = MAX_DMA_SIZE;
     }
   }
   else
   {
     DEBUG show_msgf("sb.c: Sending fin....");
     m.type = MSG_SOUND_PLAY_FIN;
     send_msg(sb_sound_pid, &m);
   }

   outportb(0xA0, 0x20);
   outportb(0x20, 0x20);
}



void sb_task(void)
{
  struct ipc_message *m;
  unsigned int ret;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_SOUND_PLAY_BUFFER:
            sb_sound_pid = m->source->pid;
            DEBUG show_msgf("sb.c: Starting to playing sound.");
            sb_play_buffer(m->MSGT_SOUND_PLAY_BUF, m->MSGT_SOUND_PLAY_SIZE);
            break;
//       case
     }
  }
}

void sb_init(unsigned int dma, unsigned int irq)
{
  int res;
  int id;
  int count;

  printk("sb.c: Initializing Soundblaster.");
  printk("sb.c: Searching for base address, dma = %d, irq = %d",dma,irq);
  count = sb_probe(dma, irq);

  if (count > 0)
  {
    res = ((struct task_struct*)CreateKernelTask((unsigned int)sb_task, "sb", RTP,0))->pid;

    printk("sb.c: Soundblaster thread created. PID = %d",res);
    sb_pid = res;
  }
}

