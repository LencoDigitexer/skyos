/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\gui_wm.c
/* Last Update: 02.04.1999
/* Version    : alpha
/* Coded by   : Hayrapetian Gregory
/* Docus      :
/************************************************************************/
/* Definition:
/*   GUI Window Manager
/************************************************************************/

#include "sched.h"
#include "newgui.h"
#include "controls.h"

extern rect_t screen_rect;

extern struct task_struct *current;
extern window_t *win_console, *win_alert;

extern critical_section_t csect_mm;

extern unsigned int BPP;
unsigned int BACKGROUND = 0;

void clear_winmask(window_t *win);

void add_vrect(window_t *win, rect_t *rect)
{
  visible_rect_t *vrect = (visible_rect_t*) valloc(sizeof(visible_rect_t));

  vrect->next = NULL;
  vrect->rect = *rect;

  if (win->visible_start == NULL)
  {
    win->visible_start = vrect;
    win->visible_end = vrect;
  }
  else
  {
    win->visible_end->next = vrect;
    win->visible_end = vrect;
  }

}

void clear_vrect(window_t *win)
{
  visible_rect_t *vtmp, *vrect = win->visible_start;

  while (vrect)
  {
    vtmp = vrect->next;
    vfree(vrect);
    vrect = vtmp;
  }

  win->visible_start = NULL;
  win->visible_end = NULL;
}

window_t *create_window(char *wname, int x, int y, int length, int width, int flags)
{
  struct ipc_message m;
  window_t *win;

  m.type = MSG_GUI_CREATE_WINDOW;
  m.source = current;
  m.MSGT_GUI_X = x;
  m.MSGT_GUI_Y = y;
  m.MSGT_GUI_LENGTH = length;
  m.MSGT_GUI_WIDTH = width;
  m.MSGT_GUI_TASK = current;
  m.MSGT_GUI_WNAME = wname;
  m.MSGT_GUI_FLAGS = flags;

  send_msg(newgui_pid, &m);

  wait_msg(&m,MSG_GUI_CREATE_WINDOW);
  win = (window_t*) m.MSGT_GUI_WINDOW;

  if (~flags & WF_STYLE_DONT_SHOW)
	 skypanel_add_app(win);

  return win;
}

window_t *gui_create_window(struct task *task, char *wname, int x1, int y1, int x2, int y2, int flags)
{
  int field,i;
  window_t *win = (window_t*) valloc(sizeof(window_t));

  set_rect(&win->rect,x1,y1,x2,y2);
  win->z  = znum;
  win->id = next_wid++;
  strcpy(win->name, wname);
  field = ((x2-x1+1)*(y2-y1+1))/8;
  win->ptask = task;
  win->controls = NULL;
  win->focus = NULL;
  win->flags = flags;
  win->menu_active = 0;

  win->visible_start = NULL;
  win->visible_end = NULL;

  add_vrect(win, &win->rect);

  if (flags & WF_STYLE_TITLE)
    set_rect(&win->client_rect,x1+3,y1+17,x2-3,y2-3);
  else
    set_rect(&win->client_rect,x1+3,y1+3,x2-3,y2-3);

  win_list[znum++] = win;

  hide_cursor();
  if (znum > 1)
  {
    rect_t trect;

    set_screen_rect(&trect,win_list[znum-2]->rect.x1,win_list[znum-2]->rect.y1,win_list[znum-2]->rect.x2,win_list[znum-2]->rect.y1+15);
    draw_win_c(&trect, win_list[znum-2]);
  }
  draw_win_c(&screen_rect,win_list[znum-1]);
  show_cursor();

  mask_screen();

  if (znum > 1)
  {
    pass_msg(win_list[znum-2],MSG_GUI_REDRAW);
    pass_msg(win_list[znum-2],MSG_GUI_WINDOW_FOCUS_LOST);
  }

  pass_msg(win_list[znum-1],MSG_GUI_WINDOW_CREATED);
  pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
  pass_msg(win_list[znum-1],MSG_GUI_WINDOW_FOCUS_GET);

  return win;
}


int destroy_window(window_t *win)
{
  struct ipc_message m;

  m.type = MSG_GUI_CLOSE_WIN;
  m.MSGT_GUI_WINDOW = win;
  send_msg(newgui_pid, &m);
}

void destroy_app(window_t *win)
{
  int pid, flags;

  if (!get_wflag(win, WF_STATE_CLOSEABLE))
    return;

  exit_critical_task(win->ptask->pid);
//  reply_close(win);
////  destroy_controls(win);
  free_pmem(win->ptask->pid);


  task_control(win->ptask->pid,TASK_DEAD);
  destroy_window(win);
}

void size_window(window_t *win, int dx, int dy)
{
  int i, field;

  win->rect.x2 += dx;
  win->rect.y2 += dy;

  win->client_rect.x2 += dx;
  win->client_rect.y2 += dy;


  clear_winmask(win);
}

int valid_app(window_t *win)
{
  int i;

  for (i=0;i<znum;i++)
  {
    if (win_list[i] == win)
      return 1;
  }

  return 0;
}


int gui_destroy_window(window_t *win)
{
  int i, pid;

  if (~win->flags & WF_STYLE_DONT_SHOW)
    skypanel_del_app(win->id);
  
  for (i=0;i<MAX_WIN;i++)
  {
    if (win_list[i] == win)
    {
      do
      {
        win_list[i] = win_list[i+1];
        i++;
      } while (win_list[i] != NULL);

      hide_cursor();
      redraw_rect(win->rect.x1,win->rect.y1,win->rect.x2,win->rect.y2,2);
      show_cursor();

      pid = win->ptask->pid;

      clear_vrect(win);
      vfree(win);

      znum--;
      if (window_move)
        window_move--;

      window_ctl = NULL;

      hide_cursor();
      draw_win_c(&screen_rect, win_list[znum-1]);
      show_cursor();

      mask_screen();

      pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
      pass_msg(win_list[znum-1],MSG_GUI_WINDOW_FOCUS_GET);

//      task_control(pid,TASK_DEAD);
//      Scheduler();
      return 1;
    }
  }

  printk("window not found.");

  return 0;
}

void clear_winmask(window_t *win)
{
  clear_vrect(win);
  add_vrect(win, &win->rect);
}

int pt_visible(window_t *win, int x, int y)
{
  visible_rect_t *vrect = win->visible_start;

  while (vrect)
  {
    if (pt_inrect(vrect, x, y))
      return 1;

    vrect = vrect->next;
  }

  return 0;
}



void set_wflag(window_t *win, int flag, int value)
{
  if (value)
	win->flags |= flag;
  else
	win->flags &= ~flag;
}

int get_wflag(window_t *win, int flag)
{
  return win->flags & flag;
}

void draw_win_face(rect_t *rect, window_t *win)
{
//    cdraw_fill_rect(rect, win->rect.x1, win->rect.y1+16, win->rect.x2, win->rect.y2, COLOR_LIGHTGRAY);
    cdraw_fill_rect(rect, win->rect.x1, win->rect.y1, win->rect.x2, win->rect.y2, COLOR_LIGHTGRAY);
}

void draw_win_frame(rect_t *rect, window_t *win)
{
  cdraw_hline(rect, win->rect.x1, win->rect.x2-1, win->rect.y1, COLOR_LIGHTGRAY);
  cdraw_vline(rect, win->rect.x1, win->rect.y1,win->rect.y2-1, COLOR_LIGHTGRAY);

  cdraw_hline(rect, win->rect.x1+1, win->rect.x2-2, win->rect.y1+1, COLOR_WHITE);
  cdraw_vline(rect, win->rect.x1+1, win->rect.y1+1, win->rect.y2-2, COLOR_WHITE);

  cdraw_hline(rect, win->rect.x1, win->rect.x2, win->rect.y2, COLOR_BLACK);
  cdraw_vline(rect, win->rect.x2, win->rect.y1, win->rect.y2, COLOR_BLACK);

  cdraw_hline(rect, win->rect.x1+1, win->rect.x2-1, win->rect.y2-1, COLOR_GRAY);
  cdraw_vline(rect, win->rect.x2-1, win->rect.y1+1, win->rect.y2-1, COLOR_GRAY);

  cdraw_hline(rect, win->rect.x1+2, win->rect.x2-2, win->rect.y1+2, COLOR_LIGHTGRAY);
}

void draw_win_title(rect_t *rect, window_t *win)
{
  rect_t trect = {win->rect.x1, win->rect.y1, win->rect.x2-15, win->rect.y1+25};
  int tcolor;
  char str[256];

  cdraw_vline(rect, win->rect.x1+2, win->rect.y1+2, win->rect.y1+25, COLOR_LIGHTGRAY);
  cdraw_vline(rect, win->rect.x2-2, win->rect.y1+2, win->rect.y1+25, COLOR_LIGHTGRAY);

  if (win == win_alert)
    tcolor = COLOR_RED;
  else
    tcolor = (win == win_list[znum-1]) ? COLOR_BLUE : COLOR_GRAY;

  cdraw_fill_rect(rect, win->rect.x1+3, win->rect.y1+3, win->rect.x2-3, win->rect.y1+15, tcolor);

  if (win->flags & WF_STOPPED)
	sprintf(str,"%s - PAUSED",win->name);
  else
	sprintf(str,"%s",win->name);

  if (!fit_rect(&trect,rect))
    couts(&trect, win->rect.x1+6,win->rect.y1+6,str,COLOR_WHITE,255);

}

void draw_win_butclose(rect_t *rect, window_t *win)
{
  cdraw_hline(rect, win->rect.x2-15, win->rect.x2-7, win->rect.y1+5, COLOR_WHITE);
  cdraw_vline(rect, win->rect.x2-15, win->rect.y1+5, win->rect.y1+12, COLOR_WHITE);

  cdraw_hline(rect, win->rect.x2-15, win->rect.x2-6, win->rect.y1+13, COLOR_BLACK);
  cdraw_vline(rect, win->rect.x2-6, win->rect.y1+5, win->rect.y1+13, COLOR_BLACK);
  cdraw_fill_rect(rect, win->rect.x2-14, win->rect.y1+6, win->rect.x2-7, win->rect.y1+12, COLOR_LIGHTGRAY);

  couts(rect, win->rect.x2-14,win->rect.y1+5,"x",COLOR_BLACK,255);


  cdraw_hline(rect, win->rect.x2-26, win->rect.x2-18, win->rect.y1+5, COLOR_WHITE);
  cdraw_vline(rect, win->rect.x2-26, win->rect.y1+5, win->rect.y1+12, COLOR_WHITE);

  cdraw_hline(rect, win->rect.x2-26, win->rect.x2-17, win->rect.y1+13, COLOR_BLACK);
  cdraw_vline(rect, win->rect.x2-17, win->rect.y1+5, win->rect.y1+13, COLOR_BLACK);
  cdraw_fill_rect(rect, win->rect.x2-25, win->rect.y1+6, win->rect.x2-18, win->rect.y1+12, COLOR_LIGHTGRAY);

  couts(rect, win->rect.x2-25,win->rect.y1+5,"_",COLOR_BLACK,255);
}

void draw_winctl(rect_t *rect, window_t *win)
{
  control_t *ctl = win->controls;

  while (ctl)
  {
    if (win->focus == ctl)
      draw_control(rect, win, ctl, 1);
    else
      draw_control(rect, win, ctl, 0);

    ctl = ctl->next;
  }
}

void draw_win_c(rect_t *rect, window_t *win)
{
  visible_rect_t *vrect = win->visible_start;

/*
#define WF_STYLE_FACE			0x00000100
#define WF_STYLE_FRAME			0x00000200
#define WF_STYLE_TITLE			0x00000400
#define WF_SYTLE_BUTCLOSE               0x00000800

  switch (win->flags & 0x00000F00)
  {
    case WF_STYLE_FACE: draw_win_face(rect,win);
    draw_win_frame(rect,win);
    draw_win_title(rect,win);
    draw_win_butclose(rect,win);
    draw_winctl(rect,win);
  }
*/

  hide_cursor();
  if (win->flags & WF_STYLE_FACE) draw_win_face(rect,win);
  if (win->flags & WF_STYLE_FRAME) draw_win_frame(rect,win);
  if (win->flags & WF_STYLE_TITLE) draw_win_title(rect,win);
  if (win->flags & WF_STYLE_BUTCLOSE) draw_win_butclose(rect,win);
/*
  while (vrect)
  {
    cdraw_rect(rect,vrect->rect.x1,vrect->rect.y1,vrect->rect.x2,vrect->rect.y2,COLOR_LIGHTRED);
    vrect = vrect->next;
  }
*/

  draw_winctl(rect,win);
  show_cursor();

}


void draw_background(int x1, int y1, int x2, int y2, int style)
{
  int i,j;
  rect_t rect={x1,y1,x2,y2};

  BACKGROUND = style;

  if (fit_rect(&rect,&screen_rect))
    return;

  if (style == 0)
  {
    if (BPP==8)
      for (j=rect.y1;j<=rect.y2;j++)
        fdraw_hline(x1,x2,j,32+j/(screen_rect.y2/63 + 1));

    else
      for (j=rect.y1;j<=rect.y2;j++)
        fdraw_hline(x1,x2,j,COLOR_BLUE);
  }
  else
  {
    for (i=rect.x1;i<=rect.x2;i+=style)
      fdraw_vline(i, rect.y1, rect.y2, 2);
    for (j=rect.y1;j<=rect.y2;j+=style)
      fdraw_hline(x1, x2, j, 2);
   }

}

void redraw_window(window_t *win)
{
  struct ipc_message m;

  m.type = MSG_GUI_REDRAW_WINDOW;
  m.MSGT_GUI_WINADR = win;
  send_msg(newgui_pid, &m);
}

void mask_field(int x1, int y1, int x2, int y2, int wpar)
{
  int i;
  rect_t rect, wrect;

  set_screen_rect(&rect,x1,y1,x2,y2);
  set_screen_rect(&wrect,x1,y1,x2,y2);

  for (i=znum-wpar;i>=0;i--)
  {
    if (rect_inrect(&win_list[i]->rect, &wrect))
    {
      fit_rect(&wrect, &win_list[i]->rect);

      add_vrect(win_list[i], &wrect);

      if (rect.y1 < wrect.y1)
        mask_rect(rect.x1,rect.y1,rect.x2,wrect.y1-1,wpar+1);

      if (rect.y2 > wrect.y2)
        mask_rect(rect.x1,wrect.y2+1,rect.x2,rect.y2,wpar+1);

      if (rect.x1 < wrect.x1)
        mask_rect(rect.x1,wrect.y1,wrect.x1-1,wrect.y2,wpar+1);

      if (rect.x2 > wrect.x2)
        mask_rect(wrect.x2+1,wrect.y1,rect.x2,wrect.y2,wpar+1);

      return;
    }
  }

}

void mask_rect(int x1, int y1, int x2, int y2, int wpar)
{
  int i;
  rect_t rect, wrect;

  set_screen_rect(&rect,x1,y1,x2,y2);
  set_screen_rect(&wrect,x1,y1,x2,y2);

  for (i=znum-wpar;i>=0;i--)
  {
    if (rect_inrect(&win_list[i]->rect, &wrect))
    {
      fit_rect(&wrect, &win_list[i]->rect);

      add_vrect(win_list[i], &wrect);

      if (rect.y1 < wrect.y1)
        mask_rect(rect.x1,rect.y1,rect.x2,wrect.y1-1,wpar+1);

      if (rect.y2 > wrect.y2)
        mask_rect(rect.x1,wrect.y2+1,rect.x2,rect.y2,wpar+1);

      if (rect.x1 < wrect.x1)
        mask_rect(rect.x1,wrect.y1,wrect.x1-1,wrect.y2,wpar+1);

      if (rect.x2 > wrect.x2)
        mask_rect(wrect.x2+1,wrect.y1,rect.x2,wrect.y2,wpar+1);

      return;
    }
  }

}

void mask_screen()
{
  int i;

  for (i=0;i<znum;i++)
    clear_vrect(win_list[i]);

  mask_rect(0,0,screen_rect.x2,screen_rect.y2,1);
}

void invalidate_rect(window_t *win, rect_t *rect)
{
  struct ipc_message *m;
  rect_t *crect = (rect_t*) valloc(sizeof(rect_t));

  crect->x1 = rect->x1;
  crect->y1 = rect->y1;
  crect->x2 = rect->x2;
  crect->y2 = rect->y2;

  m = (struct ipc_message*)valloc(sizeof(struct ipc_message));
  m->type = MSG_GUI_REDRAW_EX;
  m->MSGT_GUI_RECT = crect;
  send_msg(win->ptask->pid, m);
  vfree(m);
}

void redraw_rect(int x1, int y1, int x2, int y2, int wpar)
{
  int i;
  rect_t rect, wrect;

  set_screen_rect(&rect,x1,y1,x2,y2);
  set_screen_rect(&wrect,x1,y1,x2,y2);

  for (i=znum-wpar;i>=0;i--)
  {
    if (rect_inrect(&win_list[i]->rect, &wrect))
    {
      fit_rect(&wrect, &win_list[i]->rect);

      draw_win_c(&wrect, win_list[i]);
      pass_msg(win_list[i],MSG_GUI_REDRAW);
//      invalidate_rect(win_list[i],&wrect);

      if (rect.y1 < wrect.y1)
        redraw_rect(rect.x1,rect.y1,rect.x2,wrect.y1-1,wpar+1);

      if (rect.y2 > wrect.y2)
        redraw_rect(rect.x1,wrect.y2+1,rect.x2,rect.y2,wpar+1);

      if (rect.x1 < wrect.x1)
        redraw_rect(rect.x1,wrect.y1,wrect.x1-1,wrect.y2,wpar+1);

      if (rect.x2 > wrect.x2)
        redraw_rect(wrect.x2+1,wrect.y1,rect.x2,wrect.y2,wpar+1);

      return;
    }
  }

  draw_background(wrect.x1,wrect.y1,wrect.x2,wrect.y2, BACKGROUND);
}


void redraw_rect_win(int x1, int y1, int x2, int y2, int wnr)
{
  int i;
  rect_t rect, rect2;

  set_screen_rect(&rect,x1,y1,x2,y2);

  for (i=znum-2;i>=wnr;i--)
  {
    rect2 = win_list[i]->rect;

    if (rect_inrect(&rect2, &rect))
    {
      fit_rect(&rect2,&rect);
      draw_win_c(&rect2,win_list[znum-1]);
    }
  }

}

void move_window_full(int dx, int dy)
{
  int i, x1, x2, y1, y2;
  rect_t rect;

  x1 = win_list[window_move-1]->rect.x1;
  x2 = win_list[window_move-1]->rect.x2;
  y1 = win_list[window_move-1]->rect.y1;
  y2 = win_list[window_move-1]->rect.y2;

  hide_cursor();

  copy_screen_rect(win_list[window_move-1]->rect,dx,dy);

  enlarge_rect(&win_list[window_move-1]->rect,dx,dy,dx,dy);
  enlarge_rect(&win_list[window_move-1]->client_rect,dx,dy,dx,dy);

//  draw_win_c(&screen_rect, win_list[window_move-1]);

  if (dy > 0)
  {
    if (y1 <= 0)
    {
      set_screen_rect(&rect,x1+dx,0,x2+dx,dy);
      draw_win_c(&rect, win_list[window_move-1]);
      pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
    }

    redraw_rect(x1,y1,x2,y1+dy-1,2);
  }
  else if (dy != 0)
  {
    if (y2 >= screen_rect.y2)
    {
      set_screen_rect(&rect,x1+dx,screen_rect.y2+dy,x2+dx,screen_rect.y2);
      draw_win_c(&rect, win_list[window_move-1]);
      pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
    }

    redraw_rect(x1,y2+dy+1,x2,y2,2);
  }

  if (dx > 0)
  {
    redraw_rect(x1,y1,x1+dx-1,y2,2);
    if (x1 <= 0)
    {
      set_screen_rect(&rect,0,y1+dy,dx,y2+dy);
      draw_win_c(&rect, win_list[window_move-1]);
      pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
    }

  }
  else if (dx != 0)
  {
    redraw_rect(x2+dx+1,y1,x2,y2,2);
    if (x2 >= screen_rect.x2)
    {
      set_screen_rect(&rect,screen_rect.x2+dx,y1+dy,screen_rect.x2,y2+dy);
      draw_win_c(&rect, win_list[window_move-1]);
      pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
    }

  }

//  draw_win_c(&screen_rect, win_list[window_move-1]);
//  pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
  show_cursor();

//  set_wflag(win_list[znum-1], WF_STOP_OUTPUT, 1);

  mask_screen();


//  pass_msg(win_list[znum-1],MSG_GUI_WINDOW_MOVED);
}

void move_window_rect(int dx, int dy)
{
  hide_cursor();

  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y1,COLOR_GREEN);
  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y1+1,COLOR_GREEN);
  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y1+2,COLOR_GREEN);

  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y2,COLOR_GREEN);
  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y2-1,COLOR_GREEN);
  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y2-2,COLOR_GREEN);

  cdraw_vline_inv(&screen_rect,move_rect.x1,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);
  cdraw_vline_inv(&screen_rect,move_rect.x1+1,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);
  cdraw_vline_inv(&screen_rect,move_rect.x1+2,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);

  cdraw_vline_inv(&screen_rect,move_rect.x2,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);
  cdraw_vline_inv(&screen_rect,move_rect.x2-1,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);
  cdraw_vline_inv(&screen_rect,move_rect.x2-2,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);

  enlarge_rect(&move_rect,dx,dy,dx,dy);

  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y1,COLOR_GREEN);
  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y1+1,COLOR_GREEN);
  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y1+2,COLOR_GREEN);

  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y2,COLOR_GREEN);
  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y2-1,COLOR_GREEN);
  cdraw_hline_inv(&screen_rect,move_rect.x1,move_rect.x2,move_rect.y2-2,COLOR_GREEN);

  cdraw_vline_inv(&screen_rect,move_rect.x1,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);
  cdraw_vline_inv(&screen_rect,move_rect.x1+1,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);
  cdraw_vline_inv(&screen_rect,move_rect.x1+2,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);

  cdraw_vline_inv(&screen_rect,move_rect.x2,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);
  cdraw_vline_inv(&screen_rect,move_rect.x2-1,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);
  cdraw_vline_inv(&screen_rect,move_rect.x2-2,move_rect.y1+3,move_rect.y2-3,COLOR_GREEN);

  show_cursor();
}

void size_win(int dx, int dy, int mx, int my)
{
  int i, x1, x2, y1, y2;
  rect_t rect;

  x1 = win_list[znum-1]->rect.x1;
  x2 = win_list[znum-1]->rect.x2;
  y1 = win_list[znum-1]->rect.y1;
  y2 = win_list[znum-1]->rect.y2;

  dx = mx - x2;
  dy = my - y2;

  if ((x2-x1+dx) <= 100) dx = -((x2-x1) - 100);
  if ((y2-y1+dy) <= 100) dy = -((y2-y1) - 100);

  if ((dx == 0) && (dy == 0))
    return;

  size_window(win_list[znum-1],dx,dy);

  hide_cursor();
  if (dx < 0)
  {
    redraw_rect(x2+dx+1,y1,x2,y2+abs(dy),2);
    set_screen_rect(&rect,x2+dx-3,y1,x2+dx,y2);
    draw_win_c(&rect, win_list[znum-1]);

    set_screen_rect(&rect,x2+dx-15,y1,x2+dx,y1+15);
    draw_win_c(&rect, win_list[znum-1]);
  }
  else if (dx != 0)
  {
//    mask_win(x2,y1,x2+dx,y2,2);
    set_screen_rect(&rect,x2-3,y1,x2+dx,y2);
    draw_win_c(&rect, win_list[znum-1]);

    set_screen_rect(&rect,x2-15,y1,x2+dx,y1+15);
    draw_win_c(&rect, win_list[znum-1]);
  }

  if (dy < 0)
  {
    redraw_rect(x1,y2+dy+1,x2+abs(dx),y2,2);
    set_screen_rect(&rect,x1,y2+dy-3,x2,y2+dy);
    draw_win_c(&rect, win_list[znum-1]);
  }
  else if (dy != 0)
  {
//    mask_win(x1,y2+1,x2+dx,y2+dy,2);
    set_screen_rect(&rect,x1,y2-3,x2+abs(dx),y2+dy);
    draw_win_c(&rect, win_list[znum-1]);
  }

  show_cursor();

  mask_screen();

//  set_wflag(win_list[znum-1], WF_STOP_OUTPUT, 1);

  pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
  pass_msg(win_list[znum-1],MSG_GUI_WINDOW_SIZED);
}

