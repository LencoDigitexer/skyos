/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : fs\vfs.c
/* Last Update: 12.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert, Resl Christian
/* Docus from :
/************************************************************************/
/* Definition:
/*   This file implements the virtual file system
/*   Functions to mount, unmount, read, write, open, close, ....
/*   Supported devices: all registered block and char devices.
/************************************************************************/
#include "sched.h"
#include "ctype.h"
#include "vfs.h"
#include "error.h"
#include "fcntl.h"
#include "msgbox.h"

// uncomment this to enable vfs debug
#define VFS_DEBUG 1

extern struct task_struct *current;
extern int cache_enabled;
extern struct task_struct *current;

struct list *ml = NULL;                        //mountlist
struct list *pl = NULL;                        //protocollist
struct list *fl = NULL;                        //filelist

int TASK_VFS;
unsigned int filedesc = 0;

extern int TASK_GUI;

int parse_filename(struct ipc_message *m, unsigned char* filename, unsigned char*device, unsigned char *file, unsigned int *abs)
{
  int i=0;
  int j=0;

  if ((filename[0] == '/') && (filename[1] == '/'))
  {
      i=2;
      while ((filename[i] != '/') && (filename[i] != '\0'))
      {
        device[i-2] = filename[i];
        i++;
      }
      device[i-2] = '\0';
      if (filename[i] == '\0') return -1;
      i++;
  }
  else strcpy(device, m->source->actual_device);

  if (filename[i] == '\0') return -1;

  *abs = 1;
  if (filename[i] == '/') i++;
  else *abs = 0;

  j = 0;

  while ((filename[i] != '\0'))
  {
     file[j] = filename[i];
     i++;
     j++;
  }

  file[j] = '\0';
  return SUCCESS;
}

int protocol2pid(unsigned char *protocol)
{
  int i=0;
  struct protocol_item *pi=(struct protocol_item*)get_item(pl,i++);

  while (pi!=NULL)
  {
    if (!strcmp(pi->protocol,protocol)) return pi->pid;
    pi=(struct protocol_item*)get_item(pl,i++);
  }
  return -1;
}

int device2pid(unsigned char *device)
{
  int i=0;

  struct mount_item *mi = (struct mount_item*)get_item(ml,i++);

  while (mi!=NULL)
  {
    if (!strcmp(mi->device,device)) return protocol2pid(mi->protocol);
    mi=(struct mount_item*)get_item(ml,i++);
  }
  return -1;
}

char *get_actinode(unsigned char *dev_name)
{
  int i=0;

  struct mount_item *mi = (struct mount_item*)get_item(ml,i++);

  while (mi!=NULL)
  {
    if (!strcmp(mi->device,dev_name)) return (char*)mi->actinode;
    mi=(struct mount_item*)get_item(ml,i++);
  }
  return NULL;
}

char *get_rootinode(unsigned char *dev_name)
{
  int i=0;

  struct mount_item *mi = (struct mount_item*)get_item(ml,i++);

  while (mi!=NULL)
  {
    if (!strcmp(mi->device,dev_name)) return (char*)mi->rootinode;
    mi=(struct mount_item*)get_item(ml,i++);
  }
  return NULL;
}

char *get_actsuperblock(unsigned char *dev_name)
{
  int i = 0;

  struct mount_item *mi = (struct mount_item*)get_item(ml,i++);

  while (mi!=NULL)
  {
    if (!strcmp(mi->device,dev_name)) return (char*)mi->superblock;
    mi=(struct mount_item*)get_item(ml,i++);
  }
  return NULL;
}

int pre_mount(struct ipc_message *m)
{
  // does device exist?
  if (name2device(m->MSGT_MOUNT_DEVICE) == -1) return ERROR_DEVICE_NOT_FOUND;

  // protocol registered?
  if (protocol2pid(m->MSGT_MOUNT_PROTOCOL) == -1) return ERROR_PROTOCOL_NOT_REGISTERED;

  // device already mounted?
  if (device2pid(m->MSGT_MOUNT_DEVICE) != -1) return ERROR_DEVICE_MOUNTED;

  return SUCCESS;
}

/***************************************************************************/
/* function:  mount a protocol to a device                                 */
/* input:     devicename and protocol via message                          */
/***************************************************************************/
int mount(struct ipc_message *m)
{
  struct mount_item *mi;

  mi=(struct mount_item*)valloc(sizeof(struct mount_item));

  mi->device=(unsigned char*)valloc(strlen(m->MSGT_MOUNT_DEVICE)+1);
  mi->protocol=(unsigned char*)valloc(strlen(m->MSGT_MOUNT_PROTOCOL)+1);
  mi->rootpath=(unsigned char*)valloc(2);
  mi->rootinode=(struct inode*)valloc(sizeof(struct inode));
  mi->actpath=(unsigned char*)valloc(2);
  mi->actinode=(struct inode*)valloc(sizeof(struct inode));
  mi->superblock=(char*)valloc(512);

  strcpy(mi->device,m->MSGT_MOUNT_DEVICE);
  strcpy(mi->protocol,m->MSGT_MOUNT_PROTOCOL);
  strcpy(mi->rootpath,"/");
  memcpy(mi->rootinode,m->MSGT_MOUNT_ROOTINODE,sizeof(struct inode));
  strcpy(mi->actpath,"/");
  memcpy(mi->actinode,m->MSGT_MOUNT_ROOTINODE,sizeof(struct inode));
  memcpy(mi->superblock,m->MSGT_MOUNT_SUPERBLOCK,512);

  // adding mountitem in mountlist
  ml=(struct list*)add_item(ml,mi,sizeof(struct mount_item));

  return SUCCESS;
}

/***************************************************************************/
/* function:  unmount a protocol from a device                             */
/* input:     devicename via message                                       */
/***************************************************************************/
int unmount(struct ipc_message *m)
{
  int i=0;
  struct mount_item *mi;

  if (m->source->fl != NULL) return ERROR_FILE_OPEN;

  mi=(struct mount_item*)get_item(ml,i++);
  while (mi!=NULL)
  {
    if (!strcmp(mi->device,m->MSGT_UNMOUNT_DEVICE))
    {
      ml=(struct list*)del_item(ml,i-1);
      return SUCCESS;
    }
    mi=(struct mount_item*)get_item(ml,i++);
  }
  return ERROR_DEVICE_NOT_MOUNTED;
}

int pre_open(struct ipc_message *m)
{
  struct inode *inode;
  unsigned char *device;
  unsigned char *filename;

  int i=0;
  int abs = 0;

  struct file_item *fi = (struct file_item*)get_item(fl,i++);

  device   = (unsigned char*)valloc(255);
  filename = (unsigned char*)valloc(255);
  parse_filename(m, m->MSGT_OPEN_NAME, device, filename,&abs);

  m->MSGT_OPEN_NAME = filename;

  if (device2pid(device) == -1) return ERROR_DEVICE_NOT_MOUNTED;

  while (fi != NULL)
  {
    // no opening for writing when already opened!
    if ((!strcmp(fi->fn,m->MSGT_OPEN_NAME)) && (m->MSGT_OPEN_MODE != O_RDONLY))
      return ERROR_FILE_USED;
    // no opening for reading when already opened for writing!
    if ((!strcmp(fi->fn,m->MSGT_OPEN_NAME)) && (fi->mode != O_RDONLY))
      return ERROR_FILE_USED;

    fi=(struct file_item*)get_item(fl,i++);
  }

  m->MSGT_OPEN_DEVICE = device;

  if (abs)
    m->MSGT_OPEN_ACTINODE =(struct inode*)get_rootinode(device);
  else
    m->MSGT_OPEN_ACTINODE =(struct inode*)get_actinode(device);

  m->MSGT_OPEN_INODE = (struct inode*)valloc(sizeof(struct inode));
  m->MSGT_OPEN_ROOTINODE = get_rootinode(device);

//  printk("done");
  return SUCCESS;
}

int pre_find(struct ipc_message *m)
{
  struct inode *inode;
  unsigned char *device;
  unsigned char *filename;

  int i=0;
  int abs = 0;
  int ret = 0;

  device   = (unsigned char*)valloc(255);
  filename = (unsigned char*)valloc(255);
  parse_filename(m, m->MSGT_FIND_NAME, device, filename,&abs);

  if ((filename[0] == '\\') || (filename[0]=='\0'))
  {
     memcpy(m->MSGT_FIND_INODE, get_rootinode(device),sizeof(struct inode));
     ret = 1;
  }

  inode = m->MSGT_FIND_INODE;
  inode->device = device;

  m->MSGT_FIND_NAME = filename;

  if (device2pid(device) == -1) return ERROR_DEVICE_NOT_MOUNTED;

  m->MSGT_FIND_DEVICE = device;

  if (abs)
    m->MSGT_FIND_ACTINODE =(struct inode*)get_rootinode(device);
  else
    m->MSGT_FIND_ACTINODE =(struct inode*)get_actinode(device);

  m->MSGT_FIND_ROOTINODE = get_rootinode(device);


  return ret;
}

int pre_getinode(struct ipc_message *m)
{
  struct inode *inode;
  unsigned char *device;
  unsigned char *filename;

  int i=0;
  int abs = 0;

  inode = m->MSGT_GETINODE_INODE;
  device = inode->device;

  if (device2pid(device) == -1) return ERROR_DEVICE_NOT_MOUNTED;

  m->MSGT_GETINODE_DEVICE = device;
  m->MSGT_GETINODE_ROOTINODE = get_rootinode(device);

  return SUCCESS;
}

int open(struct ipc_message *m)
{
  int i=0, fd=0, found = 1;
  struct list *ptr=fl;
  struct file_item *fi;
  char str[256];

  while ((found) && (fd < MAX_OPEN_FILES))
  {
    i = 0;
    found = 0;
    fi=(struct file_item*)get_item(m->source->fl,i++);
    while (fi != NULL)
    {
      if (fi->fd == fd) found = 1;
      fi=(struct file_item*)get_item(m->source->fl,i++);
    }
    if (found) fd++;
  }

  fi=(struct file_item*)valloc(sizeof(struct file_item));
  fi->inode = (struct inode*)valloc(sizeof(struct inode));
  fi->fn = (unsigned char*)valloc(strlen(m->MSGT_OPEN_NAME)+1);

  fi->fd = fd;

  memcpy(fi->inode, m->MSGT_OPEN_INODE, sizeof(struct inode));
  fi->inode->cache = NULL;
  fi->inode->cache_index = 0;

  strcpy(fi->fn, m->MSGT_OPEN_NAME);
  fi->fp = 0;
  fi->size = m->MSGT_OPEN_RESULT;

  // set file mode
  fi->mode = m->MSGT_OPEN_MODE;
  fi->binary = 1;

  if (fi->mode & O_BINARY) fi->binary = 1;
  else if (fi->mode & O_TEXT) fi->binary = 0;

  fi->pid = m->source->pid;

  fi->device = m->MSGT_OPEN_DEVICE;

  fi->rootinode = get_rootinode(m->MSGT_OPEN_DEVICE);

  // adding fileitem in filelist of the task and vfs
  m->source->fl=(struct list*)add_item(m->source->fl,fi,sizeof(struct file_item));
  fl=(struct list*)add_item(fl,fi,sizeof(struct file_item));

  if (fi->mode & O_RDONLY) sprintf(str,"opening file \"%s\" for reading",m->MSGT_OPEN_INODE);
  if (fi->mode & O_WRONLY) sprintf(str,"opening file \"%s\" for writing",m->MSGT_OPEN_INODE);
  if (fi->mode & O_RDWR) sprintf(str,"opening file \"%s\" for reading/writing",m->MSGT_OPEN_INODE);
  show_msg(str);

  return fd;
}

int close(struct ipc_message *m)
{
  int i=0;
  struct file_item *fi, *fi2;

  fi = (struct file_item*)get_item(fl,i);
  fi2 = (struct file_item*)get_item(m->source->fl,i++);
  while (fi != NULL)
  {
    // filedescriptor in task list found?
    if (fi2 != NULL)
    {
      if (fi2->fd == m->MSGT_CLOSE_FD)
         m->source->fl=(struct list*)del_item(m->source->fl,i-1);
    }

    // filedescriptor and pid in vfs list found?
    if ((fi->fd == m->MSGT_CLOSE_FD) && (fi->pid == m->source->pid))
    {
      fl=(struct list*)del_item(fl,i-1);
      return SUCCESS;
    }
    fi=(struct file_item*)get_item(m->source->fl,i);
    if (fi2 != NULL) fi2 = (struct file_item*)get_item(m->source->fl,i);
    i++;
  }
  return ERROR_FILE_NOT_OPEN;
}

int pre_read(struct ipc_message *m)
{
  int i=0;
  struct file_item *fi;

  fi=(struct file_item*)get_item(m->source->fl,i++);
  while (fi != NULL)
  {
    if (fi->fd == m->MSGT_READ_FD)
    {
      m->MSGT_READ_INODE = fi->inode;
      m->MSGT_READ_FP = &fi->fp;
      m->MSGT_READ_DEVICE = fi->device;
      m->MSGT_READ_ROOTINODE = get_rootinode(m->MSGT_READ_DEVICE);
      m->MSGT_READ_MODE = fi->mode;
      return SUCCESS;
    }
    fi=(struct file_item*)get_item(m->source->fl,i++);
  }
  return ERROR_FILE_NOT_OPEN;
}

int filelength(struct ipc_message *m)
{
  int i=0;
  struct file_item *fi;
  char str[256];

  fi=(struct file_item*)get_item(m->source->fl,i++);
  while (fi != NULL)
  {
    if (fi->fd == m->MSGT_FILELENGTH_FD)
    {
      return fi->size;
    }
    fi=(struct file_item*)get_item(m->source->fl,i++);
  }
  return ERROR_FILE_NOT_OPEN;
}

int IsEOF(int fd)
{
  int i=0;
  struct file_item *fi;
  char str[256];

  fi=(struct file_item*)get_item(current->fl,i++);
  while (fi != NULL)
  {
    if (fi->fd == fd)
    {
      if (fi->fp == fi->size) return 1;
    }
    fi=(struct file_item*)get_item(current->fl,i++);
  }
  return 0;
}

int fseek(struct ipc_message *m)
{
  int i=0;
  struct file_item *fi;

  fi=(struct file_item*)get_item(m->source->fl,i++);
  while (fi != NULL)
  {
    if (fi->fd == m->MSGT_SEEK_FD)
    {
      fi->fp = m->MSGT_SEEK_POS;
      return SUCCESS;
    }
    fi=(struct file_item*)get_item(m->source->fl,i++);
  }
  return ERROR_FILE_NOT_OPEN;
}

int tell(struct ipc_message *m)
{
  int i=0;
  struct file_item *fi;

  fi=(struct file_item*)get_item(m->source->fl,i++);
  while (fi != NULL)
  {
    if (fi->fd == m->MSGT_TELL_FD)
      return fi->fp;
    fi=(struct file_item*)get_item(m->source->fl,i++);
  }
  return ERROR_FILE_NOT_OPEN;
}


/***************************************************************************/
/* function:  register a protocol                                          */
/* input:     protocol and pid                                             */
/***************************************************************************/
int register_protocol(unsigned char *protocol, unsigned int pid)
{
  int i=0;
  struct protocol_item *pi;

  pi=(struct protocol_item*)get_item(pl,i++);

  // protocol already registered?
  while (pi!=NULL)
  {
    if (!strcmp(pi->protocol,protocol)) return ERROR_PROTOCOL_REGISTERED;
    pi=(struct protocol_item*)get_item(pl,i++);
  }

  pi=(struct protocol_item*)valloc(sizeof(struct protocol_item));

  pi->protocol=(unsigned char*)valloc(strlen(protocol)+1);
  strcpy(pi->protocol,protocol);
  pi->pid=pid;

  // adding mountitem in mountlist
  pl=(struct list*)add_item(pl,pi,sizeof(struct protocol_item));
  return SUCCESS;
}

void show_mount_table()
{
  int i=0;
  struct mount_item *mi;
  char str[256];

  show_msg("");
  show_msg("Mounted devices:");
  show_msg("ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄ¿");
  show_msg("³    device    ³  protocol  ³  root path  ³ actual path ³");
  show_msg("ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄ´");

  mi=(struct mount_item*)get_item(ml,i);
  i++;

  while (mi!=NULL)
  {
    sprintf(str,"³ %-12s ³ %-10s ³ %-11s ³ %-11s ³", mi->device, mi->protocol, mi->rootpath, mi->actpath);
    show_msg(str);
    mi=(struct mount_item*)get_item(ml,i);
    i++;
  }
  show_msg("ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");
}

void show_protocol_table()
{
  int i=0;
  struct protocol_item *pi;
  char str[256];

  show_msg("");
  show_msg("Registered protocols:");
  show_msg("ÚÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄ¿");
  show_msg("³  protocol  ³    pid     ³");
  show_msg("ÃÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ´");

  pi=(struct protocol_item*)get_item(pl,i);
  i++;

  while (pi != NULL)
  {
    sprintf(str,"³ %-10s ³ %-10d ³", pi->protocol, pi->pid);
    show_msg(str);
    pi=(struct protocol_item*)get_item(pl,i);
    i++;
  }
  show_msg("ÀÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÙ");
}

void show_file_table()
{
  int i=0;
  struct file_item *fi;
  char str[256];

  show_msg("");
  show_msg("Filetable for all tasks:");
  show_msg("ÚÄÄÄÄÄÄÄÂÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄ¿");
  show_msg("³  PID  ³  FD  ³    Filename    ³   Mode   ³    FP    ³  ");
  show_msg("ÃÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄ´");

  fi=(struct file_item*)get_item(fl,i++);

  while (fi != NULL)
  {
    sprintf(str,"³ %-5d ³ %-4d ³ %-14s ³ 0x%-6X ³ %-8d ³", fi->pid, fi->fd, fi->fn, fi->mode, fi->fp);
    show_msg(str);
    fi=(struct file_item*)get_item(fl,i++);
  }
  show_msg("ÀÄÄÄÄÄÄÄÁÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÙ");
  show_msg("");
  sprintf(str,"Filetable for the current task %d:",current->pid);
  show_msg(str);
  show_msg("ÚÄÄÄÄÄÄÄÂÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄ¿");
  show_msg("³  PID  ³  FD  ³    Filename    ³   Mode   ³    FP    ³  ");
  show_msg("ÃÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄ´");

  i=0;
  fi=(struct file_item*)get_item(current->fl,i++);

  while (fi!=NULL)
  {
    sprintf(str,"³ %-5d ³ %-4d ³ %-14s ³ 0x%-6X ³ %-8d ³", fi->pid, fi->fd, fi->fn, fi->mode, fi->fp);
    show_msg(str);
    fi=(struct file_item*)get_item(current->fl,i++);
  }
  show_msg("ÀÄÄÄÄÄÄÄÁÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÙ");
}

void vfs_task()
{
  unsigned char str[255];
  unsigned char buffer[512];
  struct inode *inode = (struct inode*)valloc(sizeof(struct inode));
  struct inode *in;
  struct ipc_message *m;
  struct task_struct *newtask;
  int task;
  int ret;
  int i;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_CLOCK_SYSTEM_PERFORMANCE:
             {
                 m->type = MSG_REPLY;
                 send_msg(m->source->pid, m);
                 break;
             }

       case MSG_MOUNT:
             {
               m->MSGT_MOUNT_RESULT = pre_mount(m);
               if (m->MSGT_MOUNT_RESULT)
               {
                 m->type = MSG_REPLY;
                 send_msg(m->source->pid, m);
               }
               else send_msg(protocol2pid(m->MSGT_MOUNT_PROTOCOL), m);
               break;
             }
       case MSG_UNMOUNT:
             {
               m->MSGT_MOUNT_RESULT = unmount(m);
               m->type = MSG_REPLY;
               send_msg(m->source->pid, m);
               break;
             }
       case MSG_OPEN:
             {
               printk("open");
               m->MSGT_OPEN_RESULT = pre_open(m);

               if (m->MSGT_OPEN_RESULT)
               {
                 m->type = MSG_OPEN_REPLY;
                 send_msg(m->source->pid, m);
               }
               else send_msg(device2pid(m->MSGT_OPEN_DEVICE), m);
               break;
             }
       case MSG_FIND:
             {
               m->MSGT_FIND_RESULT = pre_find(m);

               if (m->MSGT_FIND_RESULT<0)
               {
                 m->type = MSG_FIND_REPLY;
                 send_msg(m->source->pid, m);
               }
               else if(m->MSGT_FIND_RESULT == 1)
               {
                 m->type = MSG_FIND_REPLY;
                 m->MSGT_FIND_RESULT = SUCCESS;
                 send_msg(m->source->pid, m);
               }
               else send_msg(device2pid(m->MSGT_FIND_DEVICE), m);
               break;
             }
       case MSG_GETINODE:
             {
               m->MSGT_GETINODE_RESULT = pre_getinode(m);

               if (m->MSGT_GETINODE_RESULT)
               {
                 m->type = MSG_GETINODE_REPLY;
                 send_msg(m->source->pid, m);
               }
               else send_msg(device2pid(m->MSGT_GETINODE_DEVICE), m);
               break;
             }
       case MSG_CLOSE:
             {
               m->MSGT_CLOSE_RESULT = close(m);
               m->type = MSG_REPLY;
               send_msg(m->source->pid, m);
               break;
             }
       case MSG_FILELENGTH:
             {
               m->MSGT_FILELENGTH_RESULT = filelength(m);
               m->type = MSG_REPLY;
               send_msg(m->source->pid, m);
               break;
             }
       case MSG_SEEK:
             {
               m->MSGT_SEEK_RESULT = fseek(m);
               m->type = MSG_SEEK_REPLY;
               send_msg(m->source->pid, m);
               break;
             }
       case MSG_TELL:
             {
               m->MSGT_TELL_RESULT = tell(m);
               m->type = MSG_TELL_REPLY;
               send_msg(m->source->pid, m);
               break;
             }
       case MSG_READ:
             {
               m->MSGT_READ_RESULT = pre_read(m);
               if (m->MSGT_READ_RESULT)
               {
                 m->type = MSG_READ_REPLY;
                 send_msg(m->source->pid, m);
               }
               else send_msg(device2pid(m->MSGT_READ_DEVICE), m);
               break;
             }
       case MSG_WRITE:
             {
               m->MSGT_READ_RESULT = pre_read(m);
               if (m->MSGT_READ_RESULT)
               {
                 m->type = MSG_WRITE_REPLY;
                 send_msg(m->source->pid, m);
               }
               else send_msg(device2pid(m->MSGT_READ_DEVICE), m);
               break;
             }
        case MSG_READ_BLOCK:            // read a block from a block device
             {
                read_blkdev(m);
                break;
             }
        case MSG_WRITE_BLOCK:            // write a block to a device
             {
                read_blkdev(m);
                break;
             }

        case MSG_READ_CHAR:             // read a char from a char device
             {
                read_chardev(m);
                break;
             }

        case MSG_CREATE_NEW_APPLICATION:
             {
               if (m->MSGT_PROGRAMM_PRIVILEGE == KERNEL_TASK)
                 newtask = (struct task_struct*)CreateKernelTask(m->MSGT_PROGRAMM_ADDRESS, m->MSGT_PROGRAMM_NAME, NP,1);
               else
                 newtask = (struct task_struct*)CreateUserTask(m->MSGT_PROGRAMM_ADDRESS, m->MSGT_PROGRAMM_NAME, NP,1);

               m->type = MSG_GUI_CREATE_WINDOW;
               m->MSGT_GUI_CREATE_WINDOW_TASK = newtask;
               send_msg(TASK_GUI, m);
               break;
             }

// REPLY BLOCKS:

        case MSG_REPLY:
             {
               send_msg(m->source->pid, m);
               break;
             }
        case MSG_MOUNT_REPLY:
             {
               if (!m->MSGT_MOUNT_RESULT) m->MSGT_MOUNT_RESULT = mount(m);
               m->type = MSG_REPLY;
               send_msg(m->source->pid, m);
               break;
             }
        case MSG_OPEN_REPLY:
             {
               if (m->MSGT_OPEN_RESULT>0)
                 m->MSGT_OPEN_RESULT = open(m);

               m->type = MSG_OPEN_REPLY;
               send_msg(m->source->pid, m);
               break;
             }
        case MSG_FIND_REPLY:
             {
               send_msg(m->source->pid, m);
               break;
             }
        case MSG_GETINODE_REPLY:
             {
               send_msg(m->source->pid, m);
               break;
             }
        case MSG_READ_REPLY:
             {
                send_msg(m->source->pid,m);
                break;
             }
        case MSG_WRITE_REPLY:
             {
                send_msg(m->source->pid,m);
                break;
             }
        case MSG_READ_CHAR_REPLY:
             {
                send_msg(m->source->pid,m);
                break;
             }
        case MSG_READ_BLOCK_REPLY:
             {
                if (cache_enabled)                 // Add Block to Cache
                {
                  show_msg("vfs.c: adding to cache...");
                  cache_add(name2device(m->MSGT_READ_BLOCK_DEVICENAME),
                    m->MSGT_READ_BLOCK_BLOCKNUMMER, m->MSGT_READ_BLOCK_BUFFER);
                  show_msg("vfs.c: done");
                }

                send_msg(m->source->pid,m);
                break;
             }
        case MSG_WRITE_BLOCK_REPLY:
             {
                send_msg(m->source->pid,m);
                break;
             }
        case MSG_READ_BLOCK_CACHE_REPLY:
             {
                m->type = MSG_READ_BLOCK_REPLY;
                send_msg(m->source->pid,m);
                break;
             }
/*        case MSG_GUI_CREATE_WINDOW_REPLY:
             {
                task = m->source->pid;
                send_msg(task,m);
                break;
             }*/
     }
  }
}

int vfs_init(void)
{

  printk("vfs.c: Initializing virtual filesystem...");
  TASK_VFS = ((struct task_struct*)CreateKernelTask((unsigned int)vfs_task, "vfs", HP,0))->pid;


  return 0;

}


