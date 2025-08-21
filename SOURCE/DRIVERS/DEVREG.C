/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\devreg.c
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   The device manager. Register devices, show installed devices,
/*   autoirq_check,....
/************************************************************************/
#include "sched.h"
#include "head.h"
#include "devman.h"

#define NULL (void*)0

struct list *listhardware = NULL;
extern unsigned int memory_init;

unsigned int task_devreg = 0;
unsigned int hardware_id = 0;

struct s_hardware *installed_hardware = (void*)0;  // contains installed hardware

struct s_irq_usage irq_usage[16] = {0}; // contains irq of every hardware

unsigned int last_irq_request = 0;

extern void do_irq_detect_3();
extern void do_irq_detect_4();
extern void do_irq_detect_5();
extern void do_irq_detect_6();
extern void do_irq_detect_7();
extern void do_irq_detect_8();
extern void do_irq_detect_9();
extern void do_irq_detect_10();
extern void do_irq_detect_11();
extern void do_irq_detect_12();
extern void do_irq_detect_13();
extern void do_irq_detect_14();
extern void do_irq_detect_15();
extern void do_irq_detect_16();

/**************************************************************************/
/* Register a hardware.
/* name is the name of the hardware
/* irq: -1 no irq used
/**************************************************************************/
unsigned int register_hardware(unsigned int devclass, unsigned char *name, unsigned int driver_pid)
{
  struct s_hardware *phardware;

  if (!memory_init) return;

  phardware = (struct s_hardware*)valloc(sizeof(struct s_hardware));
  strcpy(phardware->name, name);
  phardware->irq = NULL;
  phardware->port = NULL;
  phardware->memory = NULL;
  phardware->dma = NULL;
  phardware->id = hardware_id++;
  phardware->driver_pid = driver_pid;
  phardware->devclass = devclass;

  phardware->configure_driver = 0;

  listhardware = (struct list*)add_item(listhardware,phardware,sizeof(struct s_hardware));

  return phardware->id;
}

int register_function(unsigned int id, unsigned int addr)
{
  int i=0;
  struct s_hardware *phardware = (struct s_hardware*)get_item(listhardware,i++);

  while (phardware!=NULL)
  {
    if (phardware->id == id) break;
    phardware=(struct s_hardware*)get_item(listhardware,i++);
  }
  if (phardware == NULL) return -1;

  phardware->configure_driver = addr;

  return 0;
}


int register_irq(unsigned int id, unsigned int irq)
{
  int i=0;
  int j=0;
  struct s_hardware *phardware = (struct s_hardware*)get_item(listhardware,i++);
  struct s_irq *pirq;

  while (phardware!=NULL)
  {
    if (phardware->id == id) break;
    phardware=(struct s_hardware*)get_item(listhardware,i++);
  }
  if (phardware == NULL) return -1;

  pirq = (struct s_irq*)get_item(phardware->irq, j++);
  while (pirq!=NULL)
  {
    if (pirq->nummer == irq) return -1;
    pirq=(struct s_irq*)get_item(phardware->irq,j++);
  }
  pirq = (struct s_irq*)valloc(sizeof(struct s_irq));
  pirq->nummer = irq;
  pirq->next = NULL;
  phardware->irq = (struct list*)add_item(phardware->irq,pirq,sizeof(struct s_irq));

  return 0;
}

int register_port(unsigned int id, unsigned int from, unsigned int to)
{
  int i=0;
  int j=0;
  struct s_hardware *phardware = (struct s_hardware*)get_item(listhardware,i++);
  struct s_port *pport;

  while (phardware!=NULL)
  {
    if (phardware->id == id) break;
    phardware=(struct s_hardware*)get_item(listhardware,i++);
  }
  if (phardware == NULL) return -1;

  pport = (struct s_port*)valloc(sizeof(struct s_port));
  pport->from = from;
  pport->to   = to;
  pport->next = NULL;
  phardware->port = (struct list*)add_item(phardware->port,pport,sizeof(struct s_port));

  return 0;
}

int register_memory(unsigned int id, unsigned int from, unsigned int to)
{
  int i=0;
  int j=0;
  struct s_hardware *phardware = (struct s_hardware*)get_item(listhardware,i++);
  struct s_memory *pm;

  while (phardware!=NULL)
  {
    if (phardware->id == id) break;
    phardware=(struct s_hardware*)get_item(listhardware,i++);
  }
  if (phardware == NULL) return -1;

  pm = (struct s_memory*)valloc(sizeof(struct s_memory));
  pm->from = from;
  pm->to   = to;
  pm->next = NULL;
  phardware->memory = (struct list*)add_item(phardware->memory,pm,sizeof(struct s_memory));

  return 0;
}

int register_dma(unsigned int id, unsigned int nummer)
{
  int i=0;
  int j=0;
  struct s_hardware *phardware = (struct s_hardware*)get_item(listhardware,i++);
  struct s_dma *pdma;

  while (phardware!=NULL)
  {
    if (phardware->id == id) break;
    phardware=(struct s_hardware*)get_item(listhardware,i++);
  }
  if (phardware == NULL) return -1;

  pdma = (struct s_dma*)valloc(sizeof(struct s_dma));
  pdma->nummer = nummer;
  pdma->next = NULL;
  phardware->dma = (struct list*)add_item(phardware->dma,pdma,sizeof(struct s_dma));

  return 0;
}

/**************************************************************************/
/* Checks wheter a irq is used from a device
/* irq: irq to check
/* hardwarename is returned
/* Return: -1 if not used, 0 if so.
/**************************************************************************/
signed char used_irq(unsigned char irq, unsigned char* hardwarename)
{
  if (irq_usage[irq].used == 0) return -1;

  strcpy(hardwarename, irq_usage[irq].hardware->name);
  return 0;
}

/**************************************************************************/
/* Prepare system to detect next irq request
/**************************************************************************/
void detect_irq(void)
{
  int i;
  unsigned char str[255];

  last_irq_request = 0;

  // set all unused interrupthandler to detect_handler
  for (i=0;i<16;i++)
  {
    if (used_irq(i,str) == -1)          // not used, set to detect_handle
    {
      switch (i)
      {
        case 3: set_intr_gate(0x20+i, do_irq_detect_3); break;
        case 4: set_intr_gate(0x20+i, do_irq_detect_4); break;
        case 5: set_intr_gate(0x20+i, do_irq_detect_5); break;
        case 6: set_intr_gate(0x20+i, do_irq_detect_6); break;
        case 7: set_intr_gate(0x20+i, do_irq_detect_7); break;
        case 8: set_intr_gate(0x20+i, do_irq_detect_8); break;
        case 9: set_intr_gate(0x20+i, do_irq_detect_9); break;
        case 10: set_intr_gate(0x20+i, do_irq_detect_10); break;
        case 11: set_intr_gate(0x20+i, do_irq_detect_11); break;
        case 12: set_intr_gate(0x20+i, do_irq_detect_12); break;
        case 13: set_intr_gate(0x20+i, do_irq_detect_13); break;
        case 14: set_intr_gate(0x20+i, do_irq_detect_14); break;
        case 15: set_intr_gate(0x20+i, do_irq_detect_15); break;
      }
    }
  }
}

unsigned char request_irq(void)
{
  return last_irq_request;
  last_irq_request = 0;
}




void devreg_task()
{
  struct ipc_message *m;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
     }
  }
}

int devreg_init(void)
{
  printk("devmem.c: Initializing device manager...");
  task_devreg = ((struct task_struct*)CreateKernelTask((unsigned int)devreg_task, "devicereg", HP,0))->pid;

  return 0;

}

void irq_detect_3(void)
{
  last_irq_request = 3;
  outportb(0x20,0x20);
}

void irq_detect_4(void)
{
  last_irq_request = 4;
  outportb(0x20,0x20);
}

void irq_detect_5(void)
{
  last_irq_request = 5;
  outportb(0x20,0x20);
}

void irq_detect_6(void)
{
  last_irq_request = 6;
  outportb(0x20,0x20);
}

void irq_detect_7(void)
{
  last_irq_request = 7;
  outportb(0x20,0x20);
}

void irq_detect_8(void)
{
  last_irq_request = 8;
  outportb(0x20,0x20);
}

void irq_detect_9(void)
{
  last_irq_request = 9;
  outportb(0x20,0x20);
}

void irq_detect_10(void)
{
  last_irq_request = 10;
  outportb(0x20,0x20);
}

void irq_detect_11(void)
{
  last_irq_request = 11;
  outportb(0x20,0x20);
}

void irq_detect_12(void)
{
  last_irq_request = 12;
  outportb(0x20,0x20);
}

void irq_detect_13(void)
{
  last_irq_request = 13;
  outportb(0x20,0x20);
}

void irq_detect_14(void)
{
  last_irq_request = 14;
  outportb(0x20,0x20);
}

void irq_detect_15(void)
{
  last_irq_request = 15;
  outportb(0x20,0x20);
}

void irq_detect_16(void)
{
  last_irq_request = 16;
  outportb(0x20,0x20);
}

