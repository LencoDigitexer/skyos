/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : fs\sub\msdos\msdos.c
/* Last Update: 04.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus from : SKY V1.0, PC Intern 5
/************************************************************************/
/* Definition:
/*   The MSDOS - Sub Filesystem                                         */
/*   Supports: FAT12/16
/*
/* To Do:
/*   - Support of FAT32
/*   - Cluster support
/************************************************************************/
#include "rtc.h"
#include "error.h"
#include "ctype.h"
#include "sched.h"
#include "vfs.h"

//#define MSDOS_S superblock->msdos
#define EOF 0

// enable this for real time debugging
#define MSDOS_DEBUG 1

void msdos_task()
{
  struct ipc_message *m;
  struct inode *in;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
        case MSG_MOUNT:
             {
               m->MSGT_MOUNT_RESULT = msdos_mount(m->MSGT_MOUNT_DEVICE,m->MSGT_MOUNT_ROOTINODE,m->MSGT_MOUNT_SUPERBLOCK);
               m->type = MSG_MOUNT_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_GET_INODE:
             {
               m->MSGT_GET_INODE_RESULT = msdos_get_inode(m->MSGT_GET_INODE_DEVICE,m->MSGT_GET_INODE_SUPERBLOCK,m->MSGT_GET_INODE_ACTINODE,m->MSGT_GET_INODE_INODE,m->MSGT_GET_INODE_NUMBER);
               m->type = MSG_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_LOOKUP_INODE:
             {
               m->MSGT_GET_INODE_RESULT = msdos_lookup_inode(m->MSGT_GET_INODE_DEVICE,m->MSGT_GET_INODE_SUPERBLOCK,m->MSGT_GET_INODE_ACTINODE,m->MSGT_GET_INODE_INODE,m->MSGT_GET_INODE_NAME);
               m->type = MSG_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_LOOKUP:
             {
               m->MSGT_LOOKUP_RESULT = msdos_lookup(m->MSGT_LOOKUP_DEVICE,m->MSGT_LOOKUP_SUPERBLOCK,m->MSGT_LOOKUP_ACTINODE,m->MSGT_LOOKUP_INODE,m->MSGT_LOOKUP_NAME);
               m->type = MSG_LOOKUP_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_FREAD:
             {
               in = (struct inode*)m->MSGT_FREAD_INODE;

               printk("msdos.c: reading %s from device at %d",in->name, in->direct[0]);
               m->type = MSG_FREAD_REPLY;
               m->MSGT_FREAD_RESULT = SUCCESS;
               send_msg(m->sender, m);
               break;
             }
     }
  }
}

int msdos_init()
{
   int pid;

   printk("msdos.c: Initializing MS-DOS filesystem.");
   pid = ((struct task_struct*)CreateKernelTask((unsigned int)msdos_task, "msdos", HP,0))->pid;

   register_protocol("msdos",pid);

   return SUCCESS;
}

int msdos_mount(unsigned char *device, struct inode *rootinode, struct msdos_super_block *superblock)
{
  unsigned char buffer[512];

#ifdef MSDOS_DEBUG
  printk("msdos_mount: device = %s",device);
#endif

  if (readblock(device, 1, buffer))
  {
    return ERROR_READ_BLOCK;
  }

  if (strncmp(&buffer[0x36],"FAT12",5))
    if (strncmp(&buffer[0x36],"FAT16",5))
      if (strncmp(&buffer[0x52],"FAT32",5))
        return ERROR_WRONG_PROTOCOL;

  memcpy(superblock->name, buffer + 0x03, 8);
  memcpy(&(superblock->BPS), buffer  + 0x0B, 2);
  memcpy(&(superblock->SPC), buffer  + 0x0D, 1);
  memcpy(&(superblock->RS), buffer   + 0x0E, 2);
  memcpy(&(superblock->FATS), buffer   + 0x10, 1);
  memcpy(&(superblock->DE), buffer   + 0x11, 2);
  memcpy(&(superblock->SOV), buffer   + 0x13, 2);
  memcpy(&(superblock->MD), buffer   + 0x15, 1);
  memcpy(&(superblock->SPF), buffer   + 0x16, 2);
  memcpy(&(superblock->SPT), buffer   + 0x18, 2);
  memcpy(&(superblock->RWH), buffer   + 0x1A, 2);
  memcpy(&(superblock->DES), buffer   + 0x1C, 2);

  superblock->begindata = superblock->RS + superblock->FATS * superblock->SPF +
                               ((superblock->DE *32) / 512) + 1 - 2;

  superblock->begindir = superblock->RS + superblock->FATS * superblock->SPF + 1;

  rootinode->direct[0] = superblock->begindir;

  superblock->beginfat  = superblock->RS + 1;

#ifdef MSDOS_DEBUG
  printk("msdos_mount: begindata = %d",superblock->begindata);
  printk("msdos_mount: begindir = %d",superblock->begindir);
  printk("msdos_mount: beginfat = %d",superblock->beginfat);
#endif

  if ((superblock->SOV / superblock->SPC) >= 4096) superblock->fat16 = 1;
  else superblock->fat16 = 0;

  return SUCCESS;
}

/************************************************************************
/* read or write an entry in the file allocation table.
/************************************************************************
/* pos = offset pos in fat
/* (op == 1) ==> read and return entry
/********************************************************************/
int msdos_fat_io(unsigned char *device,struct msdos_super_block *superblock, int pos)
{
  unsigned char buffer[512];
  int sector,offset;
  int newpos = 0;
  int maxclusters;
  int i;
  int value=0;

  if (superblock->fat16)
    {
      sector = pos / 32;
      offset = pos % 32;

      if (readblock(device, superblock->beginfat + sector, buffer))
        return ERROR_READ_BLOCK;

      newpos = offset * 2;
      memcpy(&sector, buffer + newpos, 2);
    }
    else
    {
      sector = pos / 42;
      offset = pos % 42;

#ifdef MSDOS_DEBUG
      printk("msdos.c: fat_io() fat: %d sector = %d, offset = %d, device = %s",superblock->beginfat, sector, offset, device);
#endif
      if (readblock(device, superblock->beginfat + sector, buffer))
      {
         printk("fat_io() read failure on sector: %d", sector+superblock->beginfat);
         return ERROR_READ_BLOCK;
      }
#ifdef MSDOS_DEBUG
      memory_dump(buffer,512);
      printk("fat_io() read on sector: %d", sector+superblock->beginfat);
#endif

      newpos = offset * 1.5;
      memcpy(&sector, buffer + newpos, 2);

      if (offset & 1)
        sector = sector >> 4;
      else
        sector = sector & 0xFFF;

#ifdef MSDOS_DEBUG
      printk("\nmsdos.c: newsector: %d",sector);
#endif
    }
    if (superblock->fat16)
    {
      if (sector == 0xFFFF)
        return EOF;
    }

    if (!superblock->fat16)
    {
      if (sector == 0xFFF)
        return EOF;
    }
    return sector;
}

/**************************************************************************/
/* Returns the nr'th directory entry from a buffer
/* Return Value: Always true
/**************************************************************************/
int msdos_get_entry(struct inode *inode, int nr, unsigned char *buffer)
{
  int i=0,j=0;

  i=nr*32;

  if (!buffer[i]) return ERROR_FILE_NOT_FOUND;

  while ((i<nr*32+8) && (buffer[i]!=0x20)) inode->name[j++]=buffer[i++];

  i=nr*32+8;
  if (buffer[i]!=0x20) inode->name[j++]='.';

  while ((i<nr*32+11) && (buffer[i]!=0x20)) inode->name[j++]=buffer[i++];

  inode->name[j]='\0';

  i=nr*32;
  if (buffer[i+11] & 16) inode->type=0;
  else inode->type=1;

  memcpy(&inode->size, nr * 32 + buffer + 28, 4);

  inode->direct[0] = (unsigned short)buffer[i+0x1A];

  return SUCCESS;
}


int msdos_get_inode(unsigned char *device, struct msdos_super_block *superblock, struct inode *actinode, struct inode *inode, unsigned int number)
{
  unsigned char buffer[512];
  int block = actinode->direct[0];

  while (number>15)
  {
    if (block>=superblock->begindata) block=msdos_fat_io(device,superblock,block);
    block++;
    number=number-16;
  }

  if (readblock(device, block, buffer))
    return ERROR_READ_BLOCK;

  if (msdos_get_entry(inode, number, buffer)) return ERROR_FILE_NOT_FOUND;

  if (inode->name[0] == 0xE5) inode->type=2;
  if (inode->name[0] == 0x05) inode->name[0]=0xE5;
/*  if (inode->direct[0]) inode->direct[0] = inode->direct[0] * superblock->SPF + superblock->begindata;
  else inode->direct[0] = superblock->begindir;*/

  return SUCCESS;
}

int msdos_lookup_inode(unsigned char *device, struct msdos_super_block *superblock, struct inode *actinode, struct inode *inode, unsigned char* name)
{
  unsigned char buffer[512];
  int block = actinode->direct[0];
  int number = 0;
  int ret;
  int found = 0;

  ret = msdos_get_inode(device, superblock, actinode, inode, number++);

  while ((ret == SUCCESS) && (!found))
  {
    if (inode->type == 1)                                // inode is a file
    {
        if (!strcmp(name, inode->name)) found = 1;
    }

    if (!found)
      ret = msdos_get_inode(device, superblock, actinode, inode, number++);
  }

  if (found) return SUCCESS;
  else return ret;
}



int msdos_lookup(unsigned char *device, struct msdos_super_block *superblock,struct inode *actinode, struct inode *inode, unsigned char *name)
{
  int ret, i=0;

#ifdef MSDOS_DEBUG
  printk("msdos_lookup: for \"%s\" on device %s",name,device);
#endif

  do
  {
    if (ret = msdos_get_inode(device,superblock,actinode,inode,i++)) return ret;
  } while (strcmp((unsigned char*)inode,name));

  return SUCCESS;
}

