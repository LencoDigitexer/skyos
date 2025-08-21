/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\newgui.h
/* Last Update: 21.12.1998
/* Version    : alpha
/* Coded by   : Hayrapetian Gregory
/* Docus      :
/************************************************************************/

#include "critical.h"

#define COLOR_BLACK        0
#define COLOR_BLUE         1
#define COLOR_GREEN        2
#define COLOR_CYAN         3
#define COLOR_RED          4
#define COLOR_MAGENTA      5
#define COLOR_BROWN        6
#define COLOR_LIGHTGRAY    7
#define COLOR_GRAY         8
#define COLOR_LIGHTBLUE    9
#define COLOR_LIGHTGREEN   10
#define COLOR_LIGHTCYAN    11
#define COLOR_LIGHTRED     12
#define COLOR_LIGHTMAGENTA 13
#define COLOR_YELLOW       14
#define COLOR_WHITE        15

#define KEY_ENTER		   13
#define KEY_TAB			   9
#define KEY_BACKSPACE	   8

#define MAX_WIN  1000


/* Window Flags */
#define WF_MSG_GUI_REDRAW		0x00000001
#define WF_STOP_OUTPUT			0x00000002
#define WF_ALERT				0x00000004
#define WF_STOPPED				0x00000008

#define WF_STYLE_FACE			0x00000100
#define WF_STYLE_FRAME			0x00000200
#define WF_STYLE_TITLE			0x00000400
#define WF_STYLE_BUTCLOSE		0x00000800
#define WF_STYLE_DONT_SHOW		0x00001000

#define WF_STATE_CLOSEABLE		0x00010000
#define WF_STATE_MOVEABLE		0x00020000
#define WF_STATE_SIZEABLE		0x00040000
#define WF_STATE_FREEZED		0x00080000

#define WF_STANDARD     		0x00070F00

int newgui_pid, gfx_pid;
int window_move, window_size;

typedef struct rect
{
  int x1, y1, x2, y2;
} rect_t;

typedef struct control
{
  int ctl_type;
  int ctl_id;
  void *ctl_ptr;

  struct control *next;
  struct control *prev;
} control_t;

typedef struct visible_rect
{
  rect_t rect;
  struct visible_rect *next;
} visible_rect_t;

typedef struct window
{
  int id, z;
  rect_t rect, client_rect;
  visible_rect_t  *visible_start, *visible_end;
  char name[256];
  unsigned int flags;
  struct task_struct *ptask;
  unsigned int menu_active;

  control_t *controls;
  control_t *focus;
} window_t;

window_t *win_list[MAX_WIN];
rect_t screen_rect, move_rect;
int znum, next_wid, close_pid, fullmove;

window_t *close_win;
control_t *window_ctl;

critical_section_t csect_gui;
