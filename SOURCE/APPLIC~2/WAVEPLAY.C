/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\waveplay.c
/* Last Update: 11.08.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
#include "newgui.h"
#include "controls.h"
#include "sched.h"
#include "imgdec.h"
#include "fcntl.h"

window_t *win_waveplay;

extern int sb_pid;

extern struct task_struct *current;

#define PACKED __attribute__((packed))

#define DEBUG_ON 1
#define DEBUG if (DEBUG_ON == 1)

struct wave_header
{
   unsigned char riff[4]                PACKED;
   unsigned int  filelen                PACKED;
   unsigned char wave[4]                PACKED;
   unsigned char fmt[4]                 PACKED;
   unsigned int  blocklen               PACKED;

   unsigned short format                PACKED;
   unsigned short channels              PACKED;
   unsigned int   samplerate            PACKED;
   unsigned int  bytesprosec            PACKED;
   unsigned short sampletyp             PACKED;

   unsigned short samplebits            PACKED;
};


/************************************************************************/
/* The fd is already the opened wave file, with offset at the first
/* data byte
/*
/* size is the word from the wave header for size
/************************************************************************/
int wave_read_play(int fd, unsigned int size)
{
  unsigned char *buffer;
  struct ipc_message m;

  buffer = (unsigned char*) valloc(size);

  if (!buffer)
  {
     show_msgf("waveplay.c: out of memory.");
     return;
  }

  DEBUG show_msgf("waveplay.c: reading %s bytes...",size);
  sys_read(fd, buffer, size);

  DEBUG show_msgf("waveplay.c: sending msg to sb...");
//  sb_play_buffer(buffer, size);

  m.type = MSG_SOUND_PLAY_BUFFER;
  m.source = current;
  m.MSGT_SOUND_PLAY_BUF = buffer;
  m.MSGT_SOUND_PLAY_SIZE = size;
  send_msg(sb_pid, &m);

  wait_msg(&m, MSG_SOUND_PLAY_FIN);

  DEBUG show_msgf("waveplay.c: fin received.");
}

int play_wave(int fd)
{
  struct wave_header header;
  unsigned char buffer[512];
  int i;

  int pos;
  unsigned int size;


  sys_read(fd, &header, sizeof( struct wave_header));

  /* check if file is a RIFF or WAVE */
  if (!strncmp(header.riff,"RIFF"))
    DEBUG show_msgf("waveplay.c: File is a RIFF");

  else if (!strncmp(header.wave,"WAVE"))
    DEBUG show_msgf("waveplay.c: File is a WAVE");

  else
  {
    show_msgf("waveplay.c: File is not a wave format.");
    return -1;
  }

  /* check if mono, 8bit */
  if (header.channels == 1)
  {
     DEBUG show_msgf("waveplay.c: Wave using mono.");
  }
  else
  {
     show_msgf("waveplay.c: Wave using stereo. Not supported");
     return;
  }

  if (header.sampletyp == 1)
  {
    DEBUG show_msgf("waveplay.c: Wave sample typ 1.");
  }
  else
  {
     show_msgf("waveplay.c: Wave not sample typ 1. Not supported");
     return;
  }

  if (header.samplebits == 8)
  {
     DEBUG show_msgf("waveplay.c: Wave sample bits 8.");
  }
  else
  {
     show_msgf("waveplay.c: Wave sample bits %d. Not supported",header.samplebits);
     return;
  }

  /* seek to size after the string 'data' */
  sys_read(fd, buffer, 512);

  for (i=0; i<508;i++)
  {
     if (!strncmp(buffer+i,"data"))
       break;
  }

  if (i==508)
  {
     show_msgf("waveplay.c: no data block found.");
     return -1;
  }

  sys_seek(fd, i + 4, 0);

  DEBUG show_msgf("waveplay.c: offset is %d",pos);

  /* read wave size */
  sys_read(fd, &size, 4);

  DEBUG show_msgf("waveplay.c: wave size: %d",size);

  wave_read_play(fd, size);
}

void waveplay_task()
{
  struct ipc_message m;
  ctl_button_t *but1;
  ctl_input_t *input;
  char buf[256];
  int fd;

  win_waveplay = create_window("WAVE Player",300,100,400,300,WF_STANDARD);

  but1 = create_button(win_waveplay,20,20,60,25,"Load",1);
  input = create_input(win_waveplay,250,20,100,20,4);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m.MSGT_GUI_CTL_ID == 1)
            {
              GetItemText_input(win_waveplay, input, buf);

              fd = sys_open(buf,O_RDONLY|O_BINARY);

              if (fd >= 0)
              {
                show_msgf("waveplay.c: file opend.");

                play_wave(fd);
              }
              else
                show_msgf("waveplay.c: Error while opening file.");

              sys_close(fd);
            }
            break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_waveplay);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_waveplay, WF_STOP_OUTPUT, 0);
            set_wflag(win_waveplay, WF_MSG_GUI_REDRAW, 0);
            break;
     }

  }

}

void waveplay_init_app()
{
  if (!valid_app(win_waveplay))
    CreateKernelTask(waveplay_task,"waveplay",NP,0);
}

