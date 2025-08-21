/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\rtc\rtc825x.c
/* Last Update: 24.08.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      : Soundblaster Book
/************************************************************************/
/* Definition:
/*   A Real time clock device driver for the RTC8253/8254 chip.
/************************************************************************/
#include "newgui.h"
#include "controls.h"
#include "sched.h"
#include "devman.h"

/* Export functions */
void set_clock(int hz);

static unsigned char debug = 1;
static unsigned int current_freq = 0;

static ctl_text_t *txt_info_1;
static ctl_text_t *txt_info_2;
static ctl_text_t *txt_info_3;

static ctl_text_t *txt_freq;
static ctl_text_t *txt_debug;


static ctl_input_t *ipt_freq;
static ctl_input_t *ipt_debug;

static ctl_button_t *but_apply;

static window_t *win_rtc825x;

void set_clock(int hz)
{
  unsigned int flags;
  unsigned int count;

  save_flags(flags);
  cli();

  if ( hz<=0 )
  {
     count = 0xFFFF; // set timer to 18.2 hz
     hz = 18;
  }
  else
  {
    count = 1193180 / hz;
  }

  if (debug)
    printk("rtc825x.c: New Timerfreq is %dhz, factor: %d",hz, count);

  outportb(0x43, 0x36);
  outportb(0x40, count & 0xFF);
  outportb(0x40, (count >> 8) & 0xFF);

  restore_flags(flags);

  current_freq = hz;
}

void rtc825x_layout(void)
{
  char str[255];

  txt_info_1 = create_text(win_rtc825x,10,10,310,12,"Realtime clock device driver V1.0a",TEXT_ALIGN_LEFT,1);
  txt_info_2 = create_text(win_rtc825x,10,24,310,12,"for PC-AT/8253 and PC-XT/8254 Chips",TEXT_ALIGN_LEFT,2);
  txt_info_3 = create_text(win_rtc825x,10,38,310,12,"Copyright (c) 1999 by Szeleney Robert",TEXT_ALIGN_LEFT,3);

  txt_freq   = create_text(win_rtc825x,10,70,160,18,"Current timer freq: ",TEXT_ALIGN_LEFT,5);
  txt_debug  = create_text(win_rtc825x,10,100,160,18,"Debug level       : ",TEXT_ALIGN_LEFT,6);

  ipt_freq   = create_input(win_rtc825x,180,70,60,18,10);
  ipt_debug   = create_input(win_rtc825x,180,100,60,18,11);

  but_apply  = create_button(win_rtc825x, 10, 130, 70, 18, "Apply", 20);

  sprintf(str,"%d",current_freq);
  SetItemText_input(win_rtc825x, ipt_freq, str);

  sprintf(str,"%d",debug);
  SetItemText_input(win_rtc825x, ipt_debug, str);
}

void rtc825x_task()
{
  struct ipc_message *m;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_rtc825x = create_window("RTC825x device driver",300,100,400,200,WF_STANDARD);

  rtc825x_layout();

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m->MSGT_GUI_CTL_ID == 20)
            {
              int newfreq;
              char str[255];

              GetItemText_input(win_rtc825x, ipt_freq, str);
              newfreq = atoi(str);

              set_clock(newfreq);

              GetItemText_input(win_rtc825x, ipt_debug, str);
              debug = atoi(str);
            }
            break;

       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_rtc825x);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_rtc825x, WF_STOP_OUTPUT, 0);
            set_wflag(win_rtc825x, WF_MSG_GUI_REDRAW, 0);
            break;
     }
  }
}

/* Device Manager entry function */
void rtc825x_configure(void)
{
  if (!valid_app(win_rtc825x))
    CreateKernelTask(rtc825x_task,"rtc825x",NP,0);
}


void rtc825x_init(void)
{
   unsigned int id;

   id = register_hardware(DEVICE_IDENT_SYSTEM_PHERIPHERAL,
                          "Real time clock 8253/8254",0);

   register_function(id, rtc825x_configure);

   register_port(id, 0x40, 0x43);
   register_irq(id, 0x0);
}

