/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\newgui.c
/* Last Update: 03.04.1999
/* Version    : alpha
/* Coded by   : Hayrapetian Gregory
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the Graphical User Interface
/************************************************************************/

#include "sched.h"
#include "newgui.h"
#include "controls.h"
#include "rtc.h"

extern struct task_struct *current;
extern window_t *win_console, *win_alert;
extern int move_count;

extern unsigned int MAXX, MAXY;

window_t *win_skypanel;
int next_aip;

ctl_button_t *skybut[MAX_WIN];


/* Window of Application */
window_t *win_app1, *win_app2;
char *str_app[20];

#define adjust_screen_x1(v)  if ((v) < screen_rect.x1) v = screen_rect.x1
#define adjust_screen_y1(v)  if ((v) < screen_rect.y1) v = screen_rect.y1
#define adjust_screen_x2(v)  if ((v) > screen_rect.x2) v = screen_rect.x2
#define adjust_screen_y2(v)  if ((v) > screen_rect.y2) v = screen_rect.y2

#define adjust_greater(v,av)	if (v > av) v = av;
#define adjust_smaller(v,av)	if (v < av) v = av


inline void set_rect(rect_t *rect, int x1, int y1, int x2, int y2)
{
  rect->x1 = x1;
  rect->y1 = y1;
  rect->x2 = x2;
  rect->y2 = y2;
}

inline void set_screen_rect(rect_t *rect, int x1, int y1, int x2, int y2)
{
  adjust_smaller(x1,screen_rect.x1);
  adjust_smaller(x2,screen_rect.x1);
  adjust_smaller(y1,screen_rect.y1);
  adjust_smaller(y2,screen_rect.y1);

  adjust_greater(x1,screen_rect.x2);
  adjust_greater(x2,screen_rect.x2);
  adjust_greater(y1,screen_rect.y2);
  adjust_greater(y2,screen_rect.y2);

  set_rect(rect,x1,y1,x2,y2);
}

inline int fit_rect(rect_t *fitrect, rect_t *source)
{
  if ((fitrect->x1 > source->x2) || (fitrect->y1 > source->y2) ||
      (fitrect->x2 < source->x1) || (fitrect->y2 < source->y1))
    return 1;

  adjust_smaller(fitrect->x1,source->x1);
  adjust_smaller(fitrect->y1,source->y1);
  adjust_greater(fitrect->x2,source->x2);
  adjust_greater(fitrect->y2,source->y2);

  return 0;
}

inline void enlarge_rect(rect_t *rect, int x1, int y1, int x2, int y2)
{
  rect->x1 += x1;
  rect->y1 += y1;
  rect->x2 += x2;
  rect->y2 += y2;
}

/************************************************************************/
/* This function generates absolute coordinates of a window-rect
/************************************************************************/
inline void align_rect(rect_t *rect, window_t *win)
{
  rect->x1 += win->client_rect.x1;
  rect->y1 += win->client_rect.y1;
  rect->x2 += win->client_rect.x1;
  rect->y2 += win->client_rect.y1;
}


int check_win_face(int x, int y)
{
  int i;

  for (i=znum-1;i>=0;i--)
//    if ((win_list[i] != NULL) && !get_wflag(win_list[i],WF_FREEZED))
    if (win_list[i] != NULL)
    {
      if (pt_inrect(&win_list[i]->rect,x,y))
        return i+1;
    }

  return 0;
}

int check_win_title(int x, int y)
{
  rect_t rect = {win_list[znum-1]->rect.x1+3,win_list[znum-1]->rect.y1+3,win_list[znum-1]->rect.x2-14,win_list[znum-1]->rect.y1+15};

  return pt_inrect(&rect,x,y);
}

int check_win_butclose(int x, int y)
{
  rect_t rect = {win_list[znum-1]->rect.x2-13,win_list[znum-1]->rect.y1+3,win_list[znum-1]->rect.x2-3,win_list[znum-1]->rect.y1+15};

  return pt_inrect(&rect,x,y);
}


int pt_inrect(rect_t *rect, int x, int y)
{
  return !((x < rect->x1) || (x > rect->x2) || (y < rect->y1) || (y > rect->y2));
}

int pt_inwin(window_t *win, int x, int y)
{
  rect_t rect = {win->client_rect.x1,win->client_rect.y1,win->client_rect.x2,win->client_rect.y2};

  return pt_inrect(&rect,x,y);
}

int rect_inrect(rect_t *b1, rect_t *b2)
{
  return !((b1->x2 < b2->x1) || (b1->x1 > b2->x2) || (b1->y2 < b2->y1) || (b1->y1 > b2->y2));
}

int mouse_inframe(window_t *win, int x, int y)
{
  rect_t rect1 = {win->rect.x2-10,win->rect.y2-3,win->rect.x2,win->rect.y2};
  rect_t rect2 = {win->rect.x2-3,win->rect.y2-10,win->rect.x2,win->rect.y2};

  return (pt_inrect(&rect1,x,y) || pt_inrect(&rect2,x,y));
}

int rect_displayed(rect_t *rect, window_t *win)
{
  int i,j;

  if (fit_rect(rect,&win->rect))
    return;


  return 0;
}


void pass_msg(window_t *win,int msg_type)
{
  struct ipc_message m;
  if (win->ptask == NULL)
    return;

  if (msg_type == MSG_GUI_REDRAW)
  {
    if (get_wflag(win, WF_MSG_GUI_REDRAW))
      return;
    set_wflag(win, WF_MSG_GUI_REDRAW, 1);
  }

  m.type = msg_type;
  send_msg(win->ptask->pid, &m);
}

void spass_msg(window_t *win, struct ipc_message *m)
{
  if (win->ptask == NULL)
    return;

  send_msg(win->ptask->pid, m);
}


void reply_close(window_t *win)
{
  struct ipc_message m;

  m.type = MSG_GUI_WINDOW_CLOSE_REPLY;
  send_msg(newgui_pid, &m);
}

void close_task()
{
  struct ipc_message m;
  int tcount=0;

  settimer(5);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_TIMER:
            if (tcount++ > 4)
            {
              if (win_list[znum-1] == close_win)
              {
                window_move = 0;
                window_size = 0;
                window_ctl = NULL;
                change_cmask(1);
              }

              cleartimer_pid(close_pid);
              destroy_app(close_win);
              close_win = NULL;
              close_pid = 0;
              vfree(m);

              exit(0);
            }
            break;
     }
  }
}


void pass_active_win()
{
  int wnr;
  ctl_menu_t *menu;
  rect_t trect;

  if (window_move)
    return;

  if ((win_list[znum-1]->focus != NULL) && (win_list[znum-1]->focus->ctl_type == CTL_MENU))
  {
/*    menu = window_ctl->ctl_ptr;

    hide_cursor();
    restore_menufield(win_list[znum-1],menu,menu->menutitle);
    show_cursor();

    menu->old_item = menu->active_item;
    menu->active_item = NULL;
    menu->menutitle = NULL;

    hide_cursor();
    draw_menu(&screen_rect,win_list[znum-1],menu,1);
    show_cursor();

    win_list[znum-1]->menu_active = 0;*/
    printk("closeing window");
    close_menu(win_list[znum-1]->focus->ctl_ptr);
  }

  window_ctl = NULL;

  wnr = 0;
  wnr = shift_z(wnr);

  if ((win_list[wnr-1] == close_win) || (znum < 1))
    return;

  hide_cursor();
  if (znum > 1)
  {
    set_screen_rect(&trect,win_list[znum-2]->rect.x1,win_list[znum-2]->rect.y1,win_list[znum-2]->rect.x2,win_list[znum-2]->rect.y1+15);
    draw_win_c(&trect, win_list[znum-2]);

    pass_msg(win_list[znum-2],MSG_GUI_REDRAW);
    pass_msg(win_list[znum-2],MSG_GUI_WINDOW_FOCUS_LOST);
  }
  set_screen_rect(&trect,win_list[wnr-1]->rect.x1,win_list[wnr-1]->rect.y1,win_list[wnr-1]->rect.x2,win_list[wnr-1]->rect.y1+15);
  draw_win_c(&trect, win_list[wnr-1]);

  redraw_rect_win(win_list[wnr-1]->rect.x1,win_list[wnr-1]->rect.y1,win_list[wnr-1]->rect.x2,win_list[wnr-1]->rect.y2,0);
  show_cursor();


//  clear_winmask(win_list[wnr-1]);
//  mask_win(win_list[wnr-1]->rect.x1,win_list[wnr-1]->rect.y1,win_list[wnr-1]->rect.x2,win_list[wnr-1]->rect.y2,2);

  mask_screen();

  pass_msg(win_list[wnr-1],MSG_GUI_REDRAW);
  pass_msg(win_list[wnr-1],MSG_GUI_WINDOW_FOCUS_GET);
}

void close_active_win()
{
//  if (win_list[znum-1] == win_console)
//    return;

  pass_msg(win_list[znum-1],MSG_GUI_WINDOW_CLOSE);

//  close_win = win_list[znum-1];
//  close_pid = ((struct task_struct*)CreateKernelTask((unsigned int)close_task, "gui_close", NP,0))->pid;
}

void redraw_all()
{
  struct ipc_message m;

  m.type = MSG_GUI_REDRAW_ALL;
  send_msg(newgui_pid, &m);
}

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

int transform_mouse(window_t *win, int *x, int *y)
{
  if (!pt_inrect(&win->client_rect, *x, *y))
    return 0;

  *x = *x - win->client_rect.x1;
  *y = *y - win->client_rect.y1;

  return 1;
}

void move_window(int dx, int dy)
{
  if (fullmove)
    move_window_full(dx,dy);
  else
    move_window_rect(dx,dy);
}

void window_fullmove(int value)
{
  fullmove = value;
}


void window_switch(int wnr)
{
  struct ipc_message m;

  m.type = MSG_GUI_WINDOW_SWITCH;
  m.MSGT_GUI_WINDOW = wnr;
  send_msg(newgui_pid, &m);
}

void gui_window_switch(int wnr)
{
    rect_t trect;

    shift_z(wnr-1);

    hide_cursor();
    if (znum > 1)
    {
      set_screen_rect(&trect,win_list[znum-2]->rect.x1,win_list[znum-2]->rect.y1,win_list[znum-2]->rect.x2,win_list[znum-2]->rect.y1+15);
      draw_win_c(&trect, win_list[znum-2]);

      pass_msg(win_list[znum-2],MSG_GUI_REDRAW);
      pass_msg(win_list[znum-2],MSG_GUI_WINDOW_FOCUS_LOST);
    }
    set_screen_rect(&trect,win_list[znum-1]->rect.x1,win_list[znum-1]->rect.y1,win_list[znum-1]->rect.x2,win_list[znum-1]->rect.y1+15);
    draw_win_c(&trect, win_list[znum-1]);

    redraw_rect_win(win_list[znum-1]->rect.x1,win_list[znum-1]->rect.y1,win_list[znum-1]->rect.x2,win_list[znum-1]->rect.y2,wnr-1);
    show_cursor();

//    clear_winmask(win_list[znum-1]);
    mask_screen();

    pass_msg(win_list[znum-1],MSG_GUI_REDRAW);
    pass_msg(win_list[znum-1],MSG_GUI_WINDOW_FOCUS_GET);
}

void perform_but1_pressed(int x, int y)
{
  int wnr, cid;

  if ((wnr = check_win_face(x,y)) && (wnr != znum))
  {
    gui_window_switch(wnr);
  }

  if (check_win_title(x, y) && get_wflag(win_list[znum-1],WF_STATE_MOVEABLE) && get_wflag(win_list[znum-1],WF_STYLE_TITLE))
  {
    change_cmask(3);
    window_move = znum;

    window_move = shift_z(window_move-1);
    move_rect = win_list[window_move-1]->rect;

    if (!fullmove)
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

      show_cursor();
    }

  }

  if (check_win_butclose(x, y) && (close_pid == 0) && get_wflag(win_list[znum-1],WF_STATE_CLOSEABLE) && get_wflag(win_list[znum-1],WF_STYLE_BUTCLOSE))
  {

    pass_msg(win_list[znum-1],MSG_GUI_WINDOW_CLOSE);
//    set_wflag(win_list[znum-1],WF_FREEZED,1);

//    close_win = win_list[znum-1];
//    close_pid = ((struct task_struct*)CreateKernelTask((unsigned int)close_task, "gui_close", NP,0))->pid;

    return;
  }

  if (mouse_inframe(win_list[znum-1], x, y))
    window_size = 1;

  if (win_list[znum-1]->controls && pt_inwin(win_list[znum-1],x,y))
  {
    window_ctl = check_control(x,y);
  }

}

void perform_but1_released()
{

  if (window_ctl != NULL)
    perform_control();

  if (!fullmove && window_move)
  {
    int dx,dy;

    redraw_rect(move_rect.x1,move_rect.y1,move_rect.x2,move_rect.y1+2,1);
    redraw_rect(move_rect.x1,move_rect.y2-2,move_rect.x2,move_rect.y2,1);
    redraw_rect(move_rect.x1,move_rect.y1+3,move_rect.x1+2,move_rect.y2-3,1);
    redraw_rect(move_rect.x2-2,move_rect.y1+3,move_rect.x2,move_rect.y2-3,1);

    dx = move_rect.x1 - win_list[window_move-1]->rect.x1;
    dy = move_rect.y1 - win_list[window_move-1]->rect.y1;

    move_window_full(dx,dy);
  }

  window_move = 0;
  window_size = 0;
  window_ctl = NULL;
  change_cmask(1);
}

void perform_move(int mx, int my, int ox, int oy)
{
  int dx, dy;

  dx = mx - ox;
  dy = my - oy;

  if (window_size <= 0)
  {
    if (mouse_inframe(win_list[znum-1],mx, my))
	{
      change_cmask(2);
	  window_size = -1;
	}
//    else if (window_move)
//      change_cmask(3);
    else if (window_size < 0)
	{
      change_cmask(1);
	  window_size = 0;
	}
  }

  if (window_move)
    move_window(dx,dy);

  if (window_size > 0)
    size_win(dx,dy,mx,my);

  if (window_ctl)
    handle_control(mx,my);

}

void perform_keypressed(char ch, unsigned int code, unsigned int shift)
{
  int focus = 0;
  control_t *c,*c2;
  ctl_menu_t *m;
  int i;
  window_t *w;

  if (code == 56)       // ALT pressed
  {
    w = win_list[znum-1];
    c = w->controls;
    c2 = w->controls;

    while ((c) && (c->ctl_type != CTL_MENU))
    {
       c=c->next;
       if (c2 == c) break;
    }

    if (c->ctl_type == CTL_MENU)    // if window has menu draw/open it
    {
      draw_control(&screen_rect, win_list[znum-1], win_list[znum-1]->focus, 0);

      window_ctl = win_list[znum-1]->focus = c;
      keypressed_menu(c, 56, 56);
    }
  }

  else
  switch (ch)
  {
    case KEY_TAB:  pass_active_control(win_list[znum-1], shift);
				   window_ctl = win_list[znum-1]->focus;
				   break;
    default:      if (win_list[znum-1]->focus)
                      keypressed_control(win_list[znum-1]->focus, ch,code);
    }
}

void startup_task()
{
  window_t *win_startup;


  skypanel_init();

//  win_startup = create_window("SkyOS Startup",MAXX/2 - 175,MAXY/2-100,400,200,
//                    WF_STANDARD & ~WF_STYLE_BUTCLOSE & ~WF_STATE_MOVEABLE & ~WF_STATE_SIZEABLE);

  console_init_app();
  msg_init_app();
  quick_init_app();
  msgbox_init_app();
//  app_videocon();
//  sys_mount("hd0a","fat");
//  mapfile_init_app();

  //destroy_app(win_startup);
  while(1);

}


int perform_control_keys(int code, int shift, int alt, int ctrl)
{
  if ((code == 15) && alt)			/* ALT TAB */
  {
    pass_active_win();
	return 1;
  }
  else if ((code == 62) && alt)		/* ALT F4 */
  {
    close_active_win();
    return 2;
  }
  else if ((code == 46) && alt)		/* ALT C */
  {
    CreateKernelTask((unsigned int)startup_task, "startup", NP,0);
	return 3;
  }
  else if (code == 69)				/* Pause */
  {
	if (task_in_critical_section(&csect_gui, win_list[znum-1]->ptask->pid))
	  leave_critical_section(&csect_gui);

	if (win_list[znum-1]->ptask->state == TASK_SLEEPING)
	{
	  win_list[znum-1]->flags &= ~WF_STOPPED;
      task_control(win_list[znum-1]->ptask->pid,TASK_READY);
	}
	else
	{
	  win_list[znum-1]->flags |= WF_STOPPED;
      task_control(win_list[znum-1]->ptask->pid,TASK_SLEEPING);
	}

    redraw_rect(win_list[znum-1]->rect.x1,win_list[znum-1]->rect.y1,win_list[znum-1]->rect.x2,win_list[znum-1]->rect.y1+25,1);
	return 4;
  }

  return 0;
}

void newgui_task()
{
  struct ipc_message m;
  int mouse_x, mouse_y;
  int old_x, old_y;
  int i;
  window_t *win;
  ctl_button_t *ctl;

  char strm[50];

  alert_init();

  CreateKernelTask((unsigned int)startup_task, "startup", NP,0);

  mouse_x = 200;
  mouse_y = 200;
  old_x = 0;
  old_y = 0;

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
/*
       case MSG_GUI_CREATE_BUTTON:
            ctl = cm_create_button(m->MSGT_GUI_WINDOW, m->MSGT_GUI_X,m->MSGT_GUI_Y,m->MSGT_GUI_LENGTH,m->MSGT_GUI_WIDTH,m->MSGT_GUI_WNAME,m->MSGT_GUI_CID);
            m->MSGT_GUI_PCTL = ctl;
            send_msg(m->source->pid,m);
            break;
*/
       case MSG_GUI_WINDOW_SWITCH:
	    gui_window_switch(m.MSGT_GUI_WINDOW);
            break;
       case MSG_GUI_WINDOW_CLOSE_REPLY:
            if (close_pid > 0)
            {
              cleartimer_pid(close_pid);
              task_control(close_pid, TASK_DEAD);
              close_win = NULL;
              close_pid = 0;
            }
            break;
       case MSG_GUI_REDRAW_WINDOW:
		    win = m.MSGT_GUI_WINADR;

            hide_cursor();
//            draw_win_c(&screen_rect,m.MSGT_GUI_WINADR);
			redraw_rect(win->rect.x1,win->rect.y1,win->rect.x2,win->rect.y2,1);
            show_cursor();
            break;
       case MSG_GUI_REDRAW_ALL:
            hide_cursor();
            redraw_rect(screen_rect.x1,screen_rect.y1,screen_rect.x2,screen_rect.y2,1);
            show_cursor();
            break;
       case MSG_GUI_CREATE_WINDOW:
		    win = gui_create_window(m.MSGT_GUI_TASK, m.MSGT_GUI_WNAME, m.MSGT_GUI_X, m.MSGT_GUI_Y, m.MSGT_GUI_X+m.MSGT_GUI_LENGTH, m.MSGT_GUI_Y+m.MSGT_GUI_WIDTH, m.MSGT_GUI_FLAGS);
            m.MSGT_GUI_WINDOW = win;
            send_msg(m.source->pid,&m);
            break;
       case MSG_GUI_CLOSE_WIN:
            gui_destroy_window(m.MSGT_GUI_WINDOW);
            break;
       case MSG_MOUSE_MOVE:
//	    sprintf(strm,"mc: %d %d %d ",move_count,window_move,window_size);
//            couts(&screen_rect,MAXX-80,MAXY-15,strm,COLOR_BLACK,COLOR_LIGHTGRAY);

            if ((--move_count > 1) && (window_move || (window_size > 0)))
              break;

            enter_critical_section(&csect_gui);

            old_x = mouse_x;
            old_y = mouse_y;
            mouse_x = m.MSGT_MOUSE_X;
            mouse_y = m.MSGT_MOUSE_Y;

            perform_move(mouse_x,mouse_y,old_x,old_y);

            leave_critical_section(&csect_gui);

            if (transform_mouse(win_list[znum-1],&m.MSGT_MOUSE_X,&m.MSGT_MOUSE_Y))
   	      spass_msg(win_list[znum-1],&m);
            break;
       case MSG_MOUSE_BUT1_PRESSED:
            perform_but1_pressed(mouse_x,mouse_y);
            if (transform_mouse(win_list[znum-1],&m.MSGT_MOUSE_X,&m.MSGT_MOUSE_Y))
              spass_msg(win_list[znum-1],&m);
            break;
       case MSG_MOUSE_BUT1_RELEASED:
            perform_but1_released();
            if (transform_mouse(win_list[znum-1],&m.MSGT_MOUSE_X,&m.MSGT_MOUSE_Y))
              spass_msg(win_list[znum-1],&m);
            break;
       case MSG_MOUSE_BUT2_PRESSED:
            if (transform_mouse(win_list[znum-1],&m.MSGT_MOUSE_X,&m.MSGT_MOUSE_Y))
              spass_msg(win_list[znum-1],&m);
            break;
       case MSG_MOUSE_BUT2_RELEASED:
            if (transform_mouse(win_list[znum-1],&m.MSGT_MOUSE_X,&m.MSGT_MOUSE_Y))
              spass_msg(win_list[znum-1],&m);
            break;
       case MSG_READ_CHAR_REPLY:
//            show_msgf("key: %d %d",m.MSGT_READ_CHAR_CHAR, m.u.lpara1);
            if (!perform_control_keys(m.u.lpara1,m.u.lpara2,m.u.lpara3,m.u.lpara4))
            {
              perform_keypressed(m.MSGT_READ_CHAR_CHAR,m.u.lpara1, m.u.lpara2);
              spass_msg(win_list[znum-1],&m);
            }
            break;
     }
  }

}

int get_wnr(int wid)
{
  int i;

  for (i=0;i<znum;i++)
    if (win_list[i]->id == wid)
      return i;

  return -1;
}

window_t *get_win(int wid)
{
  int i;

  for (i=0;i<znum;i++)
    if (win_list[i]->id == wid)
      return win_list[i];

  return NULL;
}

void show_applist()
{
  int i;

  show_msgf(" ");
  show_msgf(" WID  Flags       Task        Window name");

  for (i=0;i<znum;i++)
    show_msgf("%4d  0x%08x  %-10s  %s",win_list[i]->id,win_list[i]->flags,win_list[i]->ptask->name,win_list[i]->name);

  show_msgf(" ");
}

int get_cid(void *cptr)
{
  control_t *ctl = win_skypanel->controls;

  while (ctl)
  {
    if (ctl->ctl_ptr == cptr)
	  return ctl->ctl_id;

    ctl = ctl->next;
  }

  return 0;
}

void skypanel_add_app(window_t *win)
{
  struct ipc_message m;

  m.type = MSG_SKYPANEL_ADD_APP;
  m.MSGT_GUI_WINADR = win;

  send_msg(win_skypanel->ptask->pid, &m);
}

void skypanel_del_app(int wid)
{
  struct ipc_message m;

  m.type = MSG_SKYPANEL_DEL_APP;
  m.MSGT_GUI_WID = wid;

  send_msg(win_skypanel->ptask->pid, &m);
}

void gui_skypanel_add_app(window_t *win)
{
  char str[256];

  strcpy(str,win->name);
  str[7] = 0;

  skybut[next_aip] = create_button(win_skypanel,80+(next_aip-2)*100,3,90,20,str,win->id);
  next_aip++;
}

void gui_skypanel_del_app(int wid)
{
  int i;
  control_t *ctl = win_skypanel->controls;
  ctl_button_t *but, *butn;

  i = 0;

  while (i < next_aip)
  {
    if (get_cid(skybut[i]) == wid) break;
    i++;
  }

  if (i == next_aip) return;

  while (ctl)
  {
    if (ctl->ctl_ptr == skybut[i])
      break;

    ctl = ctl->next;
  }

  while (ctl->next)
  {
    but = ctl->ctl_ptr;
    butn = ctl->next->ctl_ptr;

    ctl->ctl_id = ctl->next->ctl_id;

    strcpy(but->name, butn->name);
    ctl = ctl->next;
  }

  next_aip--;
  destroy_control(win_skypanel, skybut[next_aip], 1);
}

void skypanel_task()
{
  struct ipc_message m;
  int wnr;

  win_skypanel = create_window("SKY", screen_rect.x1, screen_rect.y2-30, screen_rect.x2, 30, WF_STYLE_FACE | WF_STYLE_FRAME | WF_STYLE_DONT_SHOW); 

  skybut[0] = create_button(win_skypanel,10,3,40,20,"Sky",1000);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m.MSGT_GUI_CTL_ID != 1000)
            {
              wnr = get_wnr(m.MSGT_GUI_CTL_ID);

              window_switch(wnr+1);
            }
			else 
			  quick_init_app();
            break;
       case MSG_SKYPANEL_ADD_APP:
	    gui_skypanel_add_app(m.MSGT_GUI_WINADR);
            break;
       case MSG_SKYPANEL_DEL_APP:
	    gui_skypanel_del_app(m.MSGT_GUI_WID);
            break;
     }
  }
}

void newgui_update()
{
  set_rect(&screen_rect, 0, 0, MAXX - 1, MAXY - 1);
  skypanel_update();

  redraw_all();
}

void skypanel_update()
{
  if (valid_app(win_skypanel))
  {
    set_rect(&win_skypanel->rect,screen_rect.x1, screen_rect.y2-30, screen_rect.x2, screen_rect.y2);
    set_rect(&win_skypanel->client_rect,screen_rect.x1+3, screen_rect.y2-27, screen_rect.x2-3, screen_rect.y2-3);
  }
}

void skypanel_init()
{
  if (!valid_app(win_skypanel))
  {
    next_aip = 2;
	CreateKernelTask(skypanel_task, "skypanel", NP,0); 
  }
}

void newgui_init()
{
  int i;

  newgui_pid = 0;
  window_move = 0;
  window_size = 0;
  window_ctl = NULL;
  znum = 0;
  next_wid = 1;
  fullmove = 1;

  set_rect(&screen_rect, 0, 0, MAXX - 1, MAXY - 1);

  move_count = 0;
  close_pid = 0;

  create_critical_section(&csect_gui);

  for (i=0;i<MAX_WIN;i++)
    win_list[i] = NULL;

  /* High priority */
  newgui_pid = ((struct task_struct*)CreateKernelTask((unsigned int)newgui_task, "new_gui", HP,0))->pid;

  hide_cursor();
  draw_background(screen_rect.x1,screen_rect.y1,screen_rect.x2,screen_rect.y2,0);
  show_cursor();
}


void app_test()
{
  window_t *win_app;
  struct ipc_message *m;
  char buf[256];
  int i;
  ctl_button_t *but1, *but2, *but3;
  ctl_input_t *input;
  ctl_menu_t *menu;
  ctl_text_t *text;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_app = create_window("Test Application",100,200,300,200,WF_STANDARD);

  /* !!! id should be > 0  !!! */
  but1 = create_button(win_app,50,100,100,25,"Add menu",1);
  but2 = create_button(win_app,180,100,100,25,"Delete But1",2);
  but3 = create_button(win_app,180,135,100,25,"Create But1",3);
  input = create_input(win_app,120,50,100,20,4);
  text = create_text(win_app,40,50,100,25,"Menuitem:",TEXT_ALIGN_LEFT,100);

  menu = create_menu(win_app,4);
  add_menuitem(win_app, menu, "File", 5, 0);
  add_menuitem(win_app, menu, "Edit", 6, 0);
  add_menuitem(win_app, menu, "Help", 7, 0);
  add_menuitem(win_app, menu, "Exit", 8, 0);

  add_menuitem(win_app, menu, "New", 9, 5);
  add_menuitem(win_app, menu, "Open", 10, 5);
  add_menuitem(win_app, menu, "Save", 11, 5);
  add_menuitem(win_app, menu, "Delete", 12, 5);

  add_menuitem(win_app, menu, "Copy", 13, 6);
  add_menuitem(win_app, menu, "Paste", 14, 6);

  add_menuitem(win_app, menu, "Help on Help", 15, 7);
  add_menuitem(win_app, menu, "Index", 16, 7);
  add_menuitem(win_app, menu, "Test", 17, 7);
  add_menuitem(win_app, menu, "Help me", 18, 7);

  add_menuitem(win_app, menu, "Exit now", 19, 8);
  add_menuitem(win_app, menu, "Exit 1", 20, 8);
  add_menuitem(win_app, menu, "Exit 2", 21, 8);
  add_menuitem(win_app, menu, "Exit 3", 22, 8);
  add_menuitem(win_app, menu, "Exit 4", 23, 8);
  add_menuitem(win_app, menu, "Exit 5", 24, 8);
  add_menuitem(win_app, menu, "Exit 6", 25, 8);
  add_menuitem(win_app, menu, "Exit 7", 26, 8);
  add_menuitem(win_app, menu, "Exit 8", 27, 8);
  add_menuitem(win_app, menu, "Exit 9", 28, 8);
  add_menuitem(win_app, menu, "Exit 10", 29, 8);
  add_menuitem(win_app, menu, "Exit 11", 30, 8);
  i = 31;

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_WINDOW_CLOSE:

//            for (i=0;i<10000;i++)
//              i = i;

            destroy_app(win_app);
            break;
       case MSG_GUI_CTL_MENU_ITEM:
            if (m->MSGT_GUI_CTL_ID > 0)
              show_msgf("Menu item %d selected",m->MSGT_GUI_CTL_ID);
            break;
       case MSG_GUI_CTL_BUTTON_PRESSED:
            show_msgf("Button %d pressed",m->MSGT_GUI_CTL_ID);
            if (m->MSGT_GUI_CTL_ID == 1)
            {
              GetItemText_input(win_app, input, buf);
              add_menuitem(win_app, menu, buf, i++, 0);
            }
            if ((m->MSGT_GUI_CTL_ID == 2) && (but1))
            {
              destroy_control(win_app,but1,1);
              but1 = 0;
            }
            if ((m->MSGT_GUI_CTL_ID == 3) && (!but1))
            {
              but1 = create_button(win_app,50,100,100,25,"Add menu",1);
            }
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_app, WF_STOP_OUTPUT, 0);
            set_wflag(win_app, WF_MSG_GUI_REDRAW, 0);
//            SetItemText_input(win_app,input,"test");
            break;
     }

  }

}

/*
void app_test1()
{
  struct ipc_message *m;
  char buf[256], str[50];
  int i;
  ctl_input_t *ipt_red, *ipt_green, *ipt_blue;
  int red, green, blue;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  create_text(win_app,30,30,70,25,"Red:",TEXT_ALIGN_LEFT,1);
  create_text(win_app,30,70,70,25,"Green:",TEXT_ALIGN_LEFT,2);
  create_text(win_app,30,110,70,25,"Blue:",TEXT_ALIGN_LEFT,3);

  ipt_red = create_input(win_app,120,30,70,20,10);
  ipt_green = create_input(win_app,120,70,70,20,11);
  ipt_blue = create_input(win_app,120,110,70,20,12);

  create_button(win_app,210,30,25,25,"+",20);
  create_button(win_app,250,30,25,25,"-",21);
  create_button(win_app,210,70,25,25,"+",22);
  create_button(win_app,250,70,25,25,"-",23);
  create_button(win_app,210,110,25,25,"+",24);
  create_button(win_app,250,110,25,25,"-",25);

  red = 0;
  green = 0;
  blue = 0;

  i = 10;

  while (1)
  {

//    for (i=10;i<600;i++)
//      win_draw_fill_rect(win_app,10,10,i,i,32+(i%64));


//    sleep(10);


     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            switch (m->MSGT_GUI_CTL_ID)
            {
              case 20: red++;
                       sprintf(str,"%d",red);
                       SetItemText_input(win_app,ipt_red,str);
                       break;
              case 21: red--;
                       sprintf(str,"%d",red);
                       SetItemText_input(win_app,ipt_red,str);
                       break;
              case 22: green++;
                       sprintf(str,"%d",green);
                       SetItemText_input(win_app,ipt_green,str);
                       break;
              case 23: green--;
                       sprintf(str,"%d",green);
                       SetItemText_input(win_app,ipt_green,str);
                       break;
              case 24: blue++;
                       sprintf(str,"%d",blue);
                       SetItemText_input(win_app,ipt_blue,str);
                       break;
              case 25: blue--;
                       sprintf(str,"%d",blue);
                       SetItemText_input(win_app,ipt_blue,str);
                       break;
            }
            ChangeColorReg(13,red,green,blue);
            break;
       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_app);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_app, WF_STOP_OUTPUT, 0);
            set_wflag(win_app, WF_MSG_GUI_REDRAW, 0);
            win_draw_fill_rect(win_app,300,30,400,130,13);
            break;
     }
  }

}
*/

void app_test2()
{
  struct ipc_message *m;
  int start=0, count=0, etime;
  char str[50];
  struct time t, t2;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_app1 = create_window("Test Application 1",100,200,200,200,WF_STANDARD);

  while (!start)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_READ_CHAR_REPLY:
            m->type = 0x3f3f;
            send_msg(win_app2->ptask->pid, m);
            start = 1;
            break;
     }
  }

  get_time(&t);

  while (1)
  {
    wait_msg(m, 0x3f3f);

    m->type = 0x3f3f;
    send_msg(win_app2->ptask->pid, m);

    sprintf(str,"%d",count++);
    win_outs(win_app1,40,70,str,COLOR_BLACK,COLOR_LIGHTGRAY);

	if (count == 3000)
	  break;
  }

  get_time(&t2);
  etime = (t2.min - t.min)*60 + (t2.sec - t.sec);

  redraw_window(win_app1);
  sprintf(str,"%d",etime);
  win_outs(win_app1,40,70,str,COLOR_BLACK,COLOR_LIGHTGRAY);

  set_wflag(win_app1, WF_MSG_GUI_REDRAW, 0);

  while (1)
  {
    wait_msg(m, -1);

    switch (m->type)
    {
      case MSG_GUI_WINDOW_CLOSE:
           destroy_app(win_app1);
           break;
      case MSG_GUI_REDRAW:
           set_wflag(win_app1, WF_STOP_OUTPUT, 0);
           set_wflag(win_app1, WF_MSG_GUI_REDRAW, 0);
           win_outs(win_app1,40,70,str,COLOR_BLACK,COLOR_LIGHTGRAY);
           break;
    }
  }

}

void app_test3()
{
  struct ipc_message *m;
  int count=0, etime;
  char str[50];
  struct time t, t2;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_app2 = create_window("Test Application 2",300,200,200,200,WF_STANDARD);

  get_time(&t);

  while (1)
  {
    wait_msg(m, 0x3f3f);

    m->type = 0x3f3f;
    send_msg(win_app1->ptask->pid, m);

    sprintf(str,"%d",count++);
    win_outs(win_app2,40,70,str,COLOR_BLACK,COLOR_LIGHTGRAY);

	if (count == 3000)
	  break;
  }

  get_time(&t2);
  etime = (t2.min - t.min)*60 + (t2.sec - t.sec);

  redraw_window(win_app2);
  sprintf(str,"%d",etime);
  win_outs(win_app2,40,70,str,COLOR_BLACK,COLOR_LIGHTGRAY);

  while (1)
  {
    wait_msg(m, -1);

    switch (m->type)
    {
      case MSG_GUI_WINDOW_CLOSE:
           destroy_app(win_app2);
           break;
      case MSG_GUI_REDRAW:
           set_wflag(win_app2, WF_STOP_OUTPUT, 0);
           set_wflag(win_app2, WF_MSG_GUI_REDRAW, 0);
           win_outs(win_app2,40,70,str,COLOR_BLACK,COLOR_LIGHTGRAY);
           break;
    }
  }

}

void testapp()
{
//  if (!valid_app(win_app))
    CreateKernelTask(app_test,"tapp",NP,0);
}

void testapp2()
{
  if (!valid_app(win_app1))
    CreateKernelTask(app_test2,"tapp1",NP,0);

  if (!valid_app(win_app2))
    CreateKernelTask(app_test3,"tapp2",NP,0);
}
