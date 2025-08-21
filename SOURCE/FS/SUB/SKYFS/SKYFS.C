/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : fs\sub\skyfs.c
/* Last Update: 18.01.1999
/* Version    : alpha
/* Coded by   : Resl Christian, Szeleney Robert
/* Docus from :
/************************************************************************/
/* Definition:
/*   This file implements the sky filesystem.
/*   Includes functions for mount, unmount, open, close, read, ....
/*   Supported devices: all registered block devices
/************************************************************************/
#include "sched.h"
#include "vfs.h"
#include "error.h"

// uncomment the next line to enable in-time debug messages
#define SKYFS_DEBUG 1

int skyfs_pid;                                  // Process-ID

/************************************************************************/
/* SKYFS task
/************************************************************************/
void skyfs_task()
{
  int ret;
  int task;
  unsigned int from;
  unsigned char str[255];
  struct ipc_message *m;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
        case MSG_MOUNT:
             {
               m->MSGT_MOUNT_RESULT = skyfs_mount(m);
               m->type = MSG_MOUNT_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_OPEN:
             {
               m->MSGT_OPEN_RESULT = skyfs_open(m);
               m->type = MSG_OPEN_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_LOOKUP:
             {
               m->MSGT_LOOKUP_RESULT = skyfs_lookup(m);
               m->type = MSG_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_READ:
             {
               m->MSGT_READ_RESULT = skyfs_read(m);
               m->type = MSG_READ_REPLY;
               send_msg(m->sender, m);
               break;
             }
     }
  }
}

/************************************************************************/
/* Initialize the SKY filesystem.
/* Module Init Entry Point.
/************************************************************************/
int skyfs_init(void)
{
  printk("vfs.c: Initializing SKY Filesystem...");
  skyfs_pid = ((struct task_struct*)CreateKernelTask((unsigned int)skyfs_task, "skyfs", HP,0))->pid;

  register_protocol("skyfs",skyfs_pid);

  return SUCCESS;
}

/************************************************************************/
/* Mount a device to the skyfs.
/************************************************************************/
int skyfs_mount(struct ipc_message *m)
{
  struct skyfs_inode inode_buffer[4];
  struct inode *inode;

  // read superblock
  if (readblock(m->MSGT_MOUNT_DEVICE, 1, m->MSGT_MOUNT_SUPERBLOCK)) return ERROR_READ_BLOCK;
  // check if skyfs superblock
  if (strncmp(m->MSGT_MOUNT_SUPERBLOCK+3, "SKYFS", 5)) return ERROR_WRONG_PROTOCOL;
  // read rootinode
  if (readblock(m->MSGT_MOUNT_DEVICE, 2, inode_buffer)) return ERROR_READ_BLOCK;

  inode = m->MSGT_MOUNT_ROOTINODE;

  memcpy(&(inode->u.skyfs_inode), inode_buffer, 128);

  return SUCCESS;
}

int offset2block(unsigned char *device, struct skyfs_inode *inode, unsigned int offset, unsigned int *block)
{
  unsigned int block_buffer[128];

  offset = offset / 512;

  if (offset < 8)
  {
    *block = inode->direct[offset];
    if (!*block) return ERROR_WRONG_OFFSET;
    return SUCCESS;
  }
  offset -= 8;
  if (offset < 128)
  {
    *block = inode->indirect[0];
    if (!*block) return ERROR_WRONG_OFFSET;
    readblock(device, *block, block_buffer);
    *block = block_buffer[offset];
    if (!*block) return ERROR_WRONG_OFFSET;
    return SUCCESS;
  }
  offset -= 128;
  if (offset < 128)
  {
    *block = inode->indirect[1];
    if (!*block) return ERROR_WRONG_OFFSET;
    readblock(device, *block, block_buffer);
    *block = block_buffer[offset];
    if (!*block) return ERROR_WRONG_OFFSET;
    return SUCCESS;
  }
  offset -= 128;
  if (offset < 128)
  {
    *block = inode->indirect[2];
    if (!*block) return ERROR_WRONG_OFFSET;
    readblock(device, *block, block_buffer);
    *block = block_buffer[offset];
    if (!*block) return ERROR_WRONG_OFFSET;
    return SUCCESS;
  }
  return ERROR_WRONG_OFFSET;
}

/************************************************************************/
/* Search for an Inode on a mounted device                               */
/************************************************************************/
int skyfs_lookup(struct ipc_message *m)
{
  unsigned int ret, block, offset = 0;
  struct skyfs_inode inode_buffer[4] = {0};
  unsigned char str[255] = {0};
  struct inode *vfs_i;
  struct skyfs_inode *inode;
  struct skyfs_inode *inode2;
  struct superblock *sb;

  vfs_i = (struct inode*)m->MSGT_LOOKUP_ACTINODE;
  inode = (struct skyfs_inode*)&(vfs_i->u.skyfs_inode);

  ret = offset2block(m->MSGT_LOOKUP_DEVICE, inode, offset, &block);

  while (!ret)
  {
    readblock(m->MSGT_LOOKUP_DEVICE,block,&inode_buffer);

    if (!strcmp(inode_buffer[0].name,m->MSGT_LOOKUP_NAME))
    {
      vfs_i = (struct inode*)m->MSGT_LOOKUP_INODE;
      inode2 = (struct skyfs_inode*)&(vfs_i->u.skyfs_inode);

      memcpy(inode2,&inode_buffer,sizeof(struct skyfs_inode));
      return SUCCESS;
    }
    offset += 512;
    ret = offset2block(m->MSGT_LOOKUP_DEVICE, inode, offset, &block);
  }
  return ERROR_FILE_NOT_FOUND;
}

int skyfs_open(struct ipc_message *m)
{
  return skyfs_lookup(m);
}

/************************************************************************/
/* Read from an opened file on a skyfs mounted device                   */
/************************************************************************/
int skyfs_read(struct ipc_message *m)
{
  struct inode *vfs_i;
  struct skyfs_inode *inode;
  unsigned int size = m->MSGT_READ_SIZE;
  unsigned int i,j;
  unsigned char dummy_buff[512] = {0};
  unsigned int read_bytes;
  unsigned int sum_read = 0;
  unsigned char *buffer_ptr = m->MSGT_READ_BUFFER;
  unsigned char str[255];
  unsigned int block;
  int lastblock = 0;

  vfs_i = m->MSGT_READ_INODE;
  inode = &(vfs_i->u.skyfs_inode);

  do
  {
    i = *m->MSGT_READ_FP / 512;
    j = *m->MSGT_READ_FP % 512;
    read_bytes = 512 - j;
    if (read_bytes > size) read_bytes = size;
    if ((*m->MSGT_READ_FP + read_bytes) > inode->size)
    {
      lastblock = 1;
      read_bytes = inode->size - *m->MSGT_READ_FP;
    }

    if (offset2block(m->MSGT_READ_DEVICE, inode, *m->MSGT_READ_FP, &block)) return 0;

    if (readblock(m->MSGT_READ_DEVICE, block, dummy_buff)) return ERROR_READ_BLOCK;

    memcpy(buffer_ptr, dummy_buff+j, read_bytes);

    buffer_ptr += read_bytes;
    *m->MSGT_READ_FP += read_bytes;
    size -= read_bytes;
    sum_read += read_bytes;
  } while ((sum_read < m->MSGT_READ_SIZE) && (!lastblock));

  return sum_read;
}

