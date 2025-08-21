/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\msg.c
/* Last Update: 12.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus from :
/************************************************************************/
/* Definition:
/*   SKY interprocess communication module
/*   Functions to communicate with other tasks by messages
/*   send_msg, wait_msg,...
/*   Dynamic coded
/************************************************************************/
#include "sched.h"
#include "twm.h"

#define NULL (void*)0

//#define MSG_DEBUG 1

struct ipc_message *ipc_log=NULL;
extern unsigned int timerticks;

struct twm_window msg_error={0};

unsigned char str[255] = {0};

void ipclog_search(unsigned int addr);

extern struct task_struct *current;

/* source: -1 = ANY */
void wait_msg(struct ipc_message * message, int type)
{
   struct ipc_message *ipclog;
   struct ipc_message *m;
   struct ipc_message *oldmsg;
   unsigned int flags;

   save_flags(flags);
   cli();

   current->waitingfor = type;

   if (current->msgbuf == NULL)
   {
wait_for_message:                       // set current task to WAITING

      current->state = TASK_WAITING;

      Scheduler();
      while (current->state == TASK_WAITING);
   }

   m = current->msgbuf;
   if ((m->type == type) || (type == -1))         // first msg OK
   {
     memcpy(message, m ,sizeof(struct ipc_message));

     current->msgbuf = current->msgbuf->next;

     vfree(m);

     restore_flags(flags);
     return;
   }

// get message with right type

   while ((m->next) && (m->next->type != type))
     m = m->next;

   if (!m->next) goto wait_for_message;

   memcpy(message, m->next ,sizeof(struct ipc_message));

   oldmsg = m->next;
   m->next = m->next->next;

   if (vfree(oldmsg))
   {
     cli();

     msg_error.x = 200;
     msg_error.length = 400;
     msg_error.y = 180;
     msg_error.heigth = 240;
     msg_error.style = 128;
     strcpy(msg_error.title, "Fatal system error occured");

     draw_window(&msg_error);

     sprintf(str, "Source file : msg.c");
     out_window(&msg_error, str);
     sprintf(str, "Error       : unable to free allocated message");
     out_window(&msg_error, str);
     sprintf(str, "              at %d",m);
     out_window(&msg_error, str);
     sprintf(str, "Current Task: %s",current->name);
     out_window(&msg_error, str);
     sprintf(str, "Type        : %d",message->type);
     out_window(&msg_error, str);
     sprintf(str, "Source      : PID : %d %s",message->source->pid,message->source->name);
     out_window(&msg_error, str);
     sprintf(str, "Sender      : PID : %d",message->sender);
     out_window(&msg_error, str);
     sprintf(str, "Destination : PID : %d",message->reciever);
     out_window(&msg_error, str);
     sprintf(str, "");
     out_window(&msg_error, str);
     sprintf(str, "Message type information:");
     switch (message->type)
     {
       case 1: sprintf(str,"Devicename: %s",message->MSGT_READ_BLOCK_DEVICENAME); break;
       default : sprintf(str,"Unknown or unhandled type!"); break;
     }
     out_window(&msg_error, str);
     sprintf(str, "");
     out_window(&msg_error, str);
     sprintf(str, "       System halted. Please reboot!");
     out_window(&msg_error, str);
     while(1);
   }

   restore_flags(flags);
}

int send_msg(int destination,   struct ipc_message *m)
{
  struct ipc_message *msgbuf;
  struct ipc_message *ipclog;
  struct task_struct *t;
  unsigned int flags;
  unsigned char str[255];

  save_flags(flags);
  cli();

  ipclog = ipc_log;

  m->reciever = destination;
  m->sender = current->pid;
  m->next = NULL;

//  sprintf(str,"Msg %0004d from %003d to %003d",m->type, m->sender, destination);
//  show_msg(str);

  t = (struct task_struct*)GetTask(destination);
  if (t == NULL)
  {
      alert("File: msg.c  Function: send_msg\n\n%s%d%s%d",
            "Invalid destination pid. Source: ",current->pid,
          "\nDestination pid: ", destination);
     restore_flags(flags);
     return -1;
  }

  msgbuf = t->msgbuf;

  if (msgbuf == NULL)   // first message in queue
  {
    t->msgbuf = (struct ipc_message*)valloc(sizeof(struct ipc_message));
    memcpy(t->msgbuf, m, sizeof(struct ipc_message));
    t->msgbuf->next = NULL;
  }
  else                  // already messages in queue
  {
    //printk("msg.c: Add to queue for task %s",t->name);
    while (msgbuf->next != NULL)
     msgbuf = msgbuf->next;

    msgbuf->next = (struct ipc_message*)valloc(sizeof(struct ipc_message));
    memcpy(msgbuf->next, m, sizeof(struct ipc_message));

    msgbuf->next->next = NULL;
  }

#ifdef MSG_DEBUG
  if (ipc_log == NULL)
  {
    ipc_log = (struct ipc_message*)valloc(sizeof(struct ipc_message));
    memcpy(ipc_log, m, sizeof(struct ipc_message));
    ipc_log->next = NULL;
  }
  else                  // already messages in queue
  {
    while (ipclog->next != NULL)
     ipclog = ipclog->next;

    ipclog->next = (struct ipc_message*)valloc(sizeof(struct ipc_message));
    memcpy(ipclog->next, m, sizeof(struct ipc_message));
    ipclog->next->next = NULL;
  }
#endif

  if (t->state == TASK_WAITING)
    t->state  = TASK_READY;
//   task_control(t->pid, TASK_READY);

  restore_flags(flags);
}


void dump_ipclog(void)
{
  struct ipc_message *msg;

  msg = ipc_log;

  while (msg)
  {
    printk("From %d to %d, real: %d, type: %d", msg->sender, msg->reciever, msg->source->pid,msg->type);
    msg = msg->next;
  }
}

void msg_debug(void)
{
  struct task_struct *t;
  int i;
  struct ipc_message *m;
  int j;
  unsigned int flags;

  for (i=0;i<=100;i++)
  {
    save_flags(flags);
    cli();

    t = GetTask(i);
    m = t->msgbuf;
    j = 0;

    while (m)
    {
      j++;

      m=m->next;
    }

    restore_flags(flags);


    printk("Task %d (%s) with %d messages.",t->pid, t->name, j);
  }
}

