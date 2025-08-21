/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\devman.c
/* Last Update: 24.08.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the device manager
/************************************************************************/
#include "newgui.h"
#include "sched.h"
#include "controls.h"
#include "devman.h"

#define MAX_BUT_CLASSES 20
#define MAX_BUT_DEVICES 20

#define MAX_BUT_IRQ     5
#define MAX_BUT_DMA     5
#define MAX_BUT_PORT     5
#define MAX_BUT_MEMORY     5

window_t *win_devman;

extern struct list *listhardware;
unsigned int selected_device = 0;

ctl_button_t *but_classes[MAX_BUT_CLASSES] = {NULL};
ctl_button_t *but_devices[MAX_BUT_DEVICES] = {NULL};

ctl_button_t *but_irq[MAX_BUT_IRQ] = {NULL};
ctl_button_t *but_port[MAX_BUT_PORT] = {NULL};
ctl_button_t *but_dma[MAX_BUT_DMA] = {NULL};
ctl_button_t *but_memory[MAX_BUT_MEMORY] = {NULL};

struct s_hardware *hardware_to_device[MAX_BUT_DEVICES] = {NULL};

ctl_button_t *but_configure;
ctl_button_t *but_start, *but_stop;

ctl_text_t *txt_classes, *txt_devices;

ctl_text_t *txt_irq;
ctl_text_t *txt_dma;
ctl_text_t *txt_port;
ctl_text_t *txt_memory;


ctl_menu_t *devman_menu;

// button id's from 0 to MAX_BUT_CLASSES reserved.
// button id's from 100 to MAX_BUT_DEVICES reserved.

/* Button ID */
#define ID_CONFIGURE 500
#define ID_START 501
#define ID_STOP 502

#define ID_BUT_RES 505

/* Menu ID */
#define ID_MENU 600
#define ID_ACTION 601
#define ID_EXIT 602

/* Text ID */
#define ID_TXT1 700
#define ID_TXT_IRQ 701
#define ID_TXT_DMA 702
#define ID_TXT_PORT 703
#define ID_TXT_MEMORY 704


static const char *device_ident_name[] =
{ "Other Devices",
  "Storage Devices",
  "Network Controller",
  "Display Controller",
  "Multimedia Devices",
  "Memory",
  "Bridge Controller",
  "Communications",
  "System Pheripheral",
  "Input Device",
  "Docking Station",
  "CPU Type",
  "Serial Bus",
  NULL
};

void redraw_devman(void)
{
}

void devman_layout(void)
{
  int i, j = 0;
  char str[255];
  char *pstr;

  /* Create Menu */
  devman_menu = create_menu(win_devman,ID_MENU);
  add_menuitem(win_devman, devman_menu, "Action", ID_ACTION, 0);
  add_menuitem(win_devman, devman_menu, "Exit", ID_EXIT, ID_ACTION);

  /* Create Device classes */
  i = 0;
  pstr = device_ident_name[0];

  txt_classes   = create_text(win_devman,10,10,200,25,"Installed devices:",TEXT_ALIGN_LEFT,ID_TXT1);
  while (pstr)
  {
    but_classes[i] = create_button(win_devman, 10, 35+i*16, 200, 14, device_ident_name[i], i);
    i++;

    pstr = device_ident_name[i];
  }
}

static void device_selected(unsigned int id)
{
   struct s_irq *irq;
   struct s_dma *dma;
   struct s_port *port;
   struct s_memory *memory;
   int i, j;
   unsigned char str[255];

   delete_res();

   redraw_window(win_devman);


   if (but_configure == NULL)
   {
      but_configure = create_button(win_devman,10, 300, 200, 18, "Configure Device...", ID_CONFIGURE);
      but_start     = create_button(win_devman,220, 300, 120, 18, "Start Device", ID_START);
      but_stop      = create_button(win_devman,350, 300, 120, 18, "Stop Device", ID_STOP);
   }

   selected_device =  id - 100;
   j = 0;

   i = 0;
   txt_irq = create_text(win_devman,530,26,50,14,"IRQ:",TEXT_ALIGN_LEFT,ID_TXT_IRQ);
   irq = (struct s_irq*)get_item(hardware_to_device[selected_device]->irq, i++);
   while (irq!=NULL)
   {
     sprintf(str,"%d",irq->nummer);
     but_irq[i] = create_button(win_devman, 530, 25+i*16, 50, 14, str, ID_BUT_RES);
     irq=(struct s_irq*)get_item(hardware_to_device[selected_device]->irq,i++);
   }
   j = i;

   i = 0;
   txt_dma = create_text(win_devman,530,(j*16) + 50 ,50,14,"DMA:",TEXT_ALIGN_LEFT,ID_TXT_DMA);
   dma = (struct s_dma*)get_item(hardware_to_device[selected_device]->dma, i++);
   while (dma!=NULL)
   {
     sprintf(str,"%d",dma->nummer);
     but_dma[i] = create_button(win_devman, 530, (j*16) + 50 + i*16, 50, 14, str, ID_BUT_RES);
     dma=(struct s_dma*)get_item(hardware_to_device[selected_device]->dma,i++);
   }
   j += i;

   i = 0;
   txt_port = create_text(win_devman,530,(j*16) + 50 ,50,14,"Ports:",TEXT_ALIGN_LEFT,ID_TXT_PORT);
   port = (struct s_port*)get_item(hardware_to_device[selected_device]->port, i++);
   while (port!=NULL)
   {
     sprintf(str,"0x%X - 0x%X",port->from, port->to);
     but_port[i] = create_button(win_devman, 530, (j*16) + 50 + i*16, 200, 14, str, ID_BUT_RES);
     port=(struct s_port*)get_item(hardware_to_device[selected_device]->port,i++);
   }
   j += i;

   i = 0;
   txt_memory = create_text(win_devman,530,(j*16) + 50 ,50,14,"Memory:",TEXT_ALIGN_LEFT,ID_TXT_PORT);
   memory = (struct s_memory*)get_item(hardware_to_device[selected_device]->memory, i++);
   while (memory!=NULL)
   {
     sprintf(str,"0x%X - 0x%X",memory->from, memory->to);
     but_memory[i] = create_button(win_devman, 530, (j*16) + 50 +i*16, 200, 14, str, ID_BUT_RES);
     memory=(struct s_memory*)get_item(hardware_to_device[selected_device]->memory,i++);
   }
   j += i;
}

static void delete_res(void)
{
  int i;

  for (i=0;i<MAX_BUT_IRQ; i++)
  {
     if (but_irq[i] != NULL)
     {
        destroy_control(win_devman, but_irq[i], 0);
        but_irq[i] = NULL;
     }
     if (but_dma[i] != NULL)
     {
        destroy_control(win_devman, but_dma[i], 0);
        but_dma[i] = NULL;
     }
     if (but_port[i] != NULL)
     {
        destroy_control(win_devman, but_port[i], 0);
        but_port[i] = NULL;
     }
     if (but_memory[i] != NULL)
     {
        destroy_control(win_devman, but_memory[i], 0);
        but_memory[i] = NULL;
     }
  }
  destroy_control(win_devman, txt_irq, 0);
  destroy_control(win_devman, txt_dma, 0);
  destroy_control(win_devman, txt_port, 0);
  destroy_control(win_devman, txt_memory, 0);
}

void devman_ctl_react(int id)
{
  int i=0;
  int j=0;
  char *pstr;

/* Check if a class button is pressed */
  if ((id >= 0) && (id <= MAX_BUT_CLASSES))
  {
    struct s_hardware *hard;

    delete_res();

    /* Delete each device button */
    for (i=0;i<MAX_BUT_DEVICES;i++)
    {
       if (but_devices[i] != NULL)
       {
         destroy_control(win_devman,but_devices[i],0);
         but_devices[i] = NULL;

         hardware_to_device[i] == NULL;
       }
    }

    selected_device = 0;

    /* Delete the configureation buttons */
    if (but_configure != NULL)
    {
        destroy_control(win_devman, but_configure, 0);
        but_configure = NULL;
    }
    if (but_start != NULL)
    {
        destroy_control(win_devman, but_start, 0);
        but_start = NULL;
    }
    if (but_stop != NULL)
    {
        destroy_control(win_devman, but_stop, 0);
        but_stop = NULL;
    }

    redraw_window(win_devman);

    /* Create each device button */
    i = 0;
    hard =(struct s_hardware*)get_item(listhardware,i++);

    while (hard!=NULL)
    {
      if (hard->devclass == id)
      {
        but_devices[j] = create_button(win_devman, 210, 35+(id*16)+j*16, 300, 14, hard->name, 100 + j);
        hardware_to_device[j] = hard;
        j++;
      }

      hard=(struct s_hardware*)get_item(listhardware,i++);
    }
  }

/* Check if a device button is pressed */
  else if ((id >= 100) && (id<(100+MAX_BUT_DEVICES)))
    device_selected(id);

/* Check if a configure button is pressed */
  else if (id == ID_CONFIGURE)
  {
     if (hardware_to_device[selected_device]->configure_driver == NULL)
     {
        show_msgf("devman.c: This driver isn't configureable yet.");
     }
     else hardware_to_device[selected_device]->configure_driver();
  }

}


void task_devman()
{
  struct ipc_message *m;
  int i = 0;
  int ret;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_devman = create_window("SkyOS Device Manager",50,100,700,500,WF_STANDARD);

  devman_layout();

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_devman);
            break;
       case MSG_GUI_CTL_MENU_ITEM:
            if (m->MSGT_GUI_CTL_ID == ID_EXIT)
              destroy_app(win_devman);

            break;
      case MSG_GUI_CTL_BUTTON_PRESSED:
           devman_ctl_react(m->MSGT_GUI_CTL_ID);
           break;

      case MSG_GUI_REDRAW:
           set_wflag(win_devman, WF_STOP_OUTPUT, 0);
           set_wflag(win_devman, WF_MSG_GUI_REDRAW, 0);
           redraw_devman();
           break;
     }
  }
}

void app_devman()
{
  if (!valid_app(win_devman))
  {
    CreateKernelTask(task_devman,"devman",NP,0);
  }
}