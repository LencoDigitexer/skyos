/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : library\library.c
/* Last Update: 12.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert, Resl Christian
/* Docus from :
/************************************************************************/
/* Definition:
/*   The library functions which allows user-programms to call the kernel.
/************************************************************************/
#include "sched.h"
#include "vfs.h"
#include "error.h"

extern struct task_struct *current;
extern unsigned int TASK_VFS;
extern unsigned int TASK_GUI;
extern unsigned int TASK_VDM;
extern unsigned int net_pid;
extern unsigned char *act_device;              // name of actual device

// modified 15.04.99
// cvdm-class 
int readblock(unsigned char *device, unsigned int nr, unsigned char *buffer)
{
  struct ipc_message *m;
  void* p;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  m->type = MSG_READ_BLOCK;

  p = m->MSGT_READ_BLOCK_DEVICENAME = (char*)valloc(strlen(device)+1);
  strcpy(m->MSGT_READ_BLOCK_DEVICENAME, device);

  m->MSGT_READ_BLOCK_BUFFER = buffer;
  m->MSGT_READ_BLOCK_BLOCKNUMMER = nr;
  m->source = current;

  send_msg(TASK_VDM, m);

  wait_msg(m, MSG_READ_BLOCK_REPLY);

  vfree(p);
  vfree(m);

  return SUCCESS;
//   return m->MSGT_READ_BLOCK_RESULT;  uncomment if device driver use errorcodes
}

int writeblock(unsigned char *device, unsigned int nr, unsigned char *buffer)
{
  struct ipc_message *m;
  void* p;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  m->type = MSG_WRITE_BLOCK;

  p = m->MSGT_WRITE_BLOCK_DEVICENAME = (char*)valloc(strlen(device)+1);
  strcpy(m->MSGT_WRITE_BLOCK_DEVICENAME, device);

  m->MSGT_WRITE_BLOCK_BUFFER = buffer;
  m->MSGT_WRITE_BLOCK_BLOCKNUMMER = nr;
  m->source = current;

  send_msg(TASK_VDM, m);

  wait_msg(m, MSG_WRITE_BLOCK_REPLY);

  vfree(p);
  vfree(m);

  return SUCCESS;
//   return m->MSGT_READ_BLOCK_RESULT;  uncomment if device driver use errorcodes
}


unsigned char getch(void)
{
   struct ipc_message *m;
   unsigned char ch;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_READ_CHAR;

   m->MSGT_READ_BLOCK_DEVICENAME = (char*)valloc(strlen("keyboard\0")+1);
   strcpy(m->MSGT_READ_BLOCK_DEVICENAME, "keyboard\0");

   m->source = current;
   send_msg(TASK_VFS, m);

   wait_msg(m, MSG_READ_CHAR_REPLY);

   ch = m->MSGT_READ_CHAR_CHAR;                         // return value

   vfree(m);

   return ch;
}

int sys_mount(unsigned char *device, unsigned char *protocol)
{
   struct ipc_message *m;
   int result;
   void *p1;
   void *p2;
   void *p3;
   void *p4;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_MOUNT;

   p1 = m->MSGT_MOUNT_DEVICE = (unsigned char*)valloc(strlen(device)+1);
   p2 = m->MSGT_MOUNT_PROTOCOL = (unsigned char*)valloc(strlen(protocol)+1);
   p3 = m->MSGT_MOUNT_SUPERBLOCK=(char*)valloc(512);
   p4 = m->MSGT_MOUNT_ROOTINODE=(char*)valloc(sizeof(struct inode));

   strcpy(m->MSGT_MOUNT_DEVICE,device);
   strcpy(m->MSGT_MOUNT_PROTOCOL,protocol);

   m->source = current;

   send_msg(TASK_VFS, m);
   wait_msg(m, -1);

   vfree(p1);
   vfree(p2);
   vfree(p3);
   vfree(p4);

   result = m->MSGT_MOUNT_RESULT;

   vfree(m);

   return result;
}

int sys_clock_system_performance(void)
{
   struct ipc_message *m;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_CLOCK_SYSTEM_PERFORMANCE;

   m->source = current;

   send_msg(TASK_VFS, m);
   wait_msg(m, -1);

   vfree(m);

   return 1;
}

int sys_unmount(unsigned char *device)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_UNMOUNT;

   m->MSGT_UNMOUNT_DEVICE = (unsigned char*)valloc(strlen(device)+1);
   strcpy(m->MSGT_UNMOUNT_DEVICE,device);

   m->source = current;

   send_msg(TASK_VFS, m);

   wait_msg(m, -1);

   vfree(m->MSGT_UNMOUNT_DEVICE);

   result = m->MSGT_UNMOUNT_RESULT;

   vfree(m);

   return result;
}

int sys_open(const char *file, int mode)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_OPEN;

   m->MSGT_OPEN_NAME = (unsigned char*)valloc(strlen(file)+1);
   strcpy(m->MSGT_OPEN_NAME,file);
   m->MSGT_OPEN_MODE = mode;

   m->source = current;
   send_msg(TASK_VFS, m);
   wait_msg(m, MSG_OPEN_REPLY);

   vfree(m->MSGT_OPEN_NAME);

   result = m->MSGT_OPEN_RESULT;

   vfree(m);

   return result;
}

int sys_find(const char *file, struct inode *i)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_FIND;

   m->MSGT_FIND_NAME = (unsigned char*)valloc(strlen(file)+1);
   strcpy(m->MSGT_FIND_NAME,file);
   m->MSGT_FIND_INODE = i;

   m->source = current;
   send_msg(TASK_VFS, m);
   wait_msg(m, MSG_FIND_REPLY);

   vfree(m->MSGT_FIND_NAME);

   result = m->MSGT_FIND_RESULT;

   vfree(m);

   return result;
}

int sys_get_inode(struct inode *i, struct inode *res, int start)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_GETINODE;

   m->MSGT_GETINODE_INODE = i;
   m->MSGT_GETINODE_ACTINODE = res;
   m->MSGT_GETINODE_FIRST = start;

   m->source = current;
   send_msg(TASK_VFS, m);
   wait_msg(m, MSG_GETINODE_REPLY);

   result = m->MSGT_GETINODE_RESULT;

   vfree(m);

   return result;
}

int sys_close(int fd)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_CLOSE;
   m->source = current;

   m->MSGT_CLOSE_FD = fd;

   send_msg(TASK_VFS, m);
   wait_msg(m, -1);

   result = m->MSGT_CLOSE_RESULT;

   vfree(m);

   return result;
}

unsigned int sys_filelength(int fd)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_FILELENGTH;
   m->source = current;

   m->MSGT_FILELENGTH_FD = fd;

   send_msg(TASK_VFS, m);
   wait_msg(m, -1);

   result = m->MSGT_FILELENGTH_RESULT;

   vfree(m);

   return result;
}

int sys_seek(int fd, unsigned int pos, int whence)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_SEEK;
   m->source = current;

   m->MSGT_SEEK_FD = fd;
   m->MSGT_SEEK_POS = pos;

   send_msg(TASK_VFS, m);
   wait_msg(m, MSG_SEEK_REPLY);

   result = m->MSGT_SEEK_RESULT;

   vfree(m);

   return result;
}

int sys_tell(int fd)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_TELL;
   m->source = current;

   m->MSGT_TELL_FD = fd;

   send_msg(TASK_VFS, m);
   wait_msg(m, MSG_TELL_REPLY);

   result = m->MSGT_TELL_RESULT;

   vfree(m);

   return result;
}

int sys_read(unsigned int fd, unsigned char *buffer, unsigned int size)
{
   struct ipc_message m;
   int result;

   m.type = MSG_READ;

   m.MSGT_READ_FD = fd;
   m.MSGT_READ_BUFFER = buffer;
   m.MSGT_READ_SIZE   = size;

   m.source = current;
   send_msg(TASK_VFS, &m);
   wait_msg(&m, MSG_READ_REPLY);

   result = m.MSGT_READ_RESULT;

   return result;
}

int sys_write(unsigned int fd, unsigned char *buffer, unsigned int size)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_WRITE;

   m->MSGT_READ_FD = fd;
   m->MSGT_READ_BUFFER = buffer;
   m->MSGT_READ_SIZE   = size;

   m->source = current;
   send_msg(TASK_VFS, m);
   wait_msg(m, MSG_WRITE_REPLY);

   result = m->MSGT_READ_RESULT;

   vfree(m);

   return result;
}

int sys_actdev(unsigned char *name)
{
  strcpy(current->actual_device,name);
  return SUCCESS;
}

int sys_exec_sysprogramm(unsigned int addr, unsigned char *programmname)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_CREATE_NEW_APPLICATION;

   m->MSGT_PROGRAMM_ADDRESS = addr;

   m->MSGT_PROGRAMM_NAME = (unsigned char*)valloc(strlen(programmname)+1);
   strcpy(m->MSGT_PROGRAMM_NAME, programmname);

   m->MSGT_PROGRAMM_PRIVILEGE = KERNEL_TASK;

   m->source = current;
   send_msg(TASK_VFS, m);
   wait_msg(m, -1);

   result = m->MSGT_CREATE_PROGRAMM_RESULT;

   vfree(m->MSGT_PROGRAMM_NAME);
   vfree(m);

   return result;
}

int sys_exec_userprogramm(unsigned int addr, unsigned char *programmname)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_CREATE_NEW_APPLICATION;

   m->MSGT_PROGRAMM_ADDRESS = addr;

   m->MSGT_PROGRAMM_NAME = (unsigned char*)valloc(strlen(programmname)+1);
   strcpy(m->MSGT_PROGRAMM_NAME, programmname);

   m->MSGT_PROGRAMM_PRIVILEGE = USER_TASK;

   m->source = current;
   send_msg(TASK_VFS, m);
   wait_msg(m, -1);

   result = m->MSGT_CREATE_PROGRAMM_RESULT;

   vfree(m->MSGT_PROGRAMM_NAME);
   vfree(m);

   return result;
}


// GUI Application Programm Interface (API)
void sys_redraw()
{
   struct ipc_message *m;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_GUI_WINDOW_REDRAW;
   m->MSGT_WINDOW = current->window;
   m->source = current;

   send_msg(TASK_GUI, m);
   wait_msg(m, -1);

   vfree(m);
}


void sys_add_item(void* object)
{
   struct ipc_message *m;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_GUI_ADD_ITEM;
   m->MSGT_WINDOW = current->window;
   m->MSGT_ITEM   = object;
   m->source = current;

   send_msg(TASK_GUI, m);
   wait_msg(m, -1);

   vfree(m);
}

void sys_out_window(unsigned char *str)
{
   struct ipc_message *m;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_GUI_DRAW_STRING;
   m->MSGT_WINDOW = current->window;
   m->MSGT_GUI_DRAW_STRING_STR = str;
   m->source = current;

   send_msg(TASK_GUI, m);
   wait_msg(m, -1);

   vfree(m);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/* Network Application Interface (Sockets)                                 */
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
int sys_socket(unsigned int family, unsigned int type, unsigned int protocol)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_NET_SOCKET_SOCKET;

   m->MSGT_NET_SOCKET_SOCKET_FAMILY = family;
   m->MSGT_NET_SOCKET_SOCKET_TYPE = type;
   m->MSGT_NET_SOCKET_SOCKET_PROTOCOL = protocol;
   m->MSGT_NET_SOCKET_SOCKET_PID = current->pid;

   m->source = current;
   send_msg(net_pid, m);
   wait_msg(m, MSG_NET_SOCKET_SOCKET_REPLY);

   result = m->MSGT_NET_SOCKET_SOCKET_RESULT;

   vfree(m);

   return result;
}

int sys_bind(unsigned int sock, void* saddr, unsigned int size)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_NET_SOCKET_BIND;

   m->MSGT_NET_SOCKET_BIND_SOCKET = sock;
   m->MSGT_NET_SOCKET_BIND_SADDR = saddr;
   m->MSGT_NET_SOCKET_BIND_SIZE = size;

   m->source = current;
   send_msg(net_pid, m);
   wait_msg(m, MSG_NET_SOCKET_BIND_REPLY);

   result = m->MSGT_NET_SOCKET_BIND_RESULT;

   vfree(m);

   return result;
}

int sys_recvfrom(unsigned int sock, unsigned char* buffer , unsigned int size, void* source)
{
   struct ipc_message m;
   int result;

   m.type = MSG_NET_SOCKET_READ;

   m.MSGT_NET_SOCKET_READ_SOCKET = sock;
   m.MSGT_NET_SOCKET_READ_PID = current->pid;
   m.MSGT_NET_SOCKET_READ_BUFFER = buffer;
   m.MSGT_NET_SOCKET_READ_SIZE = size;
   m.MSGT_NET_SOCKET_READ_SOURCE = source;

   m.source = current;
   send_msg(net_pid, &m);
   wait_msg(&m, MSG_NET_SOCKET_READ_REPLY);

   result = m.MSGT_NET_SOCKET_READ_RESULT;

   return result;
}

int sys_sendto(unsigned int sock, unsigned char* buffer , unsigned int size, void* source)
{
   struct ipc_message m;
   int result;

   m.type = MSG_NET_SOCKET_WRITE;

   m.MSGT_NET_SOCKET_WRITE_SOCKET = sock;
   m.MSGT_NET_SOCKET_WRITE_BUFFER = buffer;
   m.MSGT_NET_SOCKET_WRITE_SIZE = size;
   m.MSGT_NET_SOCKET_WRITE_SOURCE = source;

   m.source = current;
   send_msg(net_pid, &m);
   wait_msg(&m, MSG_NET_SOCKET_WRITE_REPLY);

   result = m.MSGT_NET_SOCKET_WRITE_RESULT;

   return result;
}

int sys_connect(unsigned int sock, void* source)
{
   struct ipc_message m;
   int result;

   m.type = MSG_NET_SOCKET_CONNECT;

   m.MSGT_NET_SOCKET_CONNECT_SOCKET = sock;
   m.MSGT_NET_SOCKET_CONNECT_SOURCE = source;

   m.source = current;
   send_msg(net_pid, &m);
   wait_msg(&m, MSG_NET_SOCKET_CONNECT_REPLY);

   result = m.MSGT_NET_SOCKET_CONNECT_RESULT;

   return result;
}

int sys_send_ip(unsigned char* buffer, unsigned int size, unsigned char *dest)
{
   struct ipc_message *m;
   int result;

   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

   m->type = MSG_NET_SEND;

   m->MSGT_NET_SEND_BUFFER = buffer;
   m->MSGT_NET_SEND_DEST = dest;
   m->MSGT_NET_SEND_SIZE = size;

   m->source = current;
   send_msg(net_pid, m);
   wait_msg(m, -1);

   result = m->MSGT_NET_SEND_RESULT;

   vfree(m);

   return result;
}

