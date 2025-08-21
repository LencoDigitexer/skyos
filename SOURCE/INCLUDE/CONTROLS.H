/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\controls.h
/* Last Update: 27.01.1999
/* Version    : alpha
/* Coded by   : Hayrapetian Gregory
/* Docus      :
/************************************************************************/

#include "msg_ctl.h"
#include "list.h"

#define CTL_BUTTON  0x01
#define CTL_INPUT   0x02
#define CTL_MENU    0x03
#define CTL_TEXT    0x04
#define CTL_CHECK   0x05
#define CTL_LISTBOX 0x06
#define CTL_SCROLLBAR  0x07
#define CTL_TREE  0x08


/* Text align */
#define TEXT_ALIGN_LEFT    0x01
#define TEXT_ALIGN_MIDDLE  0x02
#define TEXT_ALIGN_RIGHT   0x03


typedef struct ctl_button
{
  rect_t rect;
  char name[256];
  unsigned int enabled;
  window_t *win;
} ctl_button_t;

typedef struct ctl_text
{
  rect_t rect;
  int align;
  unsigned char name[255];
  unsigned int enabled;
} ctl_text_t;

typedef struct ctl_check
{
  rect_t rect;
  int align;
  int state;
  unsigned char name[255];
  unsigned int enabled;
} ctl_check_t;

typedef struct ctl_scrollbar
{
   rect_t rect;
   unsigned int min;
   unsigned int max;

   unsigned int pos;

   control_t *parent;      // which window/control has this scrollbar
} ctl_sb_t;

typedef struct ctl_listbox
{
  rect_t rect;
  int align;               // Text Alignment
  int state;
  int count;               // Number of Items in Listbox
  int selected;            // Current selected item
  int firstshow;           // Number of items which is shown at first postition in
                           // listbox

  int width;               // Width of Listbox in charwidth (e.g. 5 = 5*8 = 40pixels)
  unsigned char name[255];
  struct list *items;

  ctl_sb_t *sb;            // pointer to a scrollbar
  unsigned int enabled;
} ctl_listbox_t;

typedef struct item_listbox
{
  int data;
  unsigned char *str;
} item_listbox_t;


typedef struct ctl_input
{
  rect_t rect;
  unsigned char data[255];
  unsigned int enabled;
} ctl_input_t;

typedef struct ctl_menuitem
{
  rect_t rect;
  int id, prev_id;
  char name[256];
  unsigned int enabled;
  unsigned int selected;         // is item selected
  unsigned int items;            // how much child items?
  unsigned int expand;           // is menu expanded

  unsigned int seperator;

  struct ctl_menuitem *next;
  struct ctl_menuitem *prev;
  struct ctl_menuitem *child;

  struct ctl_menuitem *active_child;
} ctl_menuitem_t;

typedef struct ctl_menu
{
  ctl_menuitem_t *menuitems;
  ctl_menuitem_t *menutitle;
  ctl_menuitem_t *active_item;  // current selected menuitem
  ctl_menuitem_t *old_item;

  ctl_menuitem_t *child;


  unsigned int enabled;

} ctl_menu_t;

typedef struct item_tree
{
   unsigned int id;
   unsigned char name[255];

   unsigned int selected;
   struct list *items;
   struct item_tree *child;

} item_tree_t;

typedef struct ctl_tree
{
   rect_t rect;
   unsigned int id;
   ctl_sb_t *sb;            // pointer to a scrollbar

   item_tree_t *child;
} ctl_tree_t;



