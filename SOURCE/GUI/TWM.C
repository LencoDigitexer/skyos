/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\twm.c
/* Last Update: 04.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus from :
/************************************************************************/
/* Definition:
/*   A trivial windows manager. This twm is used temporaly, until the
/*   GUI is coded.
/************************************************************************/
#include <stdarg.h>
#include "system.h"
#include "list.h"
#include "twm.h"
#include "ccm.h"

#define NULL (void*)0

struct list *desktop_windows = NULL;                        //list of all windows

extern int MAXX;
extern int MAXY;

#define BLACK           0
#define BLUE            1
#define RED             4
#define LIGHTGRAY       7
#define DARKGRAY        8
#define LIGHTBLUE       9
#define YELLOW          14
#define WHITE           15

#define LG              7
#define DG              8

// WC  ... Window Color
// TB  ... Title Background
// DB  ... Dialog Background
// DB  ... Dialog Text
// DF  ... Dialog Frame
// TC  ... Title Color
// TL  ... Top Left
// TC  ... Bottom Right

int WC_TB_EXCEPTION  = RED;
int WC_TB_NORMAL     = BLUE;
int WC_DB            = LG;
int WC_DT            = DG;
int WC_DF            = DG;
int WC_DF_EXCEPTION  = BLACK;
int WC_TC_EXCEPTION  = BLACK;
int WC_TC_NORMAL     = WHITE;
int WC_DF_TL         = WHITE;
int WC_DF_BR         = DG;
int BC_1             = LG;
int BC_2             = LG;
int BC_3             = DG;

int window_id = 0;

void clear_window(struct twm_window *window)
{
  int i;
  int j;
  unsigned int flags;

  save_flags(flags);
  cli();

  j = (window->y*2 + window->heigth + 12)/2;

  if (window->style == 128) // means exception
  {
    for (i=j; i>=window->y+12;i--)
    {
       line(window->x+1,i,window->x+window->length-1,i, LG);
       line(window->x+1,j,window->x+window->length-1,j, LG);
       j++;
    }
  }
  else if (window->style == 64) // means alert
  {
    for (i=j; i>=window->y+12;i--)
    {
       line(window->x+1,i,window->x+window->length-1,i, LG);
       line(window->x+1,j,window->x+window->length-1,j, LG);
       j++;
    }
  }

  else
  {
    for (i=j; i>=window->y+12;i--)
    {
       line(window->x+1,i,window->x+window->length-1,i,WC_DB);
       line(window->x+1,j,window->x+window->length-1,j,WC_DB);
       j++;
    }
  }

  window->actx = 5;
  window->acty = 30;

  restore_flags(flags);
}

void close_window(struct twm_window *window)
{
  int i = 0;
  struct twm_window *wi = NULL;

  wi= (struct twm_window*)get_item(desktop_windows,i++);

  while (wi!=NULL)
  {
    if (wi->window_id == window->window_id)
    {
      desktop_windows=(struct list*)del_item(desktop_windows,i-1);
      return;
    }
    wi=(struct twm_window*)get_item(desktop_windows,i++);
  }
}

void redraw_all_windows(void)
{
  int i = 0;
  struct twm_window *wi = NULL;

  wi= (struct twm_window*)get_item(desktop_windows,i++);

  while (wi!=NULL)
  {
    draw_window(wi);
    wi=(struct twm_window*)get_item(desktop_windows,i++);
  }
}

void draw_window(struct twm_window *window)
{
   unsigned int flags;

   save_flags(flags);
   cli();

   if (window->style == 128) // means exception
   {
     box(window->x, window->y, window->x+ window->length, window->y + 11, WC_TB_EXCEPTION,WC_TB_EXCEPTION,WC_TB_EXCEPTION);
     rect(window->x-1,window->y-1,window->x+window->length+1, window->y+window->heigth+1, WC_DF_EXCEPTION);

     box(window->x+window->length-30,window->y+2,window->x+window->length-21,window->y+10,BC_1,BC_2,BC_3);
     box(window->x+window->length-20,window->y+2,window->x+window->length-11,window->y+10,BC_1,BC_2,BC_3);
     box(window->x+window->length-10,window->y+2,window->x+window->length-1,window->y+10,BC_1,BC_2,BC_3);
     box(window->x+2,window->y+2,window->x+12,window->y+10,BC_1,BC_2,BC_3);

     line(window->x+window->length-8,window->y+4,window->x+window->length-3,window->y+8,0);
     line(window->x+window->length-8,window->y+8,window->x+window->length-3,window->y+4,0);
     line(window->x+window->length-28,window->y+8,window->x+window->length-23,window->y+8,0);

     rect(window->x+window->length-18,window->y+5,window->x+window->length-15,window->y+8,0);
     rect(window->x+window->length-16,window->y+4,window->x+window->length-13,window->y+7,0);

     clear_window(window);
     outs(window->x+20, window->y +2, WC_TC_EXCEPTION, window->title);
   }
   else if (window->style == 64) // means exception
   {
     box(window->x, window->y, window->x+ window->length, window->y + 11, WC_TB_EXCEPTION,WC_TB_EXCEPTION,WC_TB_EXCEPTION);
     rect(window->x-1,window->y-1,window->x+window->length+1, window->y+window->heigth+1, WC_DF_EXCEPTION);

     box(window->x+window->length-30,window->y+2,window->x+window->length-21,window->y+10,BC_1,BC_2,BC_3);
     box(window->x+window->length-20,window->y+2,window->x+window->length-11,window->y+10,BC_1,BC_2,BC_3);
     box(window->x+window->length-10,window->y+2,window->x+window->length-1,window->y+10,BC_1,BC_2,BC_3);
     box(window->x+2,window->y+2,window->x+12,window->y+10,BC_1,BC_2,BC_3);

     line(window->x+window->length-8,window->y+4,window->x+window->length-3,window->y+8,0);
     line(window->x+window->length-8,window->y+8,window->x+window->length-3,window->y+4,0);
     line(window->x+window->length-28,window->y+8,window->x+window->length-23,window->y+8,0);

     rect(window->x+window->length-18,window->y+5,window->x+window->length-15,window->y+8,0);
     rect(window->x+window->length-16,window->y+4,window->x+window->length-13,window->y+7,0);

     clear_window(window);
     outs(window->x+20, window->y +2, WC_TC_EXCEPTION, window->title);
   }

   else
   {
     clear_window(window);
     box(window->x, window->y, window->x+ window->length, window->y + 11, WC_TB_NORMAL, WC_TB_NORMAL, WC_TB_NORMAL);

     line(window->x-1,window->y-1,window->x+window->length+1,window->y-1,WC_DF_TL);
     line(window->x-1,window->y,window->x+window->length,window->y,WC_DF_TL);
     line(window->x-1,window->y-1,window->x-1,window->y+ window->heigth+1,WC_DF_TL);
     line(window->x,window->y-1,window->x,window->y+ window->heigth,WC_DF_TL);

     line(window->x-1,window->y+ window->heigth+1, window->x+window->length+1,window->y+window->heigth+1,WC_DF_BR);
     line(window->x-1,window->y+ window->heigth, window->x+window->length,window->y+window->heigth,WC_DF_BR);
     line(window->x+window->length+1,window->y-1,window->x+window->length+1,window->y+ window->heigth+1,WC_DF_BR);
     line(window->x+window->length,window->y-1,window->x+window->length,window->y+ window->heigth,WC_DF_BR);

     box(window->x+window->length-30,window->y+2,window->x+window->length-21,window->y+10,BC_1,BC_2,BC_3);
     box(window->x+window->length-20,window->y+2,window->x+window->length-11,window->y+10,BC_1,BC_2,BC_3);
     box(window->x+window->length-10,window->y+2,window->x+window->length-1,window->y+10,BC_1,BC_2,BC_3);
     box(window->x+2,window->y+2,window->x+12,window->y+10,BC_1,BC_2,BC_3);

     line(window->x+window->length-8,window->y+4,window->x+window->length-3,window->y+8,0);
     line(window->x+window->length-8,window->y+8,window->x+window->length-3,window->y+4,0);
     line(window->x+window->length-28,window->y+8,window->x+window->length-23,window->y+8,0);

     rect(window->x+window->length-18,window->y+5,window->x+window->length-15,window->y+8,0);
     rect(window->x+window->length-16,window->y+4,window->x+window->length-13,window->y+7,0);

     outs(window->x+20, window->y +2, WC_TC_NORMAL, window->title);
   }
  restore_flags(flags);
}

void open_window(struct twm_window *window)
{
  // add window to desktop list

  // every window has its own id
  window->window_id = window_id;
  window_id++;

  desktop_windows = (struct list*)add_item(desktop_windows,window,
    sizeof(struct twm_window));
}

void out_window(struct twm_window *window, unsigned char *str)
{
  if (window->y + window->acty > (window->y+window->heigth-20))
  {
/*     outs_l(window->x + window->length - 50, window->y+2, COL5, "<next page>");
     getch();
     outs_l(window->x + window->length - 50, window->y+2, COL2, "<next page>");*/
     draw_window(window);
  }

  if (window->style == 128)
    outs(window->x+5, window->y + window->acty, 0, str);
  else if (window->style == 64)
    outs(window->x+5, window->y + window->acty, 0, str);
  else
    outs(window->x+5, window->y + window->acty, WC_DT, str);

  window->acty +=10;
}

void out_window_col(struct twm_window *window, unsigned char *str, unsigned char col)
{
  if (window->y + window->acty > (window->y+window->heigth-20))
  {
/*     outs_l(window->x + window->length - 50, window->y+2, COL5, "<next page>");
     getch();
     outs_l(window->x + window->length - 50, window->y+2, COL2, "<next page>");*/
     draw_window(window);
  }

  if (window->style == 128)
    outs(window->x+5, window->y + window->acty, 0, str);
  else if (window->style == 64)
    outs(window->x+5, window->y + window->acty, 0, str);
  else
    outs(window->x+5, window->y + window->acty, col, str);

  window->acty +=10;
}

void gets_window(struct twm_window *window, unsigned char *str)
{
  unsigned char ch = 0;
  int i=0;

  while (ch!= 13)
    {
      line(window->actx + window->x, window->acty + window->y + 10, window->x+window->actx+ 8, window->y+window->acty + 10, WC_DT);
      ch = getch();
      line(window->actx + window->x, window->acty + window->y +10 , window->x+window->actx+ 8, window->y+window->acty +10, WC_DB);
      if (ch == 13)             /* ENTER - Key */
        {
          str[i] = 0;
        }
      else if (ch == 8)              /* Back */
        {
          if (i>0)
          {
          i--;
          str[i] = 0;
          box(window->x+window->actx-8, window->y+window->acty,window->x+window->actx, window->y+window->acty+8,WC_DB,WC_DB,WC_DB);
          window->actx-=8;
          }
        }
      else if (ch != 15)
        {
          outs(window->x+ window->actx, window->acty+ window->y, WC_DT, "%c",ch);
          window->actx+=8;
          str[i] = ch;
          i++;
        }
    }

  window->acty +=10;
  window->actx = 5;

}

void line_window(struct twm_window *window, int x1, int y1, int x2, int y2, int col)
{
  line(window->x+x1, window->y+y1, window->x+x2, window->y+y2, col);
}

void rect_window(struct twm_window *window, int x1, int y1, int x2, int y2, int col)
{
  rect(window->x+x1, window->y+y1, window->x+x2, window->y+y2, col);
}

void box_window(struct twm_window *window, int x1, int y1, int x2, int y2, int col1,int col2, int col3)
{
  box(window->x+x1, window->y+y1, window->x+x2, window->y+y2, col1,col2,col3);
}

void outs_l_window(struct twm_window *window, int x1, int y1, int col1, unsigned char *str)
{
  outs_l(window->x+x1, window->y+y1, col1,str);
}

void outxy_window(struct twm_window *window, int x, int y, int col, unsigned char *str)
{
  outs(window->x+x, window->y+y, col,str);
}

void start_menu(void)
{
  int x = 1;
  int y = MAXY-25;
  int length = MAXX-2;
  int heigth = 23;

  box(x, y, x+ length, y + heigth, LG,LG,LG);
  line(x,y,x+length,y,WHITE);
  line(x,y,x,y+ heigth,WHITE);

  line(x,y+ heigth, x+length,y+heigth,DG);
  line(x+length,y,x+length,y+ heigth,DG);

  x = 3;
  y = MAXY - 23;
  length = 70;
  heigth = 19;

  box(x, y, x+ length, y + heigth, LG,LG,LG);
  line(x,y,x+length,y,WHITE);
  line(x,y,x,y+ heigth,WHITE);

  line(x,y+ heigth, x+length,y+heigth,DG);
  line(x+length,y,x+length,y+ heigth,DG);

  outs_c(3,73, MAXY-16, BLACK, "Start");
}

void alert(char *text, ...)
{
  struct twm_window window;

  unsigned char buf[255];
  unsigned char str[255];
  va_list args;

  int i = 0;
  int j = 0;
  int longest = 0;
  int lines = 0;
  int x;
  int y;
  int length;
  int heigth;
  unsigned int flags;

//  save_flags(flags);
//  cli();

  va_start(args, text);
  vsprintf(buf,text,args);      // parse all to one line
  va_end(args);

  strcpy(window.title,"Alert");

  // line will looks like:
  // 'Alert in file dummy.c\nSystem halted.\n\0'

  // First, search longest line and count number of lines

  i = 0;
  j = 0;
  longest = 0;
  lines = 1;

  while (buf[i])
  {
    j++;
    if (buf[i] == '\n')
    {
      if (j>longest)
        longest = j;

      j = 0;
      lines++;
    }
    i++;
  }

  // Now we can calculate dimensions of the alert box and draw it
  length = longest * 8 + 30;
  x = (MAXX / 2) - (length / 2);

  heigth = lines * 8 + 60;
  y = (MAXY / 2) - (heigth / 2);

  window.x = x;
  window.y = y;
  window.length = length;
  window.heigth = heigth;
  window.style = 64;           // alert window

//  draw_window(&window);

  // Ok, now print the messages
  i = 0;
  j = 0;

  for (i=0;i<=strlen(buf);i++)
  {
    if ((buf[i] != '\n') && (buf[i] != '\0'))
    {
      str[j] = buf[i];
      j++;
    }
    else
    {
      str[j] = '\0';
//      out_window(&window, str);
      j = 0;
    }
  }
//  restore_flags(flags);

}

////// Common Controls
