/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\newgui.c
/* Last Update: 22.12.1998
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

extern unsigned char char_tab2[257*8];

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
  field = ((x2-x1)*(y2-y1))/8 + (x2-x1);       /* save area */
  win->bitmask = (char*) valloc(field);

  for (i=0;i<field;i++)
    win->bitmask[i] = 0xff;

  win->pid = 0;

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
      draw_background(win_list[i]->rect.x1,win_list[i]->rect.y1,win_list[i]->rect.x2,win_list[i]->rect.y2);
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
      return 1;
    }
  }

  return 0;
}

inline int win_area(window_t *win)
{
  return (win->rect.x2-win->rect.x1)*(win->rect.y2-win->rect.y1);
}

inline int win_offset(window_t *win, int x, int y)
{
  return (y-win->rect.y1)*(win->rect.x2-win->rect.x1)+(x-win->rect.x1);
}

void set_bitmask(window_t *win, unsigned int nr, char bit)
{

 if (nr > win_area(win)+(win->rect.x2-win->rect.x1))
 {
   printk("newgui.c: set_bitmask - number: %d",nr);
   return 0;
 }

 if (get_bitmask(win,nr) != bit)
     win->bitmask[nr >> 3] ^= (1 << (nr % 8));
}

int get_bitmask(window_t *win, unsigned int nr)
{

 if (nr > win_area(win)+(win->rect.x2-win->rect.x1))
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

void set_bitmask_rect(window_t *win, rect_t rect, char bit)
{
  int i,j;

  if (fit_rect(&rect,&win->rect))
    return;

  for (j=rect.y1;j<rect.y2;j++)
    for (i=rect.x1;i<rect.x2;i++)
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

void draw_win_cw(rect_t *rect, window_t *win)
{
  wcdraw_hline(win,rect, win->rect.x1, win->rect.x2-1, win->rect.y1, LIGHTGRAY);
  wcdraw_vline(win,rect, win->rect.x1, win->rect.y1  , win->rect.y2-1, LIGHTGRAY);

  wcdraw_hline(win,rect, win->rect.x1+1, win->rect.x2-2, win->rect.y1+1, WHITE);
  wcdraw_vline(win,rect, win->rect.x1+1, win->rect.y1+1, win->rect.y2-2, WHITE);

  wcdraw_hline(win,rect, win->rect.x1, win->rect.x2  , win->rect.y2, BLACK);
  wcdraw_vline(win,rect, win->rect.x2, win->rect.y1  , win->rect.y2, BLACK);

  wcdraw_hline(win,rect, win->rect.x1+1, win->rect.x2-1, win->rect.y2-1, GRAY);
  wcdraw_vline(win,rect, win->rect.x2-1, win->rect.y1+1, win->rect.y2-1, GRAY);

  wcdraw_hline(win,rect, win->rect.x1+2, win->rect.x2-2, win->rect.y1+2, LIGHTGRAY);
  wcdraw_vline(win,rect, win->rect.x1+2, win->rect.y1+2, win->rect.y1+25, LIGHTGRAY);
  wcdraw_vline(win,rect, win->rect.x2-2, win->rect.y1+2, win->rect.y1+25, LIGHTGRAY);

  wcdraw_fill_rect(win,rect, win->rect.x1+3, win->rect.y1+3, win->rect.x2-3, win->rect.y1+15, (win->z == znum-1) ? BLUE : GRAY);
  wcdraw_fill_rect(win,rect, win->rect.x1+2, win->rect.y1+16, win->rect.x2-2, win->rect.y2-2, LIGHTGRAY);

  wcdraw_hline(win,rect, win->rect.x2-15, win->rect.x2-7, win->rect.y1+5, WHITE);
  wcdraw_vline(win,rect, win->rect.x2-15, win->rect.y1+5, win->rect.y1+12, WHITE);

  wcdraw_hline(win,rect, win->rect.x2-15, win->rect.x2-6, win->rect.y1+13, BLACK);
  wcdraw_vline(win,rect, win->rect.x2-6, win->rect.y1+5, win->rect.y1+13, BLACK);
  wcdraw_fill_rect(win,rect, win->rect.x2-14, win->rect.y1+6, win->rect.x2-7, win->rect.y1+12, LIGHTGRAY);

//  outs_cc(rect, win->rect.x1+6,win->rect.y1+6,WHITE,win->name);
}

void draw_win_c(rect_t *rect, window_t *win)
{
  cdraw_hline(rect, win->rect.x1, win->rect.x2-1, win->rect.y1, LIGHTGRAY);
  cdraw_vline(rect, win->rect.x1, win->rect.y1  , win->rect.y2-1, LIGHTGRAY);

  cdraw_hline(rect, win->rect.x1+1, win->rect.x2-2, win->rect.y1+1, WHITE);
  cdraw_vline(rect, win->rect.x1+1, win->rect.y1+1, win->rect.y2-2, WHITE);

  cdraw_hline(rect, win->rect.x1, win->rect.x2  , win->rect.y2, BLACK);
  cdraw_vline(rect, win->rect.x2, win->rect.y1  , win->rect.y2, BLACK);

  cdraw_hline(rect, win->rect.x1+1, win->rect.x2-1, win->rect.y2-1, GRAY);
  cdraw_vline(rect, win->rect.x2-1, win->rect.y1+1, win->rect.y2-1, GRAY);

  cdraw_hline(rect, win->rect.x1+2, win->rect.x2-2, win->rect.y1+2, LIGHTGRAY);
  cdraw_vline(rect, win->rect.x1+2, win->rect.y1+2, win->rect.y1+25, LIGHTGRAY);
  cdraw_vline(rect, win->rect.x2-2, win->rect.y1+2, win->rect.y1+25, LIGHTGRAY);

  cdraw_fill_rect(rect, win->rect.x1+3, win->rect.y1+3, win->rect.x2-3, win->rect.y1+15, (win->z == znum-1) ? BLUE : GRAY);
  cdraw_fill_rect(rect, win->rect.x1+2, win->rect.y1+16, win->rect.x2-2, win->rect.y2-2, LIGHTGRAY);

  cdraw_hline(rect, win->rect.x2-15, win->rect.x2-7, win->rect.y1+5, WHITE);
  cdraw_vline(rect, win->rect.x2-15, win->rect.y1+5, win->rect.y1+12, WHITE);

  cdraw_hline(rect, win->rect.x2-15, win->rect.x2-6, win->rect.y1+13, BLACK);
  cdraw_vline(rect, win->rect.x2-6, win->rect.y1+5, win->rect.y1+13, BLACK);
  cdraw_fill_rect(rect, win->rect.x2-14, win->rect.y1+6, win->rect.x2-7, win->rect.y1+12, LIGHTGRAY);

  couts_c(rect, win->rect.x1+6,win->rect.y1+6,WHITE,win->name);
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

void wdraw_background(int x1, int y1, int x2, int y2)
{
  int i,j;
  rect_t rect={x1,y1,x2,y2};

  if (fit_rect(&rect,&screen_rect))
    return;

  for (j=rect.y1;j<=rect.y2;j++)
    for (i=rect.x1;i<=rect.x2;i++)
    {
      if (pt_inwin(i,j) == -1)
        put_pixel(i,j,32+j/12);
    }
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
  rect_t rect;

  for (i=znum-1;i>=0;i--)
    if (win_list[i] != NULL)
    {
//      rect = win_list[i]->rect;
      if (pt_inrect(&win_list[i]->rect,x,y))
        return i+1;
    }

  return 0;
}

int pt_inrect(rect_t *rect, int x, int y)
{
  if ((x >= rect->x1) && (x <= rect->x2) && (y >= rect->y1) && (y <= rect->y2))
    return 1;
  else
    return 0;
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
  if (win->pid == 0)
    return;

  m = (struct ipc_message*)valloc(sizeof(struct ipc_message));
  m->type = msg_type;
  send_msg(win->pid, m);
  vfree(m);
}

void spass_msg(window_t *win, struct ipc_message *m)
{
  if (win->pid == 0)
    return;

  send_msg(win->pid, m);
}

void mask_win(window_t *win, int end)
{
  rect_t rect = {win->rect.x1,win->rect.y1,win->rect.x2+1,win->rect.y2+1};
  int i;

  for (i=0;i<end;i++)
  {
    if (win_list[i] != win)
      set_bitmask_rect(win_list[i],rect,0);
  }
}

void nomask_win(window_t *win)
{
  int i, field = ((win->rect.x2-win->rect.x1)*(win->rect.y2-win->rect.y1))/8;

  for (i=0;i<field;i++)
   win->bitmask[i] = 0xff;
}

void unmask_win(int x1, int y1, int x2, int y2)
{
  int i,j;
  rect_t rect = {x1,y1,x2+1,y2+1};
  rect_t rold,wrect;
/*
  for (i=0;i<=znum-1;i++)
  {
    if (i > 0)
    {
      rold = rect;
      if (win_list[i]->rect.x1 > rect.x1) rold.x1 = win_list[i]->rect.x1;
      if (win_list[i]->rect.y1 > rect.y1) rold.y1 = win_list[i]->rect.y1;
      if (win_list[i]->rect.x2 < rect.x2) rold.x2 = win_list[i]->rect.x2;
      if (win_list[i]->rect.y2 < rect.y2) rold.y2 = win_list[i]->rect.y2;

      set_bitmask_rect(win_list[i-1],rect,0);
    }
    set_bitmask_rect(win_list[i],rect,1);
  }
*/
  for (i=0;i<znum-1;i++)
  {
//    set_rect(&wrect,win_list[i]->rect.x1,win_list[i]->rect.y1,win_list[i]->rect.x2,win_list[i]->rect.y2);
    for (j=0;j<i;j++)
    {
      set_rect(&rold,win_list[j]->rect.x1,win_list[j]->rect.y1,win_list[j]->rect.x2,win_list[j]->rect.y2);
      fit_rect(&rold,&rect);

      if (rect_inrect(&win_list[i],&rold))
        set_bitmask_rect(win_list[i-1],rold,0);
    }

    if (rect_inrect(&win_list[i]->rect,&rect))
      set_bitmask_rect(win_list[i],rect,1);
  }
}

void domask_win(int x1, int y1, int x2, int y2)
{
  int i,j,nr;
  rect_t wrect, rect = {x1,y1,x2+1,y2+1};

  fit_rect(&rect,&screen_rect);

  for (j=rect.y1;j<=rect.y2;j++)
    for (i=rect.x1;i<=rect.x2;i++)
    {
      nr = pt_inwin(i,j);
      if (nr != -1)
        set_bitmask(win_list[nr],win_offset(win_list[nr],i,j),0);
    }

/*
  for (i=0;i<znum-1;i++)
  {
    if (win_list[i] != NULL)
    {
      set_rect(&wrect,win_list[i]->rect.x1,win_list[i]->rect.y1,win_list[i]->rect.x2,win_list[i]->rect.y2);
      if (rect_inrect(&wrect,&rect))
        set_bitmask_rect(win_list[i],rect,0);
    }
  }
*/
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

int pt_inwin2(int x, int y, int nr)
{
  int i;
  rect_t rect;

  set_rect(&rect,win_list[nr]->rect.x1,win_list[nr]->rect.y1,win_list[nr]->rect.x2-1,win_list[nr]->rect.y2-1);


  for (i=znum-2;i>=0;i--)
  {
     set_rect(&rect,win_list[i]->rect.x1,win_list[i]->rect.y1,win_list[i]->rect.x2-1,win_list[i]->rect.y2-1);
     if (pt_inrect(&rect,x,y))
       return i;
  }

  return -1;
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
    set_rect(&wrect,win_list[i]->rect.x1,win_list[i]->rect.y1,win_list[i]->rect.x2,win_list[i]->rect.y2);
    if (rect_inrect(&wrect, &rect))
      wlist[j++] = win_list[i];
  }
  zn = j;

  for (j=rect.y1;j<=rect.y2;j++)
  {
    onr = -2;
    oi = rect.x1;

    for (i=rect.x1;i<=rect.x2;i++)
    {
      nr = -1;
      for (k=0;k<zn;k++)
      {
//        set_rect(&wrect,wlist[k]->rect.x1,wlist[k]->rect.y1,wlist[k]->rect.x2-1,wlist[k]->rect.y2-1);
        if (pt_inrect(&wlist[k]->rect,i,j))
        {
          nr = k;
          break;
        }
      }

      if ((onr != nr) || (i == rect.x2))
      {
        if (onr >= 0)
        {
          set_rect(&wrect,oi,j,i,j);
          draw_win_c(&wrect,wlist[onr]);
          pass_msg(wlist[onr],MSG_GUI_REDRAW);
        }
        else
         for (k=oi;k<=i;k++)
          put_pixel(k,j,32+j/12);

        oi = i;
        onr = nr;
      }
    }
  }
/*
      if (nr != -1)
      {
        set_rect(&wrect,i,j,i+8,j);
        draw_win_c(&wrect,wlist[nr]);
      }
      else
       for (k=0;k<5;k++)
        put_pixel(i+k,j,32+j/12);
    }
*/
/*
  for (i=0;i<znum-1;i++)
  {
    set_rect(&wrect,win_list[i]->rect.x1,win_list[i]->rect.y1,win_list[i]->rect.x2,win_list[i]->rect.y2);
    if (rect_inrect(&wrect,&rect))
    {
      draw_win_c(&rect,win_list[i]);
      pass_msg(win_list[i],MSG_GUI_REDRAW);
    }
  }
*/
}
/*
void redraw_rect(int x1, int y1, int x2, int y2)
{
  int i,j,k,nr=0,zn;
  rect_t rect, wrect, rect2;
  window_t *wlist[MAX_WIN];

  set_rect(&rect,x1,y1,x2,y2);

  j = 0;
  for (i=znum-2;i>=0;i--)
  {
    set_rect(&wrect,win_list[i]->rect.x1,win_list[i]->rect.y1,win_list[i]->rect.x2,win_list[i]->rect.y2);
    if (rect_inrect(&wrect, &rect))
      wlist[j++] = win_list[i];
  }
  zn = j;

  for (j=rect.y1;j<=rect.y2;j++)
    for (i=rect.x1;i<=rect.x2;i+=5)
    {
//      nr = pt_inwin2(i,j,nr);
      nr = -1;
      for (k=0;k<zn;k++)
      {
        set_rect(&wrect,wlist[k]->rect.x1,wlist[k]->rect.y1,wlist[k]->rect.x2-1,wlist[k]->rect.y2-1);
        set_rect(&rect2,i,j,i+5,j);
        if (rect_inrect(&wrect,&rect2))
        {
          nr = k;
          break;
        }
      }

      if (nr != -1)
      {
        set_rect(&wrect,i,j,i+8,j);
        draw_win_c(&wrect,wlist[nr]);
      }
      else
       for (k=0;k<5;k++)
        put_pixel(i+k,j,32+j/12);
    }
}
*/
/*
void redraw_rect(int x1, int y1, int x2, int y2)
{
  int i,j,nr=0;
  rect_t rect, wrect, rect2;
  window_t *wlist[MAX_WIN];

  set_rect(&rect,x1,y1,x2,y2);

  for (i=znum-2;i>=0;i--)
  {

  }

  for (j=y1;j<=y2;j++)
    for (i=x1;i<=x2;i++)
    {
//      nr = pt_inwin2(i,j,nr);
      for (nr=znum-2;nr>=0;nr--)
      {
        set_rect(&rect2,win_list[nr]->rect.x1,win_list[nr]->rect.y1,win_list[nr]->rect.x2-1,win_list[nr]->rect.y2-1);
        if (pt_inrect(&rect2,i,j))
          break;
      }

      if (nr != -1)
      {
        set_rect(&rect,i,j,i+1,j+1);
        draw_win_c(&rect,win_list[nr]);
      }
      else
        put_pixel(i,j,32+j/12);
    }
}
*/
void masking(int x1, int y1, int x2, int y2)
{
  int i,j,nr;
  rect_t rect = {x1,y1,x2,y2};

  fit_rect(&rect,&screen_rect);

  for (j=rect.y1;j<=rect.y2;j++)
    for (i=rect.x1;i<=rect.x2;i++)
    {
      nr = pt_inwin(i,j);

      if (nr != -1)
        set_bitmask(win_list[nr],win_offset(win_list[nr],i,j),1);
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
/*
  for (i=0;i<=znum-1;i++)
  {
    nomask_win(win_list[i]);
    mask_win(win_list[i],i);
  }
*/
/*
  if (dy > 0)
  {
///    unmask_win(x1,y1,x2,y1+dy);
    domask_win(x1+dx,y2,x2+dx,y2+dy);
    masking(x1,y1,x2,y1+dy);
  }
  else
  {
///    unmask_win(x1,y2+dy,x2,y2);
    domask_win(x1+dx,y1+dy,x2+dx,y1);
    masking(x1,y2+dy,x2,y2);
  }

  if (dx > 0)
  {
///    unmask_win(x1,y1,x1+dx,y2);
    domask_win(x2,y1+dy,x2+dx,y2+dy);
    masking(x1,y1,x1+dx,y2);
  }
  else
  {
///    unmask_win(x2+dx,y1,x2,y2);
    domask_win(x1+dx,y1+dy,x1,y2+dy);
    masking(x2+dx,y1,x2,y2);
  }
*/
  hide_cursor();
  if (dy > 0)
  {
//    draw_background(x1,y1,x2,y1+dy);
    redraw_rect(x1,y1,x2,y1+dy);
  }
  else if (dy != 0)
  {
//    draw_background(x1,y2+dy,x2,y2);
    redraw_rect(x1,y2+dy,x2,y2);
  }

  if (dx > 0)
  {
//    draw_background(x1,y1,x1+dx,y2);
    redraw_rect(x1,y1,x1+dx,y2);
  }
  else if (dx != 0)
  {
//    draw_background(x2+dx,y1,x2,y2);
    redraw_rect(x2+dx,y1,x2,y2);
  }

  draw_win_c(&screen_rect, win_list[window_move-1]);
  show_cursor();

  win_stopout = 1;
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

/*
    nomask_win(win_list[wnr-1]);
    mask_win(win_list[wnr-1],znum-1);
*/
    domask_win(win_list[wnr-1]->rect.x1,win_list[wnr-1]->rect.y1,win_list[wnr-1]->rect.x2,win_list[wnr-1]->rect.y2);
    masking(win_list[wnr-1]->rect.x1,win_list[wnr-1]->rect.y1,win_list[wnr-1]->rect.x2,win_list[wnr-1]->rect.y2);

    pass_msg(win_list[wnr-1],MSG_GUI_REDRAW);
  }

  if (check_title(x, y))
    window_move = shift_z(window_move-1);

  if (check_titlebut(x, y))
  {
// win_app
    pass_msg(win_list[znum-1],MSG_GUI_WINDOW_CLOSE);
    if (win_list[znum-1] != win_app)
      destroy_window(win_list[znum-1]);
    return;
  }
// win_app
  pass_msg(win_list[znum-1],MSG_MOUSE_BUT1_PRESSED);
}

void newgui_task()
{
  struct ipc_message *m;
  int mouse_x, mouse_y;
  int old_x, old_y;

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

            if (window_move)
            {
              int dx, dy;

              dx = mouse_x - old_x;
              dy = mouse_y - old_y;
              move_window(dx,dy);
            }
            // win_app
            pass_msg(win_list[znum-1],MSG_MOUSE_MOVE);
            break;

       case MSG_MOUSE_BUT1_PRESSED:
            perform_button1(mouse_x,mouse_y);
            break;
       case MSG_MOUSE_BUT1_RELEASED:
            window_move = 0;
            pass_msg(win_list[znum-1],MSG_MOUSE_BUT1_RELEASED);
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
  win_stopout = 0;
  znum = 0;

  screen_rect.x1 = 0;
  screen_rect.y1 = 0;
  screen_rect.x2 = 1023;
  screen_rect.y2 = 767;

  for (i=0;i<MAX_WIN;i++)
    win_list[i] = NULL;

  /* Real Time */
  newgui_pid = ((struct task_struct*)CreateKernelTask((unsigned int)newgui_task, "new_gui", RTP,0))->pid;

  hide_cursor();
  draw_background(screen_rect.x1,screen_rect.y1,screen_rect.x2,screen_rect.y2);
  show_cursor();
}

window_t *create_app(void *pTask, char *tname, int x1, int y1, int length, int width)
{
  window_t *win;

  win = create_window(x1,y1,x1+length,y1+width,tname);
  win->pid = ((struct task_struct*)CreateKernelTask((unsigned int)pTask, tname, NP,0))->pid;

  if (znum > 1)
    pass_msg(win_list[znum-2],MSG_GUI_REDRAW);
  pass_msg(win_list[znum-1],MSG_GUI_REDRAW);

  return win;
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

void app_redraw(int nr, int n, struct ipc_message *m)
{
  int i, hide;
//  rect_t rect = {win_app->rect.x1,win_app->rect.y1,win_app->rect.x2,win_app->rect.y2};

  if (nr != -1)
    sprintf(str_app[nr],"Mouse moved %d",n);
//    sprintf(str_app[nr],"Valloc called: Adr: %d Size: %d PID: %d",
//            m->MSGT_MEMORY_ADDRESS,m->MSGT_MEMORY_SIZE,m->MSGT_MEMORY_PID);

  hide = hide_cursor_rect(&win_app->rect);

  if (nr == -2)
    win_draw_fill_rect(win_app,10,30,300,290,LIGHTGRAY);
  else
    for (i=0;i<20;i++)
      if (str_app[i][0] != '\0')
        win_outs(win_app,10,30+i*13,0,str_app[i]);

  if (hide) show_cursor();
}

void app_test()
{
  struct ipc_message *m;
  unsigned int ret;
  int i,j=0,n=0;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  for (i=0;i<20;i++)
    str_app[i][0] = '\0';

  i=0;

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
/*       case MSG_MEMORY_VALLOC:
            if (i >= 20)
            {
              for (i=0;i<20;i++)
                str_app[i][0] = '\0';
              i = 0;
              app_redraw(-2,n,m);
              break;
            }
            app_redraw(i,n,m);
            i++;
            break;
*/       case MSG_MOUSE_MOVE:
            if (i >= 20)
            {
              break;
              for (i=0;i<20;i++)
                str_app[i][0] = '\0';
              i = 0;
              app_redraw(-2,n,m);
              break;
            }
            app_redraw(i,n,m);
            i++;
            break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_window(win_app);
            return;
            break;
       case MSG_GUI_REDRAW:

// auf šberlappung wie bei Mouse move berprfen !!!


//            hide_cursor();
//            win_draw_fill_rect_c(win_app,10,50,100,100,i);
//            show_cursor();

            app_redraw(-1,n++,m);
            break;
     }

  }

}

void testapp()
{
  win_app = create_app(app_test,"Test Application",400,200,400,300);
}
