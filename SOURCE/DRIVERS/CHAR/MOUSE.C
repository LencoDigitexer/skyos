/****************************************************************************/
/* Sky Operating System Version 2.0j (c) 10.11.1998 by Szeleney Robert      */
/* Serial Mouse Driver (2 Buttons)                                          */
/*                                                                          */
/* Source File: c:\sky\source\drivers\char\mouse.c                          */
/* Object File: c:\sky\object\mouse.o                                       */
/*                                                                          */
/* Last Update: 21. Dezember 1998                                           */
/*                                                                          */
/****************************************************************************/
/* Main functions:                                                          */
/*           - mouse_init: Init Mouse Driver                                */
/*                                                                          */
/****************************************************************************/
/* Updated:
    280100     Fixed bug when mouse attaching while system is already running
/****************************************************************************/
#include "devman.h"
#include "sched.h"
#include "newgui.h"

/* Only to see the cursor better */
#define _ 0xff
#define O 0x0f
#define I 0x00

#define PORT_COM1  0x3f8
#define PORT_COM2  0x2f8

#define IRQ3       0xF7  /* COM2 */
#define IRQ4       0xEF  /* COM1 */

#define MOUSE_TIMEOUT 50000  /* 500000 */

extern rect_t screen_rect;

int mouse_com=0, mouse_pid, mouse_connected;
int move_count;

int mouse_x, mouse_y;
int mouse_but1, mouse_but2;
int old_x, old_y, old_but1, old_but2;
int mouse_found = 0;

int bytenum;
unsigned char combytes[3];  /* Microsoft standard */
int cursor_on;

int mouse_deb = 0, mouse_cur = 0;

unsigned char back[2000];  //back[256];
unsigned char *cursor_mask;

unsigned char cmask_pointer[256] =
{I,I,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
 I,O,I,_,_,_,_,_,_,_,_,_,_,_,_,_,
 I,O,O,I,_,_,_,_,_,_,_,_,_,_,_,_,
 I,O,O,O,I,_,_,_,_,_,_,_,_,_,_,_,
 I,O,O,O,O,I,_,_,_,_,_,_,_,_,_,_,
 I,O,O,O,O,O,I,_,_,_,_,_,_,_,_,_,
 I,O,O,O,O,O,O,I,_,_,_,_,_,_,_,_,
 I,O,O,O,O,O,O,O,I,_,_,_,_,_,_,_,
 I,O,O,O,O,O,I,O,O,I,_,_,_,_,_,_,
 I,O,O,I,O,O,I,I,I,I,_,_,_,_,_,_,
 I,I,I,I,I,O,O,I,I,_,_,_,_,_,_,_,
 _,_,_,_,I,I,O,O,I,_,_,_,_,_,_,_,
 _,_,_,_,_,I,I,I,I,_,_,_,_,_,_,_,
 _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
 _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
 _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_};

unsigned char cmask_size[256] =
{I,I,I,I,I,I,I,_,_,_,_,_,_,_,_,_,
 I,O,O,O,O,O,I,_,_,_,_,_,_,_,_,_,
 I,O,O,O,O,I,_,_,_,_,_,_,_,_,_,_,
 I,O,O,O,I,_,_,_,_,_,_,_,_,_,_,_,
 I,O,O,I,O,I,_,_,_,_,_,_,_,_,_,_,
 I,O,I,_,I,O,I,_,_,_,_,_,_,_,_,_,
 I,I,_,_,_,I,O,I,_,_,_,_,_,_,_,_,
 _,_,_,_,_,_,I,O,I,_,_,_,_,_,_,_,
 _,_,_,_,_,_,_,I,O,I,_,_,_,_,_,_,
 _,_,_,_,_,_,_,_,I,O,I,_,_,_,I,I,
 _,_,_,_,_,_,_,_,_,I,O,I,_,I,O,I,
 _,_,_,_,_,_,_,_,_,_,I,O,I,O,O,I,
 _,_,_,_,_,_,_,_,_,_,_,I,O,O,O,I,
 _,_,_,_,_,_,_,_,_,_,I,O,O,O,O,I,
 _,_,_,_,_,_,_,_,_,I,O,O,O,O,O,I,
 _,_,_,_,_,_,_,_,_,I,I,I,I,I,I,I,};


void draw_cursor();

void mouse1_interrupt(void)
{
  mouse_handling(PORT_COM1);
}

void mouse2_interrupt(void)
{
  mouse_handling(PORT_COM2);
}

void mouse_handling(int mport)
{
  struct ipc_message m;
  unsigned char inbyte;
  int dx, dy;

  inbyte = inportb(mport);

  if (!mouse_connected)
  {
     if (!mouse_detect((mport == PORT_COM1) ? 1 : 2))
     {
       mouse_connected = 1;
       mouse_reset();
     }
     return;
  }

  if (!newgui_pid)
    return;

  if (mouse_deb)
    show_msgf("mport: %d",inbyte);

  /* Make sure we are properly "synched" */
  if (inbyte & 64)
    bytenum = 0;

  /* Store the byte and adjust bytenum */
  combytes[bytenum] = inbyte;
  bytenum++;

  if (bytenum == 3)
  {
    dx = ((combytes[0] & 3) << 6) + combytes[1];
    dy = ((combytes[0] & 12) << 4) + combytes[2];
    if (dx >= 128) dx = dx - 256;
    if (dy >= 128) dy = dy - 256;
    old_x = mouse_x;
    old_y = mouse_y;
    old_but1 = mouse_but1;
    old_but2 = mouse_but2;

    mouse_x += dx;
    mouse_y += dy;
    mouse_but1 = combytes[0] & 32;
    mouse_but2 = combytes[0] & 16;
    bytenum = 0;

    if (mouse_x < screen_rect.x1) mouse_x = screen_rect.x1;
    if (mouse_y < screen_rect.y1) mouse_y = screen_rect.y1;
    if (mouse_x > screen_rect.x2) mouse_x = screen_rect.x2;
    if (mouse_y > screen_rect.y2) mouse_y = screen_rect.y2;

    /* Message handling */
    if ((old_x != mouse_x) || (old_y != mouse_y))
    {
      move_count++;
      m.type = MSG_MOUSE_MOVE;
      m.MSGT_MOUSE_X = mouse_x;
      m.MSGT_MOUSE_Y = mouse_y;
      send_msg(newgui_pid, &m);
    }

    if (old_but1 < mouse_but1)
    {
      m.type = MSG_MOUSE_BUT1_PRESSED;
      m.MSGT_MOUSE_X = mouse_x;
      m.MSGT_MOUSE_Y = mouse_y;
      send_msg(newgui_pid, &m);
    }
    else if (old_but1 > mouse_but1)
    {
      m.type = MSG_MOUSE_BUT1_RELEASED;
      m.MSGT_MOUSE_X = mouse_x;
      m.MSGT_MOUSE_Y = mouse_y;
      send_msg(newgui_pid, &m);
    }

    if (old_but2 < mouse_but2)
    {
      m.type = MSG_MOUSE_BUT2_PRESSED;
      m.MSGT_MOUSE_X = mouse_x;
      m.MSGT_MOUSE_Y = mouse_y;
      send_msg(newgui_pid, &m);
    }
    else if (old_but2 > mouse_but2)
    {
      m.type = MSG_MOUSE_BUT2_RELEASED;
      m.MSGT_MOUSE_X = mouse_x;
      m.MSGT_MOUSE_Y = mouse_y;
      send_msg(newgui_pid, &m);
    }

    if (cursor_on)
      draw_cursor();
  }

}

void save_back()
{
  int i,j;

  for (j=0;j<16;j++)
    for (i=0;i<16;i++)
      back[j*16+i] = getpix(mouse_x+i,mouse_y+j);
}

void restore_back()
{
  int i,j;

  for (j=0;j<16;j++)
    for (i=0;i<16;i++)
      setpix(old_x+i,old_y+j,back[j*16+i]);
}

void hide_cur()
{
  int flags;

  if (!cursor_on)
    return;

  save_flags(flags);
  cli();

  cursor_on = 0;
  old_x = mouse_x;
  old_y = mouse_y;
  restore_back();

  restore_flags(flags);
}

void show_cur()
{
  int flags;

  if (cursor_on)
    return;

  save_flags(flags);
  cli();

  cursor_on = 1;
  old_x = mouse_x;
  old_y = mouse_y;

  save_back();
  draw_cursor();
  restore_flags(flags);
}

void hide_cursor()
{
  if (!mouse_found) return;

  mouse_cur++;

  hide_cur();
}

void show_cursor()
{
  if (!mouse_found) return;

  mouse_cur--;

  if (mouse_cur < 1)
    show_cur();
}

void draw_cursor()
{
  int i,j;
  int sx, sy;

  if (!mouse_connected)
    return;

  sx = sy = 16;
  if ((mouse_x+15) > screen_rect.x2) sx = 16-((mouse_x+15) - screen_rect.x2);
  if ((mouse_y+15) > screen_rect.y2) sy = 16-((mouse_y+15) - screen_rect.y2);

  restore_back();
  save_back();

  for (j=0;j<sy;j++)
    for (i=0;i<sx;i++)
    {
      if (cursor_mask[j*16+i] != 0xff)
        setpix(mouse_x+i,mouse_y+j,cursor_mask[j*16+i]);
    }

}

int mouse_inrect(rect_t *rect)
{
  rect_t mrect= {mouse_x,mouse_y,mouse_x+16,mouse_y+16};

  return rect_inrect(rect,&mrect);
}

int pt_inmouse(int x, int y)
{
  rect_t mrect= {mouse_x,mouse_y,mouse_x+16,mouse_y+16};

  return pt_inrect(&mrect,x,y);
}

void mouse_debug(int value)
{
  mouse_deb = value;
}

void change_cmask(int cm)
{
  hide_cursor();

  switch (cm)
  {
    case 1: cursor_mask = cmask_pointer;
            break;
    case 2: cursor_mask = cmask_size;
            break;
  }

  show_cursor();
}

void mouse_setpos(int x, int y)
{
  hide_cursor();
  mouse_x = x;
  mouse_y = y;
  show_cursor();
}

void mouse_init()
{
  int ret=0;

  newgui_pid = 0;

  printk("mouse.c: Detecting serial mouse...");

  ret = mouse_detect(1);
  ret += mouse_detect(2);

  if (ret > 1)
  {
    printk("mouse.c: No Microsoft compatible mouse found");
    mouse_connected = 0;
  }
  else
  {
    mouse_connected = 1;
    mouse_reset();
  }
}

int mouse_detect(int port)
{
  int timeout = 0;
  int id;

  mouse_com = port;

  switch (mouse_com)
  {
    case 1: port = PORT_COM1;
            break;
    case 2: port = PORT_COM2;
            break;
  }

  outportb(port+1 , 0);					/* Turn off interrupts */
  outportb(port+3 , 0x80);

  outportb(port+3,inportb(0x03) | 0x80);	/* SET DLAB ON */
  outportb(port,0x60);
  outportb(port+1,0);
  outportb(port+3,inportb(0x03) & ~0x80);

  outportb(port+3 , 0x02);
  inportb(port);

  outportb(port+4 , 0x0b);

  while ((++timeout < MOUSE_TIMEOUT) && (inportb(port) != 'M')) ;
  if (timeout < MOUSE_TIMEOUT)
  {
    printk("mouse.c: Microsoft compatible mouse found at COM%d",mouse_com);
    mouse_found = 1;
    id = register_hardware(DEVICE_IDENT_INPUT, "Microsoft compatible mouse", 0);
    if (mouse_com == PORT_COM1)
    {
      register_irq(id, 4);
      register_port(id, 0x3f8, 0x3f8 + 4);
    }
    else
    {
      register_irq(id, 3);
      register_port(id, 0x2f8, 0x2f8 + 4);
    }
  }
  else
  {
    outportb(0x21,inportb(0x21) & ((mouse_com==1) ? IRQ4 : IRQ3));
    outportb(port+1 , 0x01);

    return 1;
  }

  outportb(0x21,inportb(0x21) & ((mouse_com==1) ? IRQ4 : IRQ3));
  outportb(port+1 , 0x01);

  return 0;
}

void mouse_reset()
{
  mouse_x = mouse_y = 200;
  old_x = old_y = 200;
  mouse_but1 = mouse_but2 = 0;
  bytenum = 0;
  cursor_on = 1;

  cursor_mask = &cmask_pointer;

  save_back();
  draw_cursor();
}
