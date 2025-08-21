/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\console.c
/* Last Update: 04.12.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*  A standard console device driver. Functions like printch,.....
/************************************************************************/
#include "version.h"
#include "chars.h"
#include "twm.h"
#define CHAR_HEIGHT 8
#define CHAR_WIDTH  8

#define CHAR_BACK_COLOR 255

extern unsigned int MAXX;
extern unsigned int MAXY;
extern int syslog;
unsigned long video_mem_start=0;
unsigned char *pos=0;
unsigned int  curx = 0;
unsigned int  cury = 0;
int CHAR_COLOR = 0;

struct twm_window kmw={0};

int message_count=0;

#define MAX_LOG 200
unsigned char messages[MAX_LOG][100] = {0};

void screen_color(int col)
{
  unsigned char *dest;
  int i,j;

  dest = (unsigned char*)video_mem_start;

  for (i=MAXY/2;i>=0;i--)
  {
     line(0,i,MAXX,i,col);
     line(0,MAXY-i, MAXX, MAXY-i,col);
  }
}

void vga_console_outch(unsigned char ch, int x, int y, unsigned char vg, unsigned char hg)
{
   int i,k;

   unsigned char maske;

   if (hg==255)
   {
     for (i=0;i<8;i++)
     {
       maske = char_tab[ch*8+i];
       for (k=0; k<8;++k,maske <<=1)
         if (maske & 128)
            setpix(x+k,y+i,vg);
     }
    }
}

void console_printch(unsigned char ch)
{
  pos = (unsigned char*)video_mem_start + (cury*CHAR_HEIGHT*MAXX) + curx*CHAR_WIDTH;

  if (ch == '\n')               /* new line */
    {
      curx = 0;
      cury++;
    }
  else if (ch == '\b')               /* back */
    {
      curx--;
      *pos = ' ';
    }
  else
    {
      vga_console_outch(ch, curx * CHAR_WIDTH, cury * CHAR_HEIGHT, CHAR_COLOR, CHAR_BACK_COLOR);
      curx++;

      if (curx >= MAXX/CHAR_WIDTH)                   /* end of line reached... */
        {
         curx = 0;
         cury++;
        }
    }
   if (cury >= MAXY/CHAR_HEIGHT-CHAR_HEIGHT)
     {
      screen_color(0);
      cury=0;
     }
}

/*void console_gets(unsigned char *buffer)
{
  gets_window(&kmw, buffer);
} */

void console_setcolor(int color)
{
  CHAR_COLOR = color;
}

void console_clear(void)
{
   clear_window(&kmw);
   draw_window(&kmw);
}

int console_init()
{
   video_mem_start = 0xA0000;
   curx = 0;
   cury = 0;
}

int desktop_draw()
{
   char str[255];
   int i;

//   screen_color(1);
/*   screen_color(0);

   for (i=15;i<MAXX;i++)
   {
     line(i,0,i,MAXY,2);
     i+=15;
   }
   for (i=10;i<MAXY;i++)
   {
     line(0,i,MAXX,i,2);
     i+=10;
   }*/
   set_palette();
   draw_background(0,0,MAXX,MAXY);

   kmw.x = 20;
   kmw.y = 40;
   kmw.length = 600;
   kmw.heigth = 400;
   kmw.acty = 30;
   strcpy(kmw.title, "System Initialization...");
   draw_window(&kmw);
}

void console_init_finished(void)
{
  //open_window(&kmw);
//  syslog_init();

}


void console_print(char *string)
{
   int i;

/*   for (i=0;i<strlen(string);i++)
   {
      console_printch(string[i]);
   }*/
   if (syslog)
   {
      syslog_add(string);
   }
   else
   {
     out_window_col(&kmw, string, CHAR_COLOR);

     if (message_count < MAX_LOG)
     {
       strcpy(messages[message_count], string);
       message_count++;
     }
   }
}

void dump_messages(void)
{
   int i=0;

   while (i < message_count)
   {
     show_msg(messages[i]);
     i++;
   }
}

