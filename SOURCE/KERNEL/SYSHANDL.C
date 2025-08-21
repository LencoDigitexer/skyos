/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\syshandl.c
/* Last Update: 02.03.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus from :
/************************************************************************/
/* Definition:
/*   This file handles all syscalls.
/*
/*
/* WARNING!!!
/*   Pointers in received parameter messages are usertask-orientated.
/*   They need to be converted to kernel memory space.
/* WARNING!!!
/************************************************************************/

#include "sched.h"
#include "msg_lib.h"
#include "twm.h"

extern struct task_struct *current;
struct twm_window syscall_window={0};

void do_syscall_debug(struct ipc_message *m)
{
//   unsigned char str[255];

//   strcpy(str, m->MSGT_DEBUG_STR + current->code_start);
//   str[strlen(str)+1] = 0;

//   show_msgf("testoutput");
}
extern int newgui_pid;

void do_syscall_create_client_window(struct ipc_message *m)
{
   unsigned char str[255];

   strcpy(str,m->MSGT_GUI_WNAME + current->code_start);

   m->MSGT_GUI_WNAME = str;

   send_msg(newgui_pid, m);
   wait_msg(m, MSG_GUI_CREATE_CLIENT_WINDOW_REPLY);
}

void syscall(unsigned int addr)
{
  unsigned char str[255];
  unsigned int cr2;
  struct tss_struct *tss;
  unsigned char *p_c;
  int i;
  struct ipc_message *m;

/*  syscall_window.x = 100;
  syscall_window.length = 820;
  syscall_window.y = 230;
  syscall_window.heigth = 500;
  syscall_window.style = 128;
  strcpy(syscall_window.title, current->name);

  draw_window(&syscall_window);

  sprintf(str,"Syscall captured.");
  out_window(&syscall_window,str);
  out_window(&syscall_window,"");
  sprintf(str,"Current Task: %s/%d",current->name,current->pid);
  out_window(&syscall_window,str);
  out_window(&syscall_window,"");
  sprintf(str,"Address of message      : 0x%00000008X", addr);
  out_window(&syscall_window,str);
  sprintf(str,"Address of data segment : 0x%00000008X", current->code_start);
  out_window(&syscall_window,str);
  sprintf(str,"Absolute address        : 0x%00000008X", current->code_start + addr);
  out_window(&syscall_window,str);

  out_window(&syscall_window, "");
  out_window(&syscall_window, "");
  out_window(&syscall_window, "");
  sprintf(str,"Message Type   : %d",m->type);
  out_window(&syscall_window, str);*/


/* This is the main syscall routine. Here is decided, which message
   is sended to which kernel-task.
   Each syscall handler is coded in syshandl.c */

//   m = (struct ipc_message*)valloc(sizeof(struct ipc_message));

//   memcpy(m, current->code_start + addr, sizeof(struct ipc_message));

/*   switch (m->type)
   {
      case MSG_DEBUG:  do_syscall_debug(m);
                       break;

      case MSG_GUI_CREATE_CLIENT_WINDOW:
                       do_syscall_create_client_window(m);
                       break;
   }*/

//   vfree(m);
}

