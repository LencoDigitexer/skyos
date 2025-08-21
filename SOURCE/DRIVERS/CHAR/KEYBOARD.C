/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\char\keyboard.c
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   A keyboard device driver for 102-KEY Keyboards with german layout.
/************************************************************************/
#include "devman.h"
#include "german.h"
#include "sched.h"
#include "devices.h"

extern int TASK_VFS;
extern int newgui_pid;
extern int shell1_pid;
extern int shell2_pid;
extern struct task_struct *current;

extern struct focus_struct *focus_list;
extern struct focus_struct *focus_task;
struct task_struct *focus;

volatile int keyintcount = 0;
volatile int shift = 0;
volatile int alt = 0;
volatile int ctrl = 0;
volatile int last_read_code;
volatile int last_code;
#define MAX_BUFFER 100
unsigned char keybuffer[MAX_BUFFER];

extern void startup_task();
unsigned int reboot_state = 0;

typedef struct rect
{
  unsigned int x1,y1,x2,y2;
} rect_t;
extern rect_t screen_rect;

int keyboard_pid = 0;

void put_queue(unsigned char code)
{
  unsigned char ascii;
  struct ipc_message m;

  ascii = keymap[6 * code + shift];

  /* send character to GUI */
  m.type = MSG_READ_CHAR_REPLY;
  m.MSGT_READ_CHAR_CHAR = ascii;
  m.u.lpara1 = code;
  m.u.lpara2 = shift;
  m.u.lpara3 = alt;
  m.u.lpara4 = ctrl;
  send_msg(newgui_pid, &m);

}

unsigned char kb_read_char(struct ipc_message *m)
{
   volatile unsigned char ch;

   if (m->source->last_read_code == m->source->last_code) // if no char in buffer
   {
//     printk("keyboard.c: No char in buffer. waiting...");
     return 0;                            // return 0
   }

   // else return char

   ch = m->source->KeyBuffer[m->source->last_read_code];
   m->source->last_read_code++;

   if (m->source->last_read_code == MAX_KEYS_IN_BUFFER)
     m->source->last_read_code = 0;

   m->MSGT_READ_CHAR_CHAR = ch;
   return ch;
}

void gets(char *str)
{
  unsigned char ch = 0;
  int i=0;

  while (ch!= 13)
    {
      ch = getch();
      if (ch == 13)             /* ENTER - Key */
        {
          str[i] = 0;
          printk("\n");
        }

      else if (ch == 8)              /* Back */
        {
          i--;
          str[i] = 0;
          printk("\b");
          printk(" ");
          printk("\b");
        }

      else
        {
          printk("%c",ch);
          str[i] = ch;
          i++;
        }
    }

}
void key_interrupt(void)
{
   unsigned char code, val;
   struct ipc_message m;

/*   outs(700,90,1,"Key   : %d",keyintcount);*/
   keyintcount++;
/*   outs(700,90,15,"Key   : %d",keyintcount);*/


   code = inportb(0x60);
   val = inportb(0x61);
   outportb(0x61, val | 0x80);
   outportb(0x61, val);

   if (code > 128)              /* Key released... */
     {
       if (code == SHIFT+128) shift = 0;
       if (code == RSHIFT+128) shift = 0;
       if (code == ALT+128) alt = 0;
       if (code == CTRL+128) ctrl = 0;
       return;
     }

   if (code ==  SHIFT) shift = 1;
   else if (code ==  RSHIFT) shift = 1;
   else if (code == ALT)
   {
     alt = 1;
     put_queue(code);
   }

   else if (code == CTRL) ctrl = 1;

   else if ((code == 01) && (ctrl))     // STRG + ESC
   {
//	 char str[256];
//	 struct task_struct *task = GetTask(newgui_pid);
////     ps();

//	 sprintf(str,"%d eip: 0x%x",task->state,task->tss.eip);
//	 couts(&screen_rect,10,100,str,1,7);
    quick_init_app();
   }
   else if ((code == 83) && (ctrl) && (alt))    // CTRL + ALT + DELETE
   {
     if (reboot_state == 1) reboot();
     else shutdown();
   }
   else
   {
      put_queue(code);
   }
}

void keyboard_task(void)
{
  struct ipc_message m;
  unsigned int ret;

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_READ_CHAR:
          ret = kb_read_char(&m); // convert 32bit to 8bit
          if (ret != 0)
          {
            m.type = MSG_READ_CHAR_REPLY;
            send_msg(TASK_VFS, &m);
          }
          break;
     }
  }
}

void keyboard_init(void)
{
  int res;
  int id;

  printk("keyboard.c: Initializing keyboard...\n");

  res = ((struct task_struct*)CreateKernelTask((unsigned int)keyboard_task, "keyboard", RTP,0))->pid;

  keyboard_pid = res;

  register_device(DEVICE_KEYBOARD_MAJOR, DEVICE_KEYBOARD_NAME, keyboard_pid);

  id = register_hardware(DEVICE_IDENT_INPUT,"Standard Keyboard (102-Keys)",keyboard_pid);
  register_port(id, 0x60, 0x61);
  register_irq(id, 1);

  last_read_code = last_code = 0;
}

