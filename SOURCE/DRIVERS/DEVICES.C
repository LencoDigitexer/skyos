/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\devices.c
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   A standard interface to communicate with the device drivers.
/*   Functions to register a device, read/write from/to them,....
/************************************************************************/
//  OUT OF DATE
#include "devices.h"
#include "sched.h"
#include "error.h"
#include "netdev.h"

unsigned int cache_enabled = 0;
extern int TASK_VFS;

struct s_devices devices[MAX_DEVICES] = {0};

struct s_devices net_devices[MAX_DEVICES] = {0};
unsigned int net_devices_counter = 0;
unsigned int devices_major = 1;

/**************************** Net devices ****************************/
int register_netdev(unsigned char*name, int pid)
{
   net_devices[net_devices_counter].major = net_devices_counter;
   net_devices[net_devices_counter].pid = pid;
   strcpy(net_devices[net_devices_counter].name,name);
   net_devices_counter++;

   return (net_devices_counter-1);
}

void write_netdev(unsigned int major, struct s_net_buff *buff)
{
  struct ipc_message m;

  m.type = MSG_WRITE_BLOCK;

  m.MSGT_WRITE_BLOCK_BUFFER = buff;

  send_msg(net_devices[major].pid, &m);

  return SUCCESS;
}

/**************************** Block devices **************************/
void register_blkdev(int major, unsigned char* name, int pid)
{
  devices[major].major = devices_major;
  devices[major].pid = pid;
  strcpy(devices[major].name, name);
  devices_major++;
}

int read_blkdev(struct ipc_message *m)
{
  int major;
  int blocknr;
  unsigned char buffer[512];

  major = name2device(m->MSGT_READ_BLOCK_DEVICENAME);
  blocknr = m->MSGT_READ_BLOCK_BLOCKNUMMER;

  if (cache_enabled)
  {
    // Is Block in Cache?
    if (cache_hit(major, blocknr, buffer))
    {
      memcpy(m->MSGT_READ_BLOCK_BUFFER, buffer, 512);
      m->type = MSG_READ_BLOCK_CACHE_REPLY;
      send_msg(TASK_VFS, m);
      return SUCCESS;
    }
  }

  m->MSGT_READ_BLOCK_MINOR = major;
  send_msg(devices[major].pid, m);

  return SUCCESS;
}

int write_blkdev(struct ipc_message *m)
{
  int major;
  int blocknr;
  unsigned char buffer[512];

  major = name2device(m->MSGT_WRITE_BLOCK_DEVICENAME);
  blocknr = m->MSGT_WRITE_BLOCK_BLOCKNUMMER;

  m->MSGT_WRITE_BLOCK_MINOR = major;

  send_msg(devices[major].pid, m);

  return SUCCESS;
}

/**************************** Character Devices **************************/

void register_device(int major, unsigned char* name, int pid)
{
  devices[major].major = devices_major;
  devices[major].pid = pid;
  strcpy(devices[major].name, name);
  devices_major++;
}

int read_chardev(struct ipc_message *m)
{
  int ret;
  int major;
  unsigned char* buffer;

  major = name2device(m->MSGT_READ_CHAR_DEVICENAME);

//  m->source->waitingfor = devices[major].pid;

  vfree(m->MSGT_READ_BLOCK_DEVICENAME);

  send_msg(devices[major].pid, m);

  return ret;
}

int name2device(unsigned char *name)
{
  int i;

  for (i=0;i<MAX_DEVICES;i++)
    if (!strcmp(name,devices[i].name)) return i;
  for (i=0;i<MAX_DEVICES;i++)
    if (!strcmp(name,net_devices[i].name)) return i;

  return -1;
}

int device_getmajor(unsigned int i)
{
  return devices[i].major;
}


int name2task(unsigned char *name)
{
  int i;

  for (i=0;i<MAX_DEVICES;i++)
    if (!strcmp(name,devices[i].name)) return devices[i].pid;
  for (i=0;i<MAX_DEVICES;i++)
    if (!strcmp(name,net_devices[i].name)) return net_devices[i].pid;

  return -1;
}

void devices_dump(void)
{
   int i;

   printk("Dumping device status: \n\n");


   printk("--------------------------------------------------------------------------");
   printk("Major  |  Name");
   printk("--------------------------------------------------------------------------");
   for (i = 0; i< MAX_DEVICES; i++)
   {
     printk("%00005d  |  %-20s",
            devices[i].major, devices[i].name
/*            devices[i].fops->open,
            devices[i].fops->read,
            devices[i].fops->write*/
            );
   }
}

