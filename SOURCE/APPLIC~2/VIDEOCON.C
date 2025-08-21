/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\videocon.c
/* Last Update: 19.08.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the video configuration application.
/************************************************************************/
#include "newgui.h"
#include "sched.h"
#include "controls.h"
#include "svga\modes.h"
#include "svga\svgadev.h"

extern display_device_driver *registered_drivers[];

window_t *win_videocon;

ctl_button_t *but_cards[MAX_SVGA_DEVICES]; // this are the buttons for each card
ctl_button_t *but_modes[MAX_SVGA_RESOLUTIONS] = {NULL}; // this are button for each graphic mode
ctl_button_t *but_init;

ctl_text_t *txt_state, *txt_mode;
ctl_input_t *ipt_active, *ipt_driver, *ipt_chipset, *ipt_vendor, 
			*ipt_memory, *ipt_mode;

// ID's reserved from 1 to 50 for each device driver
// ID's reserved from 100 to 150 for each resolution

unsigned int selected_card = 0, selected_mode = 0;


void redraw_videocon(void)
{
}

int videocon_detect_card(unsigned int i)
{
  int j, detected = 0;
  char str[255];
  mode_info_t *mode_table;

  for (j=0;j<MAX_SVGA_RESOLUTIONS;j++)
  {
    if (but_modes[j])
	{
	  destroy_control(win_videocon,but_modes[j],0);
	  but_modes[j] = NULL;
	}
  }

  destroy_control(win_videocon,but_init,0);

  win_draw_fill_rect(win_videocon,290,180,390,260,COLOR_LIGHTGRAY);
  win_draw_fill_rect(win_videocon,426,205,500,225,COLOR_LIGHTGRAY);


  detected = registered_drivers[i]->detect();
  mode_table = registered_drivers[i]->mode_table;

  switch (detected)
  {
    case 0:   sprintf(str,"Detection failed");
		      break;
    case 255: sprintf(str,"Detection failed");
		      break;
    default:  sprintf(str,"%s %s",registered_drivers[i]->name,registered_drivers[i]->desc);
		      break;
  }

  SetItemText_input(win_videocon, ipt_mode, "");
  SetItemText_input(win_videocon, ipt_driver, str);  

  if ((detected == SVGAOK) && (mode_table != NULL))
  {
	i = 0;
	while (MODE_ENTRY_VALID(mode_table[i]))
	{
	  sprintf(str,"%dx%dx%d",mode_table[i].xdim,mode_table[i].ydim,mode_table[i].bpp);
      but_modes[i] = create_button(win_videocon, 290, 180+i*20, 100, 18, str, 100+i);
	  if (i == 0)
		SetItemText_input(win_videocon, ipt_mode, str);
	  i++;
	}

    but_init = create_button(win_videocon, 426, 205, 70, 18, "Init", 401);
  }

  return detected;
}

void videocon_layout(void)
{
  int i, j = 0;
  char str[255];
  display_device_driver *driver;
  mode_info_t *mode_table = active_display_driver->mode_table;

  for (i=0;i<MAX_SVGA_RESOLUTIONS;i++)
	but_modes[i] = NULL;

  create_text(win_videocon,10,40,210,25,"Installed graphic drivers:",TEXT_ALIGN_LEFT,201);
  create_text(win_videocon,10,10,510,25,"Active graphic driver:",TEXT_ALIGN_LEFT,202);
  txt_state = create_text(win_videocon,280,40,300,25,"Graphic card informations:",TEXT_ALIGN_LEFT,203);
  create_text(win_videocon,280,62,100,25,"Driver:",TEXT_ALIGN_LEFT,204);
  create_text(win_videocon,280,85,100,25,"Chipset:",TEXT_ALIGN_LEFT,205);
  create_text(win_videocon,280,107,100,25,"Vendor:",TEXT_ALIGN_LEFT,206);
  create_text(win_videocon,280,128,100,25,"Memory:",TEXT_ALIGN_LEFT,207);
  create_text(win_videocon,410,128,100,25,"KB",TEXT_ALIGN_LEFT,208);
  create_text(win_videocon,280,155,150,25,"Supported modes:",TEXT_ALIGN_LEFT,209);

  ipt_active = create_input(win_videocon,195,12,250,18,301);
  ipt_driver = create_input(win_videocon,350,63,150,18,302);
  ipt_chipset = create_input(win_videocon,350,86,150,18,303);
  ipt_vendor = create_input(win_videocon,350,108,100,18,304);
  ipt_memory = create_input(win_videocon,350,129,50,18,305);
  ipt_mode = create_input(win_videocon,410,180,97,18,306);

  but_init = create_button(win_videocon, 426, 205, 70, 18, "Init", 401);

  sprintf(str,"%s %s",active_display_driver->name,active_display_driver->desc);
  SetItemText_input(win_videocon, ipt_active, str);
  SetItemText_input(win_videocon, ipt_driver, str);


  for (i=0;i<MAX_SVGA_DEVICES;i++)
  {
    if (registered_drivers[i] != 0)
    {
       driver = registered_drivers[i];
       sprintf(str,"%s %s",driver->name,driver->desc);

       but_cards[j] = create_button(win_videocon, 10, 65+j*22, 255, 18, str, i+1);
       j++;
    }
  }

  if (mode_table != NULL)
  {
	i = 0;
	while (MODE_ENTRY_VALID(mode_table[i]))
	{
	  sprintf(str,"%dx%dx%d",mode_table[i].xdim,mode_table[i].ydim,mode_table[i].bpp);
      but_modes[i] = create_button(win_videocon, 290, 180+i*20, 100, 18, str, 100+i);
	  if (i == 0)
		SetItemText_input(win_videocon, ipt_mode, str);
	  i++;
	}
  }

}


void videocon_select_mode(int nr)
{
  char str[256];
  mode_info_t *mode_table = registered_drivers[selected_card]->mode_table;

  sprintf(str,"%dx%dx%d",mode_table[nr].xdim,mode_table[nr].ydim,mode_table[nr].bpp);
  SetItemText_input(win_videocon, ipt_mode, str);
}

void videocon_init()
{
  mode_info_t *mode_table;
  int flags;

  save_flags(flags);
  cli();

  if (active_display_driver != registered_drivers[selected_card])
  {
    display_driver_init(registered_drivers[selected_card]);
  }

  mode_table = active_display_driver->mode_table;

  display_setmode(&mode_table[selected_mode]);

  restore_flags(flags);
}

void videocon_ctl_react(int id)
{

  if ((id > 0) && (id <= 50))
  {
    videocon_detect_card(id - 1);
    selected_card = id - 1;
  }
  else if ((id >= 100) && (id < 150))
  {
	videocon_select_mode(id - 100);
    selected_mode = id - 100;
  }
  else if (id == 401)
  {
	videocon_init();
  }

}

void task_videocon()
{
  struct ipc_message m;
  int i = 0;
  int ret;

  win_videocon = create_window("SkyOS Video Configuration",50,100,530,300,WF_STANDARD);

  videocon_layout();

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_videocon);
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:
		    videocon_ctl_react(m.MSGT_GUI_CTL_ID);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_videocon, WF_STOP_OUTPUT, 0);
            set_wflag(win_videocon, WF_MSG_GUI_REDRAW, 0);
            redraw_videocon();
            break;
     }
  }
}

void app_videocon()
{
  if (!valid_app(win_videocon))
  {
    CreateKernelTask(task_videocon,"videocon",NP,0);
  }
}
