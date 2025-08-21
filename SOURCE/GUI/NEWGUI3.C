/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\newgui.c
/* Last Update: 06.01.1999
/* Version    : alpha
/* Coded by   : Hayrapetian Gregory
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the Graphical User Interface
/************************************************************************/

#include <stdarg.h>
#include "sched.h"
#include "newgui.h"

window_t *create_window(int x1, int y1, int x2, int y2, char *name)
{
  int i, field;
  window_t *win = (window_t*) valloc(sizeof(window_t));

  win->rect.x1 = x1;
  win->rect.y1 = y1;
  win->rect.x2 = x2;
  win->rect.y2 = y2;
  win->z  = znum;
  strcpy(win->name, name);
  field = ((x2-x1+1)*(y2-y1+1))/8;
  win->bitmask = (char*) valloc(field);

  for (i=0;i<field;i++)
    win->bitmask[i] = 0xff;

  win->ptask = NULL;
  win->flags = 0;

  win_list[znum++] = win;

  hide_cursor();

  if (znum > 1)
    draw_win_c(&screen_rect,win_list[znum-2]);
  draw_win_c(&screen_rect,win_list[znum-1]);

  show_cursor();

  return win;
}

int destroy_window(window_t *win)
{
  int i;
 
  for (i=0;i<MAX_WIN;i++)
  {
    if (win_list[i] == win)
    {
      hide_cursor();
      redraw_rect(win_list[i]->rect.x1,win_list[i]->rect.y1,win_list[i]->rect.x2,win_list[i]->rect.y2);
      show_cursor();

      vfree(win_list[i]->bitmask);
      vfree(win_list[i]);

      do
      {
        win_list[i] = win_list[i+1];
        i++;
      } while (win_list[i] != NULL);
      znum--;

      hide_cursor();
      draw_win_c(&screen_rect, win_list[znum-1]);
      show_cursor();

      pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
      return 1;
    }
  }

  return 0;
}

void size_window(window_t *win, int dx, int dy)
{
  int i, field;

  win->rect.x2 += dx;
  win->rect.y2 += dy;
  field = ((win->rect.x2-win->rect.x1)*(win->rect.y2-win->rect.y1))/8 + (win->rect.x2-win->rect.x1);       /* save area */
  vfree(win->bitmask);
  win->bitmask = (char*) valloc(field);

  for (i=0;i<field;i++)
    win->bitmask[i] = 0xff;
}

inline int win_area(window_t *win)
{
  return (win->rect.x2-win->rect.x1+1)*(win->rect.y2-win->rect.y1+1);
}

inline int win_offset(window_t *win, int x, int y)
{
  return (y-win->rect.y1)*(win->rect.x2-win->rect.x1)+(x-win->rect.x1);
}

void set_bitmask(window_t *win, unsigned int nr, char bit)
{

 if (nr > win_area(win))
 {
   printk("newgui.c: set_bitmask - number: %d",nr);
   return 0;
 }

 if (get_bitmask(win,nr) != bit)
     win->bitmask[nr >> 3] ^= (1 << (nr % 8));
}

int get_bitmask(window_t *win, unsigned int nr)
{

 if (nr > win_area(win))
 {
   printk("newgui.c: get_bitmask - number: %d",nr);
   return 0;
 }

 return (win->bitmask[nr>>3] >> (nr % 8)) & 1;
}

int fit_rect(rect_t *fitrect, rect_t *source)
{
  if ((fitrect->x1 > source->x2) || (fitrect->y1 > source->y2) ||
      (fitrect->x2 < source->x1) || (fitrect->y2 < source->y1))
    return 1;

  if (fitrect->x1 < source->x1) fitrect->x1 = source->x1;
  if (fitrect->y1 < source->y1) fitrect->y1 = source->y1;
  if (fitrect->x2 > source->x2) fitrect->x2 = source->x2;
  if (fitrect->y2 > source->y2) fitrect->y2 = source->y2;

  return 0;
}

void set_bitmask_rect(window_t *win, rect_t *rect, char bit)
{
  int i,j;

  if (fit_rect(rect,&win->rect))
    return;

  for (j=rect->y1;j<rect->y2;j++)
    for (i=rect->x1;i<rect->x2;i++)
      set_bitmask(win,win_offset(win,i,j),bit);
}

int rect_displayed(rect_t *rect, window_t *win)
{
  int i,j;

  if (fit_rect(rect,&win->rect))
    return;

  for (j=rect->y1;j<rect->y2;j++)
    for (i=rect->x1;i<rect->x2;i++)
      if (get_bitmask(win,win_offset(win,i,j)))
        return 1;

  return 0;
}

void set_rect(rect_t *rect, int x1, int y1, int x2, int y2)
{
  if (x1 < screen_rect.x1) x1 = screen_rect.x1;
  if (y1 < screen_rect.y1) y1 = screen_rect.y1;
  if (x2 < screen_rect.x1) x2 = screen_rect.x1;
  if (y2 < screen_rect.y1) y2 = screen_rect.y1;

  if (x1 > screen_rect.x2) x1 = screen_rect.x2;
  if (y1 > screen_rect.y2) y1 = screen_rect.y2;
  if (x2 > screen_rect.x2) x2 = screen_rect.x2;
  if (y2 > screen_rect.y2) y2 = screen_rect.y2;

  rect->x1 = x1;
  rect->y1 = y1;
  rect->x2 = x2;
  rect->y2 = y2;
}

void draw_win_cw(rect_t *rect, window_t *win)
{
  wcdraw_hline(win,rect, win->rect.x1, win->rect.x2-1, win->rect.y1, COLOR_LIGHTGRAY);
  wcdraw_vline(win,rect, win->rect.x1, win->rect.y1  , win->rect.y2-1, COLOR_LIGHTGRAY);

  wcdraw_hline(win,rect, win->rect.x1+1, win->rect.x2-2, win->rect.y1+1, COLOR_WHITE);
  wcdraw_vline(win,rect, win->rect.x1+1, win->rect.y1+1, win->rect.y2-2, COLOR_WHITE);

  wcdraw_hline(win,rect, win->rect.x1, win->rect.x2  , win->rect.y2, COLOR_BLACK);
  wcdraw_vline(win,rect, win->rect.x2, win->rect.y1  , win->rect.y2, COLOR_BLACK);

  wcdraw_hline(win,rect, win->rect.x1+1, win->rect.x2-1, win->rect.y2-1, COLOR_GRAY);
  wcdraw_vline(win,rect, win->rect.x2-1, win->rect.y1+1, win->rect.y2-1, COLOR_GRAY);

  wcdraw_hline(win,rect, win->rect.x1+2, win->rect.x2-2, win->rect.y1+2, COLOR_LIGHTGRAY);
  wcdraw_vline(win,rect, win->rect.x1+2, win->rect.y1+2, win->rect.y1+25, COLOR_LIGHTGRAY);
  wcdraw_vline(win,rect, win->rect.x2-2, win->rect.y1+2, win->rect.y1+25, COLOR_LIGHTGRAY);

  wcdraw_fill_rect(win,rect, win->rect.x1+3, win->rect.y1+3, win->rect.x2-3, win->rect.y1+15, (win->z == znum-1) ? COLOR_BLUE : COLOR_GRAY);
  wcdraw_fill_rect(win,rect, win->rect.x1+2, win->rect.y1+16, win->rect.x2-2, win->rect.y2-2, COLOR_LIGHTGRAY);

  wcdraw_hline(win,rect, win->rect.x2-15, win->rect.x2-7, win->rect.y1+5, COLOR_WHITE);
  wcdraw_vline(win,rect, win->rect.x2-15, win->rect.y1+5, win->rect.y1+12, COLOR_WHITE);

  wcdraw_hline(win,rect, win->rect.x2-15, win->rect.x2-6, win->rect.y1+13, COLOR_BLACK);
  wcdraw_vline(win,rect, win->rect.x2-6, win->rect.y1+5, win->rect.y1+13, COLOR_BLACK);
  wcdraw_fill_rect(win,rect, win->rect.x2-14, win->rect.y1+6, win->rect.x2-7, win->rect.y1+12, COLOR_LIGHTGRAY);

//  outs_cc(rect, win->rect.x1+6,win->rect.y1+6,WHITE,win->name);
}

void draw_win_c(rect_t *rect, window_t *win)
{
  cdraw_hline(rect, win->rect.x1, win->rect.x2-1, win->rect.y1, COLOR_LIGHTGRAY);
  cdraw_vline(rect, win->rect.x1, win->rect.y1  , win->rect.y2-1, COLOR_LIGHTGRAY);

  cdraw_hline(rect, win->rect.x1+1, win->rect.x2-2, win->rect.y1+1, COLOR_WHITE);
  cdraw_vline(rect, win->rect.x1+1, win->rect.y1+1, win->rect.y2-2, COLOR_WHITE);

  cdraw_hline(rect, win->rect.x1, win->rect.x2  , win->rect.y2, COLOR_BLACK);
  cdraw_vline(rect, win->rect.x2, win->rect.y1  , win->rect.y2, COLOR_BLACK);

  cdraw_hline(rect, win->rect.x1+1, win->rect.x2-1, win->rect.y2-1, COLOR_GRAY);
  cdraw_vline(rect, win->rect.x2-1, win->rect.y1+1, win->rect.y2-1, COLOR_GRAY);

  cdraw_hline(rect, win->rect.x1+2, win->rect.x2-2, win->rect.y1+2, COLOR_LIGHTGRAY);
  cdraw_vline(rect, win->rect.x1+2, win->rect.y1+2, win->rect.y1+25, COLOR_LIGHTGRAY);
  cdraw_vline(rect, win->rect.x2-2, win->rect.y1+2, win->rect.y1+25, COLOR_LIGHTGRAY);

  cdraw_fill_rect(rect, win->rect.x1+3, win->rect.y1+3, win->rect.x2-3, win->rect.y1+15, (win->z == znum-1) ? COLOR_BLUE : COLOR_GRAY);
  cdraw_fill_rect(rect, win->rect.x1+2, win->rect.y1+16, win->rect.x2-2, win->rect.y2-2, COLOR_LIGHTGRAY);

  cdraw_hline(rect, win->rect.x2-15, win->rect.x2-7, win->rect.y1+5, COLOR_WHITE);
  cdraw_vline(rect, win->rect.x2-15, win->rect.y1+5, win->rect.y1+12, COLOR_WHITE);

  cdraw_hline(rect, win->rect.x2-15, win->rect.x2-6, win->rect.y1+13, COLOR_BLACK);
  cdraw_vline(rect, win->rect.x2-6, win->rect.y1+5, win->rect.y1+13, COLOR_BLACK);
  cdraw_fill_rect(rect, win->rect.x2-14, win->rect.y1+6, win->rect.x2-7, win->rect.y1+12, COLOR_LIGHTGRAY);

  couts(rect, win->rect.x1+6,win->rect.y1+6,COLOR_WHITE,win->name);
}

void draw_background(int x1, int y1, int x2, int y2)
{
  int i,j;
  rect_t rect={x1,y1,x2,y2};

  if (fit_rect(&rect,&screen_rect))
    return;

  for (j=rect.y1;j<=rect.y2;j++)
    for (i=rect.x1;i<=rect.x2;i++)
      put_pixel(i,j,32+j/12);
}

void redraw_all()
{
  int i;

  hide_cursor();
  draw_background(screen_rect.x1,screen_rect.y1,screen_rect.x2,screen_rect.y2);
  for (i=0;i<MAX_WIN;i++)
  {
    if (win_list[i] != NULL)
    {
      draw_win_c(&screen_rect,win_list[i]);
      pass_msg(win_list[i],MSG_GUI_REDRAW);
    }
  }
  show_cursor();
}

int check_title(int x, int y)
{
  int i;
  rect_t rect;

  for (i=znum;i>=0;i--)
    if (win_list[i] != NULL)
    {
      rect.x1 = win_list[i]->rect.x1+3;
      rect.y1 = win_list[i]->rect.y1+3;
      rect.x2 = win_list[i]->rect.x2-13;
      rect.y2 = win_list[i]->rect.y1+15;
      if (pt_inrect(&rect,x,y))
      {
        window_move = i+1;
        return 1;
      }
    }

  return 0;
}

int check_titlebut(int x, int y)
{
  int i;
  rect_t rect;

  for (i=znum;i>=0;i--)
    if (win_list[i] != NULL)
    {
      rect.x1 = win_list[i]->rect.x2-13;
      rect.y1 = win_list[i]->rect.y1+3;
      rect.x2 = win_list[i]->rect.x2-3;
      rect.y2 = win_list[i]->rect.y1+15;
      if (pt_inrect(&rect,x,y))
      {
        window_move = i+1;
        return 1;
      }
    }

  return 0;
}

int check_win(int x, int y)
{
  int i;

  for (i=znum-1;i>=0;i--)
    if (win_list[i] != NULL)
    {
      if (pt_inrect(&win_list[i]->rect,x,y))
        return i+1;
    }

  return 0;
}

int pt_inrect(rect_t *rect, int x, int y)
{
  if ((x < rect->x1) || (x > rect->x2) || (y < rect->y1) || (y > rect->y2))
    return 0;
  else
    return 1;
}

int rect_inrect(rect_t *b1, rect_t *b2)
{
  if ((b1->x2 < b2->x1) || (b1->x1 > b2->x2) || (b1->y2 < b2->y1) || (b1->y1 > b2->y2))
    return 0;
  else
    return 1;
}
/*
void redraw_rect(int x1, int y1, int x2, int y2)
{
  int i,j;
  rect_t rect, wrect;

  set_rect(&rect,x1,y1,x2,y2);

  for (i=0;i<znum-1;i++)
  {
    set_rect(&wrect,win_list[i]->rect.x1,win_list[i]->rect.y1,win_list[i]->rect.x2,win_list[i]->rect.y2);
    if (rect_inrect(&wrect,&rect))
    {
      draw_win_c(&rect,win_list[i]);
      pass_msg(win_list[i],MSG_GUI_REDRAW);
    }
  }

}
*/
int shift_z(int wnr)
{
  int i;
  window_t *tmp = win_list[wnr];

  if (win_list[wnr+1] == NULL)
    return wnr+1;

  for (i=wnr;i<MAX_WIN-1;i++)
  {
    win_list[i] = win_list[i+1];
    if (win_list[i] == NULL)
      break;
    win_list[i]->z--;
  }
  tmp->z = znum-1;
  win_list[i] = tmp;

  return i+1;
}

void pass_msg(window_t *win,int msg_type)
{
  struct ipc_message *m;
  if (win->ptask == NULL)
    return;

  if (msg_type == MSG_GUI_REDRAW)
  {
    if (win->flags & WF_MSG_GUI_REDRAW)
      return;
    win->flags |= WF_MSG_GUI_REDRAW;
  }

  m = (struct ipc_message*)valloc(sizeof(struct ipc_message));
  m->type = msg_type;
  send_msg(win->ptask->pid, m);
  vfree(m);
}

void spass_msg(window_t *win, struct ipc_message *m)
{
  if (win->ptask == NULL)
    return;

  send_msg(win->ptask->pid, m);
}

void clear_winmask(window_t *win)
{
  int i, field = ((win->rect.x2-win->rect.x1)*(win->rect.y2-win->rect.y1))/(8*4);
  unsigned int *bitmask = win->bitmask;

  for (i=0;i<field;i++)
   bitmask[i] = 0xffffffff;
}

void mask_win(int x1, int y1, int x2, int y2)
{
  int i,j,nr,zn,k;
  rect_t rect = {x1,y1,x2,y2};
  window_t *wlist[MAX_WIN];

  fit_rect(&rect,&screen_rect);

  j = 0;
  for (i=znum-2;i>=0;i--)
  {
    if (rect_inrect(&win_list[i]->rect, &rect))
      wlist[j++] = win_list[i];
  }
  zn = j;

  for (j=rect.y1;j<=rect.y2;j++)
    for (i=rect.x1;i<=rect.x2;i++)
    {
      for (k=0;k<zn;k++)
      {
        if (pt_inrect(&wlist[k]->rect,i,j))
        {
          set_bitmask(wlist[k],win_offset(wlist[k],i,j),0);
          break;
        }
      }
    }

}

int pt_inwin(int x, int y)
{
  int i;
  rect_t rect;

  for (i=znum-2;i>=0;i--)
  {
//     set_rect(&rect,win_list[i]->rect.x1,win_list[i]->rect.y1,win_list[i]->rect.x2-1,win_list[i]->rect.y2-1);
     if (pt_inrect(&win_list[i]->rect,x,y))
       return i;
  }

  return -1;
}

void pass_active_win()
{
  int wnr;

  wnr = 1;
  wnr = shift_z(wnr-1);
  hide_cursor();
  if (znum > 1)
  {
    draw_win_c(&screen_rect, win_list[znum-2]);
    pass_msg(win_list[znum-2],MSG_GUI_REDRAW);
  }
  draw_win_c(&screen_rect, win_list[wnr-1]);
  show_cursor();

  clear_winmask(win_list[wnr-1]);
  mask_win(win_list[wnr-1]->rect.x1,win_list[wnr-1]->rect.y1,win_list[wnr-1]->rect.x2,win_list[wnr-1]->rect.y2);

  pass_msg(win_list[wnr-1],MSG_GUI_REDRAW);
}

void set_wflag_stopout(window_t *win, int value)
{
  if (value)
	win->flags |= WF_STOP_OUTPUT;
  else
	win->flags &= ~WF_STOP_OUTPUT;
}

void set_wflag_guiredraw(window_t *win, int value)
{
  if (value)
	win->flags |= WF_MSG_GUI_REDRAW;
  else
	win->flags &= ~WF_MSG_GUI_REDRAW;
}

int get_wflag_stopout(window_t *win)
{
  return win->flags & WF_STOP_OUTPUT;
}

int get_wflag_guiredraw(window_t *win)
{
  return win->flags & WF_MSG_GUI_REDRAW;
}

void redraw_rect(int x1, int y1, int x2, int y2)
{
  int i,j,k,nr=-2,zn,onr=-2,oi;
  rect_t rect, wrect, rect2;
  window_t *wlist[MAX_WIN];

  set_rect(&rect,x1,y1,x2,y2);

  j = 0;
  for (i=znum-2;i>=0;i--)
  {
    if (rect_inrect(&win_list[i]->rect, &rect))
    {
      wlist[j++] = win_list[i];
      pass_msg(win_list[i],MSG_GUI_REDRAW);
    }
  }
  zn = j;

  for (j=rect.y1;j<=rect.y2;j++)
  {
    onr = -2;
    oi = rect.x1;

    wrect.y1 = j;
    wrect.y2 = j;

    for (i=rect.x1;i<=rect.x2;i++)
    {
      nr = -1;
      for (k=0;k<zn;k++)
      {
        if (pt_inrect(&wlist[k]->rect,i,j))
        {
          set_bitmask(wlist[k],win_offset(wlist[k],i,j),1);
          nr = k;
//          if (rect.x1 == rect.x2)
//            onr = nr;
          break;
        }
      }

      if ((onr != nr) || (i == rect.x2))
      {

        if (onr >= 0)
        {
          wrect.x1 = oi;
          wrect.x2 = i;
          draw_win_c(&wrect,wlist[onr]);
        }
        else
         for (k=oi;k<=i;k++)
           put_pixel(k,j,32+j/12);

        if (i == rect.x2)
        {
          if (nr >= 0)
          {
            wrect.x1 = i;
            wrect.x2 = i;
            draw_win_c(&wrect,wlist[nr]);
          }
          else
            put_pixel(i,j,32+j/12);
        }

        oi = i;
        onr = nr;
      }
    }
  }
}

void move_window(int dx, int dy)
{
  int i, x1, x2, y1, y2;

  x1 = win_list[window_move-1]->rect.x1;
  x2 = win_list[window_move-1]->rect.x2;
  y1 = win_list[window_move-1]->rect.y1;
  y2 = win_list[window_move-1]->rect.y2;

  win_list[window_move-1]->rect.x1 += dx;
  win_list[window_move-1]->rect.x2 += dx;
  win_list[window_move-1]->rect.y1 += dy;
  win_list[window_move-1]->rect.y2 += dy;

  hide_cursor();
  if (dy > 0)
  {
    mask_win(x1+dx,y2,x2+dx,y2+dy);
    redraw_rect(x1,y1,x2,y1+dy-1);
  }
  else if (dy != 0)
  {
    mask_win(x1+dx,y1+dy,x2+dx,y1);
    redraw_rect(x1,y2+dy+1,x2,y2);
  }

  if (dx > 0)
  {
    mask_win(x2,y1+dy,x2+dx,y2+dy);
    redraw_rect(x1,y1,x1+dx-1,y2);
  }
  else if (dx != 0)
  {
    mask_win(x1+dx,y1+dy,x1,y2+dy);
    redraw_rect(x2+dx+1,y1,x2,y2);
  }

  draw_win_c(&screen_rect, win_list[window_move-1]);
  show_cursor();

//  win_list[znum-1]->flags |= WF_STOP_OUTPUT;
  set_wflag_stopout(win_list[znum-1], 1);

  pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
}

void perform_button1(int x, int y)
{
  int wnr;

  if ((wnr = check_win(x,y)) && (wnr != znum))
  {
    wnr = shift_z(wnr-1);
    hide_cursor();
    if (znum > 1)
    {
      draw_win_c(&screen_rect, win_list[znum-2]);
      pass_msg(win_list[znum-2],MSG_GUI_REDRAW);
    }
    draw_win_c(&screen_rect, win_list[wnr-1]);
    show_cursor();


    clear_winmask(win_list[wnr-1]);
    mask_win(win_list[wnr-1]->rect.x1,win_list[wnr-1]->rect.y1,win_list[wnr-1]->rect.x2,win_list[wnr-1]->rect.y2);

    pass_msg(win_list[wnr-1],MSG_GUI_REDRAW);
  }

  if (check_title(x, y))
    window_move = shift_z(window_move-1);

  if (check_titlebut(x, y))
  {
    pass_msg(win_list[znum-1],MSG_GUI_WINDOW_CLOSE);
    if (win_list[znum-1]->ptask == NULL)
      destroy_window(win_list[znum-1]);
    return;
  }

  if (mouse_inframe(win_list[znum-1], x, y))
    window_size = 2;

  pass_msg(win_list[znum-1],MSG_MOUSE_BUT1_PRESSED);
}

int mouse_inframe(window_t *win, int x, int y)
{
  rect_t rect1 = {win->rect.x2-10,win->rect.y2-3,win->rect.x2,win->rect.y2};
  rect_t rect2 = {win->rect.x2-3,win->rect.y2-10,win->rect.x2,win->rect.y2};

  return (pt_inrect(&rect1,x,y) || pt_inrect(&rect2,x,y));
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
    redraw_rect(x2+dx+1,y1,x2,y2+abs(dy));
    set_rect(&rect,x2+dx-15,y1,x2+dx,y2);
    draw_win_c(&rect, win_list[znum-1]);
  }
  else if (dx != 0)
  {
    mask_win(x2,y1,x2+dx,y2);
    set_rect(&rect,x2-15,y1,x2+dx,y2);
    draw_win_c(&rect, win_list[znum-1]);
  }

  if (dy < 0)
  {
    redraw_rect(x1,y2+dy+1,x2+abs(dx),y2);
    set_rect(&rect,x1,y2+dy-3,x2,y2+dy);
    draw_win_c(&rect, win_list[znum-1]);
  }
  else if (dy != 0)
  {
    mask_win(x1,y2+1,x2+dx,y2+dy);
    set_rect(&rect,x1,y2-3,x2+abs(dx),y2+dy);
    draw_win_c(&rect, win_list[znum-1]);
  }

  show_cursor();

//  win_list[znum-1]->flags |= WF_STOP_OUTPUT;
  set_wflag_stopout(win_list[znum-1], 1);

  pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
}

void newgui_task()
{
  struct ipc_message *m;
  int mouse_x, mouse_y;
  int old_x, old_y;
  int flags;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_MOUSE_MOVE:
            if ((--move_count > 3) && window_move)
              break;
            old_x = mouse_x;
            old_y = mouse_y;
            mouse_x = m->MSGT_MOUSE_X;
            mouse_y = m->MSGT_MOUSE_Y;

            if (!window_size)
            {
              if (mouse_inframe(win_list[znum-1],mouse_x, mouse_y))
                change_cmask(2);
              else
                change_cmask(1);
            }

            if (window_move)
            {
              int dx, dy;

              dx = mouse_x - old_x;
              dy = mouse_y - old_y;
              move_window(dx,dy);
            }

            if (window_size)
            {
              int dx, dy;

              dx = mouse_x - old_x;
              dy = mouse_y - old_y;
              size_win(dx,dy,mouse_x,mouse_y);
            }
            pass_msg(win_list[znum-1],MSG_MOUSE_MOVE);
            break;

       case MSG_MOUSE_BUT1_PRESSED:
            perform_button1(mouse_x,mouse_y);
            break;
       case MSG_MOUSE_BUT1_RELEASED:
            window_move = 0;
            window_size = 0;
            pass_msg(win_list[znum-1],MSG_MOUSE_BUT1_RELEASED);
            break;

       case MSG_READ_CHAR_REPLY:
            spass_msg(win_list[znum-1],m);
            break;

       case MSG_MEMORY_VALLOC:
//            spass_msg(win_app,m);
            break;
       case MSG_MEMORY_VFREE:
//            spass_msg(win_app,MSG_MEMORY_VFREE);
            break;
     }
  }

}

void newgui_init()
{
  int i;

  newgui_pid = 0;
  window_move = 0;
  window_size = 0;
  znum = 0;

  screen_rect.x1 = 0;
  screen_rect.y1 = 0;
  screen_rect.x2 = 1023;
  screen_rect.y2 = 767;

  for (i=0;i<MAX_WIN;i++)
    win_list[i] = NULL;

  /* High priority */
  newgui_pid = ((struct task_struct*)CreateKernelTask((unsigned int)newgui_task, "new_gui", HP,0))->pid;

  hide_cursor();
  draw_background(screen_rect.x1,screen_rect.y1,screen_rect.x2,screen_rect.y2);
  show_cursor();
}

window_t *create_app(void *pTask, char *pname, char *wname, int x1, int y1, int length, int width)
{
  window_t *win;

  win = create_window(x1,y1,x1+length,y1+width,wname);
  win->ptask = ((struct task_struct*)CreateKernelTask((unsigned int)pTask, pname, NP,0));

  mask_win(win->rect.x1,win->rect.x1,win->rect.x2,win->rect.y2);

  if (znum > 1)
    pass_msg(win_list[znum-2],MSG_GUI_REDRAW);

  pass_msg(win_list[znum-1],MSG_GUI_WINDOW_CREATED);
  pass_msg(win_list[znum-1],MSG_GUI_REDRAW);

  return win;
}

void win_hide_cursor(window_t *win)
{
  if (mouse_inrect(&win->rect))
    hide_cursor();
}

void win_show_cursor(window_t *win)
{
  show_cursor();
}

void app_redraw(int nr, int n, struct ipc_message *m)
{
  int i, hide;

  if (nr == -3)
  {
    win_hide_cursor(win_app);
    for (i=0;i<20;i++)
      if (str_app[i][0] != '\0')
        win_outs(win_app,10,30+i*13,COLOR_LIGHTGRAY,str_app[i]);

    win_show_cursor(win_app);
    return;
  }

  if (nr != -1)
    sprintf(str_app[nr],"Mouse moved %d",n);


  win_hide_cursor(win_app);

  if (nr == -2)
    win_draw_fill_rect(win_app,10,30,250,290,COLOR_LIGHTGRAY);
  else
    for (i=0;i<20;i++)
      if (str_app[i][0] != '\0')
        win_outs(win_app,10,30+i*13,COLOR_BLACK,str_app[i]);

  win_show_cursor(win_app);
}

void app_test()
{
  struct ipc_message *m;
  unsigned int ret;
  int i,j=0,n=0;
  char string[20][256];

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  for (i=0;i<20;i++)
  {
    string[i][0] = '\0';
    str_app[i] = string[i];
  }

  i=0;
  j=0;

  while (1)
  {
/*
    for (j=0;j<100000;j++)
      j = j;

            if (i >= 20)
            {
              for (i=0;i<20;i++)
                str_app[i][0] = '\0';
              i = 0;
              app_redraw(-2,n,m);
            }
            app_redraw(i,n,m);
            i++;
            n++;
*/
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_MOUSE_MOVE:
            if (i >= 20)
            {
              app_redraw(-3,n,m);
              for (i=0;i<19;i++)
                str_app[i] = str_app[i+1];
              i = 19;
              str_app[i] = string[j];
              j++;
              if (j == 20) j = 0;
            }
            app_redraw(i,n,m);
            i++;
            n++;
            break;
       case MSG_GUI_WINDOW_CREATED:
            sprintf(str_app[i],"Window created");
	    i++;
            app_redraw(-1,n++,m);
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_window(win_app);
            win_app = 0;
//            RemoveTask(win_app->ptask);
            break;
       case MSG_GUI_REDRAW:
            set_wflag_stopout(win_app, 0);
		    app_redraw(-1,n,m);
            set_wflag_guiredraw(win_app, 0);
            break;
     }

  }

}

void testapp()
{
  win_app = create_app(app_test,"tapp","Test Application",400,200,400,300);
}
