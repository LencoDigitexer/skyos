/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 -1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\sound\sc_int.c
/* Last Update: 10.08.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Internet, Linux, Soundblaster Book, PC Underground
/************************************************************************/
/* Definition:
/*   This is the sound card interface.
/************************************************************************/
#include "newgui.h"
#include "controls.h"
#include "sched.h"
#include "imgdec.h"
#include "fcntl.h"

unsigned int ifsound_pid = 0;
extern struct task_struct *current;

void sound_play_click(void)
{
  sb_play_click();
}

void system_sound(unsigned char *str)
{
    struct ipc_message m;

    m.type = MSG_SOUND_PLAY_SYSTEM;
    m.MSGT_SOUND_PLAY_BUF = str;
    m.source = current;

    send_msg(ifsound_pid , &m);
}


void ifsound_system(struct ipc_message *m)
{
   int fd;

   fd = sys_open(m->MSGT_SOUND_PLAY_BUF,O_RDONLY);

   if (fd<0)
   {
     show_msgf("if_sound.c: Error while open file");
     show_msgf("            to play system sound.");
     return;
   }

   play_wave(fd);
   sys_close(fd);
}

void ifsound_task(void)
{
  struct ipc_message *m;
  unsigned int ret;
  unsigned char *buffer;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_SOUND_PLAY_SYSTEM:
            ifsound_system(m);
            break;
     }
  }
}

void ifsound_init(void)
{
  int res;

  res = ((struct task_struct*)CreateKernelTask((unsigned int)ifsound_task, "if_sound", RTP,0))->pid;

  printk("if_sound.c: Soundcard interface created. PID = %d",res);
  ifsound_pid = res;
}

