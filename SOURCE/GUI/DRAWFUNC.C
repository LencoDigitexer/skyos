/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\drawfunc.c
/* Last Update: 27.08.1999
/* Version    : alpha
/* Coded by   : Hayrapetian Gregory
/* Docus      :
/************************************************************************/

#include "newgui.h"
#include "sched.h"
#include "svga\svgadev.h"

extern display_draw_table active_draw_table;

extern unsigned char char_tab2[257*8];

/*
 *  Drawing functions with clipping (cdraw)
 *  Used only by GUI
 *
*/

int clip_rect(rect_t *crect, int *x1, int *y1, int *x2, int *y2)
{
  if ((*x1 > crect->x2) || (*y1 > crect->y2) || (*x2 < crect->x1) || (*y2 < crect->y1)) return 1;

  if (*x1 < crect->x1) *x1 = crect->x1;
  if (*y1 < crect->y1) *y1 = crect->y1;
  if (*x2 > crect->x2) *x2 = crect->x2;
  if (*y2 > crect->y2) *y2 = crect->y2;

  return 0;
}

void cdraw_pixel(rect_t *rect,int x, int y, int color)
{
  if ((x >= rect->x1) && (x <= rect->x2) && (y >= rect->y1) && (y <= rect->y2))
    active_draw_table.draw_pixel(x,y,color);
}

void cdraw_line(rect_t *rect, int x1, int y1, int x2, int y2, unsigned char color)
{
  int i, deltax, deltay, numpixels,
    d, dinc1, dinc2,
    x, xinc1, xinc2,
    y, yinc1, yinc2;

  deltax = abs(x2 - x1);
  deltay = abs(y2 - y1);

  if (deltax >= deltay)
  {
      numpixels = deltax + 1;
      d = (2 * deltay) - deltax;
      dinc1 = deltay << 1;
      dinc2 = (deltay - deltax) << 1;
      xinc1 = 1;
      xinc2 = 1;
      yinc1 = 0;
      yinc2 = 1;
  }
  else
  {
      numpixels = deltay + 1;
      d = (2 * deltax) - deltay;
      dinc1 = deltax << 1;
      dinc2 = (deltax - deltay) << 1;
      xinc1 = 0;
      xinc2 = 1;
      yinc1 = 1;
      yinc2 = 1;
  }
  if (x1 > x2)
  {
      xinc1 = - xinc1;
      xinc2 = - xinc2;
  }
  if (y1 > y2)
  {
      yinc1 = - yinc1;
      yinc2 = - yinc2;
  }
  x = x1;
  y = y1;

  for (i=1;i<=numpixels;i++)
  {
     cdraw_pixel(rect,x, y, color);
      if (d < 0)
      {
	  d = d + dinc1;
	  x = x + xinc1;
	  y = y + yinc1;
      }
      else
      {
	  d = d + dinc2;
	  x = x + xinc2;
	  y = y + yinc2;
      }
  }
}

void cdraw_hline(rect_t *rect, int x1, int x2, int y, int color)
{
  if ((x1 > rect->x2) || (x2 < rect->x1) || (y < rect->y1) || (y > rect->y2)) return;
  if (x1 < rect->x1) x1 = rect->x1;
  if (x2 > rect->x2) x2 = rect->x2;

  active_draw_table.draw_hline(x1,x2,y,color);
}

void cdraw_vline(rect_t *rect, int x, int y1, int y2, int color)
{
  if ((y1 > rect->y2) || (y2 < rect->y1) || (x > rect->x2) || (x < rect->x1)) return;
  if (y1 < rect->y1) y1 = rect->y1;
  if (y2 > rect->y2) y2 = rect->y2;

  active_draw_table.draw_vline(x,y1,y2,color);
}

void cdraw_fill_rect(rect_t *rect, int x1, int y1, int x2, int y2, int color)
{
  int i, j;

  if ((x1 > rect->x2) || (y1 > rect->y2) || (x2 < rect->x1) || (y2 < rect->y1)) return;
  if (x1 < rect->x1) x1 = rect->x1;
  if (y1 < rect->y1) y1 = rect->y1;
  if (x2 > rect->x2) x2 = rect->x2;
  if (y2 > rect->y2) y2 = rect->y2;

  for (j=y1;j<=y2;j++)
    active_draw_table.draw_hline(x1,x2,j,color);
}

void cdraw_rect(rect_t *rect, int x1, int y1, int x2, int y2, int color)
{
  cdraw_hline(rect, x1, x2, y1, color);
  cdraw_hline(rect, x1, x2, y2, color);
  cdraw_vline(rect, x1, y1, y2, color);
  cdraw_vline(rect, x2, y1, y2, color);
}

void coutch(rect_t *rect, unsigned char ch, int x, int y, unsigned char vg, unsigned char hg)
{
  int i,k;
  unsigned char maske;
  rect_t mrect;

  if (ch<30) return;

  set_rect(&mrect,x,y,x+7,y+7);
  if (fit_rect(&mrect,rect))
    return;

  if (hg != 255)
  {
    for (i=mrect.y1;i<=mrect.y2;i++)
    {
      maske = char_tab2[ch*8+(i-y)];
      maske <<= mrect.x1 - x;

      for (k=mrect.x1; k<=mrect.x2;++k,maske <<=1)
        active_draw_table.draw_pixel(k,i,(maske & 128) ? vg : hg);
    }
  }
  else
  {
    for (i=mrect.y1;i<=mrect.y2;i++)
    {
      maske = char_tab2[ch*8+(i-y)];
      maske <<= mrect.x1 - x;

      for (k=mrect.x1; k<=mrect.x2;++k,maske <<=1)
        if (maske & 128)
          active_draw_table.draw_pixel(k,i,vg);
    }
  }
}

void couts(rect_t *rect, int x, int y, char *string, unsigned char fcol, unsigned char bcol)
{
  int x2;

  x2 = x+strlen(string)*8;

  if ((x > rect->x2) || (y > rect->y2) || ((y+7) < rect->y1) ||  (x2 < rect->x1)) return;

  for (;(*string) && (x<=rect->x2);x+=8)
  {
    coutch(rect,*string,x,y,fcol,bcol);
    string++;
  }
}

/*
void couts(rect_t *rect, int x, int y, char *str,unsigned char fcol, unsigned char bcol)
{
  rect_t srect = {x,y,x+strlen(str)*8,y+7};
  int x1, i, k;
  unsigned char maske;
  rect_t mrect;

  if (fit_rect(&srect, rect))
    return;

  x1 = x + ((srect.x1 - x)/8)*8;

  set_rect(&mrect,x,y,x+7,y+7);

  for (;(*str) && (x<=srect.x2);x+=8)
  if ((x >= x1) && (*str >= 30))
  {
    mrect.x1 = x;
    mrect.x2 = x+7;
    fit_rect(&mrect,rect);

    if (bcol != 255)
    {
      for (i=mrect.y1;i<=mrect.y2;i++)
      {
        maske = char_tab2[*str*8+(i-y)];
        maske <<= mrect.x1 - x;

        for (k=mrect.x1; k<=mrect.x2;++k,maske <<=1)
          setpix(k,i,(maske & 128) ? fcol : bcol);
      }
    }
    else
    {
      for (i=mrect.y1;i<=mrect.y2;i++)
      {
        maske = char_tab2[*str*8+(i-y)];
        maske <<= mrect.x1 - x;

        for (k=mrect.x1; k<=mrect.x2;++k,maske <<=1)
          if (maske & 128)
            setpix(k,i,fcol);
      }
    }
    str++;
  }
}
*/


void cdraw_hline_inv(rect_t *rect, int x1, int x2, int y, int color)
{
  int i;

  if ((x1 > rect->x2) || (x2 < rect->x1) || (y < rect->y1) || (y > rect->y2)) return;
  if (x1 < rect->x1) x1 = rect->x1;
  if (x2 > rect->x2) x2 = rect->x2;

  FillArea(x1,y,x2,y,0xf,3);
//  for (i=x1;i<=x2;i++)
//    setpix(i,y,~getpix(i,y));
}

void cdraw_vline_inv(rect_t *rect, int x, int y1, int y2, int color)
{
  int i;

  if ((y1 > rect->y2) || (y2 < rect->y1) || (x > rect->x2) || (x < rect->x1)) return;
  if (y1 < rect->y1) y1 = rect->y1;
  if (y2 > rect->y2) y2 = rect->y2;

  FillArea(x,y1,x,y2,0xf,3);
//  for (i=y1;i<=y2;i++)
//    setpix(x,i,~getpix(x,i));
}

void win_drawconst(window_t *win, void (*draw_func)())
{
  rect_t rrect, drect;
  visible_rect_t *vrect;

  enter_critical_section(&csect_gui);
/*
  x += win->rect.x1+3;
  y += win->rect.y1+17;

  set_screen_rect(&drect,x,y,x+strlen(str)*8,y+8);
*/
  if (mouse_inrect(&drect))
    hide_cursor();

  for (vrect = win->visible_start;vrect;vrect = vrect->next)
  {
    rrect = vrect->rect;
    if (!fit_rect(&rrect, &win->client_rect))
      if (rect_inrect(&drect,&rrect))
        draw_func();
  }

  show_cursor();

  leave_critical_section(&csect_gui);
}

void win_outs(window_t *win, int x, int y, char *str, int fcol, int bcol)
{
  rect_t rrect, drect;
  visible_rect_t *vrect;

  enter_critical_section(&csect_gui);

  vrect = win->visible_start;

  x += win->rect.x1+3;
  y += win->rect.y1+17;

  set_screen_rect(&drect,x,y,x+strlen(str)*8,y+8);

  if (mouse_inrect(&drect))
    hide_cursor();

  while (vrect)
  {
    if (rect_inrect(&drect,&vrect->rect))
    {
      set_rect(&rrect, vrect->rect.x1, vrect->rect.y1, vrect->rect.x2, vrect->rect.y2);
      fit_rect(&rrect, &win->client_rect);
      couts(&rrect,x,y,str,fcol,bcol);
    }

    vrect = vrect->next;
  }

  leave_critical_section(&csect_gui);

  show_cursor();
}


void win_draw_fill_rect(window_t *win, int x1, int y1, int x2, int y2, int color)
{
  rect_t drect, rrect;
  visible_rect_t *vrect;
  int flags;

  enter_critical_section(&csect_gui);

  vrect = win->visible_start;

  x1 += win->rect.x1+3;
  y1 += win->rect.y1+17;
  x2 += win->rect.x1+3;
  y2 += win->rect.y1+17;

  set_screen_rect(&drect,x1,y1,x2,y2);

  while (vrect)
  {
    if (rect_inrect(&drect,&vrect->rect))
    {
      set_rect(&rrect, vrect->rect.x1, vrect->rect.y1, vrect->rect.x2, vrect->rect.y2);
      fit_rect(&rrect, &win->client_rect);

      fit_rect(&rrect,&drect);
      if (mouse_inrect(&rrect))
        hide_cursor();

      cdraw_fill_rect(&rrect, x1, y1, x2, y2, color);

      show_cursor();
    }

    vrect = vrect->next;
  }

  leave_critical_section(&csect_gui);
}

void owin_draw_vline(window_t *win, int x, int y1, int y2, int color)
{
  struct ipc_message *m;

  m = (struct ipc_message*)valloc(sizeof(struct ipc_message));
  m->type = MSG_GUI_DRAW_VLINE;
  m->MSGT_GUI_WINADR = win;
  m->MSGT_GUI_X1 = x;
  m->MSGT_GUI_Y1 = y1;
  m->MSGT_GUI_Y2 = y2;
  m->MSGT_GUI_COLOR = color;
  send_msg(gfx_pid, m);
  vfree(m);
}

void win_draw_vline(window_t *win, int x, int y1, int y2, int color)
{
  rect_t rect, drect, rrect;
  visible_rect_t *vrect = win->visible_start;
  int flags;

  set_screen_rect(&rect,win->rect.x1+3,win->rect.y1+17,win->rect.x2-3,win->rect.y2-3);
  x += win->rect.x1+3;
  y1 += win->rect.y1+17;
  y2 += win->rect.y1+17;

  set_screen_rect(&drect,x,y1,x,y2);

  if (mouse_inrect(&drect))
    hide_cursor();

  save_flags(flags);
  cli();

  while (vrect)
  {
    if (rect_inrect(&drect,&vrect->rect))
    {
      set_rect(&rrect, vrect->rect.x1, vrect->rect.y1, vrect->rect.x2, vrect->rect.y2);
      if (!fit_rect(&rrect, &win->client_rect))
        cdraw_vline(&rrect, x, y1, y2, color);
    }

    vrect = vrect->next;
  }

  restore_flags(flags);

  show_cursor();
}

void cwin_draw_vline(window_t *win, rect_t *rect, int x, int y1, int y2, int color)
{
  rect_t drect, rrect;
  visible_rect_t *vrect = win->visible_start;

  x += win->rect.x1+3;
  y1 += win->rect.y1+17;
  y2 += win->rect.y1+17;

  set_screen_rect(&drect,x,y1,x,y2);

  if (mouse_inrect(&drect))
    hide_cursor();

  while (vrect)
  {
    if (rect_inrect(&drect,&vrect->rect))
    {
      fit_rect(&rrect, rect);
      fit_rect(&rrect, vrect);
      cdraw_vline(rect, x, y1, y2, color);
    }

    vrect = vrect->next;
  }

  show_cursor();
}


void gwin_draw_vline(window_t *win, int x, int y1, int y2, int color)
{
  rect_t drect;
  visible_rect_t *vrect = win->visible_start;

  x += win->rect.x1+3;
  y1 += win->rect.y1+17;
  y2 += win->rect.y1+17;

  set_screen_rect(&drect,x,y1,x,y2);

  if (mouse_inrect(&drect))
    hide_cursor();

  while (vrect)
  {
    if (rect_inrect(&drect,&vrect->rect))
      cdraw_vline(vrect, x, y1, y2, color);

    vrect = vrect->next;
  }

  show_cursor();
}

void win_draw_hline(window_t *win, int x1, int x2, int y, int color)
{
  rect_t drect, rrect;
  visible_rect_t *vrect = win->visible_start;
  int flags;

  x1 += win->rect.x1+3;
  x2 += win->rect.x1+3;
  y += win->rect.y1+17;

  set_screen_rect(&drect,x1,y,x2,y);

  if (mouse_inrect(&drect))
    hide_cursor();

  save_flags(flags);
  cli();

  while (vrect)
  {
    if (rect_inrect(&drect,&vrect->rect))
    {
      set_rect(&rrect, vrect->rect.x1, vrect->rect.y1, vrect->rect.x2, vrect->rect.y2);
      fit_rect(&rrect, &win->client_rect);
      cdraw_hline(&rrect, x1, x2, y, color);
    }

    vrect = vrect->next;
  }

  restore_flags(flags);

  show_cursor();
}

void win_draw_pixel(window_t *win, int x, int y, int color)
{
  rect_t drect, rrect;
  visible_rect_t *vrect;
  int flags;

  enter_critical_section(&csect_gui);

  vrect = win->visible_start;

  x += win->rect.x1+3;
  y += win->rect.y1+17;

  set_screen_rect(&drect,x,y,x,y);

  while (vrect)
  {
    if (rect_inrect(&drect,&vrect->rect))
    {
      set_rect(&rrect, vrect->rect.x1, vrect->rect.y1, vrect->rect.x2, vrect->rect.y2);
      fit_rect(&rrect, &win->client_rect);

      fit_rect(&rrect,&drect);
      if (mouse_inrect(&rrect))
        hide_cursor();

      cdraw_pixel(&rrect, x, y, color);

      show_cursor();
    }

    vrect = vrect->next;
  }

  leave_critical_section(&csect_gui);

}

/* circlefill:
 *  Draws a filled circle.
 */
void cdraw_circlefill(rect_t *rect, int x, int y, int radius, int color)
{
   int cx = 0;
   int cy = radius;
   int df = 1 - radius; 
   int d_e = 3;
   int d_se = -2 * radius + 5;
   int clip, sx, sy, dx, dy;

   do
   {
      cdraw_hline(rect, x-cy, x+cy, y-cx, color);

      if (cx)
	     cdraw_hline(rect, x-cy, x+cy, y+cx, color);

      if (df < 0)  {
	 df += d_e;
	 d_e += 2;
	 d_se += 2;
      }
      else { 
	 if (cx != cy) {
	    cdraw_hline(rect, x-cx, x+cx, y-cy, color);

	    if (cy)
	       cdraw_hline(rect, x-cx, x+cx, y+cy, color);
	 }

	 df += d_se;
	 d_e += 2;
	 d_se += 4;
	 cy--;
      } 

      cx++; 

   } while (cx <= cy);
}

/* do_circle:
 *  Helper function for the circle drawing routines. Calculates the points
 *  in a circle of radius r around point x, y, and calls the specified 
 *  routine for each one. The output proc will be passed first a copy of
 *  the bmp parameter, then the x, y point, then a copy of the d parameter
 *  (so putpixel() can be used as the callback).
 */

void cdraw_circle(rect_t *rect, int x, int y, int radius, int d)
{
   int cx = 0;
   int cy = radius;
   int df = 1 - radius; 
   int d_e = 3;
   int d_se = -2 * radius + 5;

   do {
      cdraw_pixel(rect, x+cx, y+cy, d);

      if (cx) 
	     cdraw_pixel(rect, x-cx, y+cy, d);

      if (cy) 
	     cdraw_pixel(rect, x+cx, y-cy, d);

      if ((cx) && (cy)) 
	     cdraw_pixel(rect, x-cx, y-cy, d);

      if (cx != cy) {
	     cdraw_pixel(rect,  x+cy, y+cx, d);

	 if (cx) 
	    cdraw_pixel(rect,  x+cy, y-cx, d);

	 if (cy) 
	    cdraw_pixel(rect,  x-cy, y+cx, d);

	 if (cx && cy) 
	    cdraw_pixel(rect,  x-cy, y-cx, d);
      }

      if (df < 0)  {
	 df += d_e;
	 d_e += 2;
	 d_se += 2;
      }
      else { 
	 df += d_se;
	 d_e += 2;
	 d_se += 4;
	 cy--;
      } 

      cx++; 

   } while (cx <= cy);
}

void win_draw_circle(window_t *win, int x, int y, int radius, int color)
{
  rect_t drect, rrect;
  visible_rect_t *vrect;
  int flags;
  int x1, y1;
  int x2, y2;

  enter_critical_section(&csect_gui);

  vrect = win->visible_start;

  x += win->rect.x1+3;
  y += win->rect.y1+17;
  x1 = x-radius-1;
  y1 = y-radius-1;
  x2 = x+radius+1;
  y2 = y+radius+1;

  set_screen_rect(&drect,x1,y1,x2,y2);

  while (vrect)
  {
    if (rect_inrect(&drect,&vrect->rect))
    {
      set_rect(&rrect, vrect->rect.x1, vrect->rect.y1, vrect->rect.x2, vrect->rect.y2);
      fit_rect(&rrect, &win->client_rect);

      fit_rect(&rrect,&drect);
      if (mouse_inrect(&rrect))
        hide_cursor();

      cdraw_circle(&rrect, x, y, radius, color);

      show_cursor();
    }

    vrect = vrect->next;
  }

  leave_critical_section(&csect_gui);
}

void win_draw_circlefill(window_t *win, int x, int y, int radius, int color)
{
  rect_t drect, rrect;
  visible_rect_t *vrect;
  int flags;
  int x1, y1;
  int x2, y2;

  enter_critical_section(&csect_gui);

  vrect = win->visible_start;

  x += win->rect.x1+3;
  y += win->rect.y1+17;
  x1 = x-radius-1;
  y1 = y-radius-1;
  x2 = x+radius+1;
  y2 = y+radius+1;

  set_screen_rect(&drect,x1,y1,x2,y2);

  while (vrect)
  {
    if (rect_inrect(&drect,&vrect->rect))
    {
      set_rect(&rrect, vrect->rect.x1, vrect->rect.y1, vrect->rect.x2, vrect->rect.y2);
      fit_rect(&rrect, &win->client_rect);

      fit_rect(&rrect,&drect);
      if (mouse_inrect(&rrect))
        hide_cursor();

      cdraw_circlefill(&rrect, x, y, radius, color);

      show_cursor();
    }

    vrect = vrect->next;
  }

  leave_critical_section(&csect_gui);
}


