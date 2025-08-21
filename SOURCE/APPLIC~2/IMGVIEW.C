/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\imgview.c
/* Last Update: 19.03.1999
/* Version    : alpha
/* Coded by   : Gregory Hayrapetian
/* Docus      :
/************************************************************************/

#include "newgui.h"
#include "controls.h"
#include "sched.h"
#include "imgdec.h"
#include "fcntl.h"


void imgview_redraw(window_t *win_imgview, TGAFile *tgafile, int loaded)
{
  int i,j;

  if (loaded)
  {
    for (j=0;j<tgafile->header.Height;j++)
      for (i=0;i<tgafile->header.Width;i++)
        win_draw_pixel(win_imgview,i,70+j,tgafile->source[(tgafile->header.Height-j)*tgafile->header.Width+i]);
  }
}

void imgview_task()
{
  struct ipc_message m;
  ctl_button_t *but1, *but2, *but3;
  ctl_input_t *input;
  char buf[256];
  window_t *win_imgview;
  int fd, loaded=0;
  TGAFile tgafile;

  win_imgview = create_window("Image viewer",300,100,400,300,WF_STANDARD);

  but1 = create_button(win_imgview,20,20,60,25,"Load",1);
  but2 = create_button(win_imgview,100,20,50,25,"Pal",2);
  but3 = create_button(win_imgview,170,20,50,25,"Std",3);
  input = create_input(win_imgview,250,20,100,20,4);

  fd = -1;
  tgafile.source = 0;

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m.MSGT_GUI_CTL_ID == 1)
            {
              int i,j;

              if (fd < 0)
                sys_close(fd);

              GetItemText_input(win_imgview, input, buf);
              show_msgf("Loading file: %s",buf);
              fd = sys_open(buf,O_RDONLY);
              show_msgf("fd: %d",fd);
              load_TGA(win_imgview, fd, &tgafile);
              loaded = 1;
              imgview_redraw(win_imgview,&tgafile,loaded);
            }
            if (m.MSGT_GUI_CTL_ID == 2)
              load_coltab(&tgafile.color_table);
            if (m.MSGT_GUI_CTL_ID == 3)
              std_pal();
            break;
       case MSG_GUI_WINDOW_CLOSE:
            sys_close(fd);
            destroy_app(win_imgview);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_imgview, WF_STOP_OUTPUT, 0);
            set_wflag(win_imgview, WF_MSG_GUI_REDRAW, 0);
            imgview_redraw(win_imgview,&tgafile,loaded);
            break;
     }

  }

}

void imgview_init_app()
{
//  if (!valid_app(win_imgview))
    CreateKernelTask(imgview_task,"imgview",NP,0);
}


