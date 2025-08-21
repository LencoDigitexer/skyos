/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\controls.c
/* Last Update: 27.08.1999
/* Version    : alpha
/* Coded by   : Hayrapetian Gregory
/* Docus      :
/************************************************************************/
/* Todo:
     - Autoopen next menu when cursor left/right
     - Underscore hotkey character
/************************************************************************/
#include "sched.h"
#include "newgui.h"
#include "controls.h"
#include "msg_ctl.h"

extern struct task_struct *current;

void draw_control(rect_t *rect, window_t *win, control_t *ctl, int focus_set);
void wdraw_control(rect_t *rect, window_t *win, control_t *ctl, int focus_set);

void add_control(window_t *win, void *cptr, int type, int id)
{

  control_t *control = (control_t*) valloc(sizeof(control_t));
  int focus = 0;

  control->ctl_type = type;
  control->ctl_ptr = cptr;
  control->ctl_id = id;
  control->next = NULL;
  control->prev = NULL;

  if (win->controls == NULL)
  {
    control->prev = control;
    win->controls = control;
    win->focus = control;
    focus = 1;
  }
  else
  {
    control->prev = win->controls->prev;
    win->controls->prev->next = control;
    win->controls->prev = control;
  }

  hide_cursor();
  wdraw_control(&win->rect,win,control,focus);
  show_cursor();
}


int destroy_control(window_t *win, void *cptr, int redraw)
{
  control_t *ctl_tmp, *ctl = win->controls;

  while (ctl)
  {
    if (ctl->ctl_ptr == cptr)
    {
      ctl_tmp = ctl;

      if (ctl->prev->next)
      {
       ctl->prev->next = ctl->next;

       if (ctl->next) ctl->next->prev = ctl->prev;
       else win->controls->prev = ctl->prev;
      }
      else
      {
        if (ctl->next)
        {
          ctl->next->prev = ctl->prev;
          win->controls = ctl->next;
        }
        else win->controls = NULL;
      }

      vfree(ctl_tmp->ctl_ptr);
      vfree(ctl_tmp);

      if (redraw)
      {
        hide_cursor();
    	redraw_rect(win->rect.x1,win->rect.y1,win->rect.x2,win->rect.y2,1);
        show_cursor();
      }

      return 0;
    }
    ctl = ctl->next;
  }

  return -1;
}

void wdraw_control(rect_t *rect, window_t *win, control_t *ctl, int focus_set)
{
  rect_t drect, rrect;
  visible_rect_t *vrect = win->visible_start;
  ctl_button_t *ctl_but;
  ctl_input_t *ctl_ipt;
  ctl_menu_t *ctl_menu;

  switch (ctl->ctl_type)
  {
    case CTL_BUTTON:    ctl_but = (ctl_button_t*) ctl->ctl_ptr;
                        drect = ctl_but->rect;
                        align_rect(&drect,win);

                        for (;vrect;vrect = vrect->next)
                        {
                          rrect = vrect->rect;
                          if (!fit_rect(&rrect, &win->client_rect))
                            if (rect_inrect(&drect,&rrect))
							{
                              draw_button(&rrect,win,ctl->ctl_ptr, focus_set);
							}

                        }
                        break;
    case CTL_INPUT:     ctl_ipt = (ctl_input_t*) ctl->ctl_ptr;
                        drect = ctl_ipt->rect;
                        align_rect(&drect,win);

                        for (;vrect;vrect = vrect->next)
                        {
                          rrect = vrect->rect;
                          if (!fit_rect(&rrect, &win->client_rect))
                            if (rect_inrect(&drect,&rrect))
							{
                              draw_input(&rrect, win, ctl->ctl_ptr, focus_set);
							}
                        }
                        break;
    case CTL_MENU:       draw_menu(rect, win, ctl->ctl_ptr, focus_set);
                        break;
    case CTL_TEXT:      draw_text(rect, win, ctl->ctl_ptr, focus_set);
                        break;
    case CTL_CHECK:     draw_check(rect, win, ctl->ctl_ptr, focus_set);
                        break;
    case CTL_LISTBOX:   draw_listbox(rect, win, ctl->ctl_ptr, focus_set);
                        break;
    case CTL_TREE   :   draw_tree(rect, win, ctl->ctl_ptr, focus_set);
                        break;
  }
}

void draw_control(rect_t *rect, window_t *win, control_t *ctl, int focus_set)
{

  switch (ctl->ctl_type)
  {
    case CTL_BUTTON:    draw_button(rect,win,ctl->ctl_ptr, focus_set);
                        break;
    case CTL_INPUT:     draw_input(rect, win, ctl->ctl_ptr, focus_set);
                        break;
    case CTL_MENU:      draw_menu(rect, win, ctl->ctl_ptr, focus_set);
                        break;
    case CTL_TEXT:      draw_text(rect, win, ctl->ctl_ptr, focus_set);
                        break;
    case CTL_CHECK:     draw_check(rect, win, ctl->ctl_ptr, focus_set);
                        break;
    case CTL_LISTBOX:   draw_listbox(rect, win, ctl->ctl_ptr, focus_set);
                        break;
    case CTL_TREE   :   draw_tree(rect, win, ctl->ctl_ptr, focus_set);
                        break;
  }

}

ctl_button_t *create_button(window_t *win, int x, int y, int length, int width, char *name, int id)
{
  int i;
  ctl_button_t *button = (ctl_button_t*) valloc(sizeof(ctl_button_t));

  set_rect(&button->rect,x,y,x+length,y+width);
  strcpy(button->name, name);

  button->win = win;
  add_control(win,button,CTL_BUTTON,id);

  return button;
}

ctl_text_t *create_text(window_t *win, int x, int y, int length, int width, char *name, int align, int id)
{
  int i;
  ctl_text_t *text = (ctl_text_t*) valloc(sizeof(ctl_text_t));

  set_rect(&text->rect,x,y,x+length,y+width);
  text->align = align;
  strcpy(text->name, name);

  add_control(win,text,CTL_TEXT,id);

  return text;
}

ctl_check_t *create_check(window_t *win, int x, int y, int length, int width, char *name, int state, int id)
{
  int i;
  ctl_check_t *check = (ctl_check_t*) valloc(sizeof(ctl_check_t));

  set_rect(&check->rect,x,y,x+length,y+width);
  check->state = state;
  strcpy(check->name, name);

  add_control(win,check,CTL_CHECK,id);

  return check;
}

ctl_sb_t *create_sb(window_t *win, int x, int y, int x2, int y2, int id)
{
  int i;
  ctl_sb_t *sb = (ctl_sb_t*) valloc(sizeof(ctl_listbox_t));

  set_rect(&sb->rect, x, y, x2, y2);
  sb->min = sb->max = sb->pos = 0;

  add_control(win,sb,CTL_SCROLLBAR,id);

  return sb;
}


ctl_listbox_t *create_listbox(window_t *win, int x, int y, int length, int width, int id)
{
  int i;
  ctl_listbox_t *listbox = (ctl_listbox_t*) valloc(sizeof(ctl_listbox_t));
  control_t *control = (control_t*)valloc(sizeof(control_t));

  set_rect(&listbox->rect,x,y,x+length,y+width*10+4);

  listbox->items = NULL;
  listbox->count = listbox->selected = listbox->firstshow = 0;
  listbox->width = width;
  listbox->enabled = 1;

  // create the scrollbar

  listbox->sb = create_sb(win, x + length+1, y, x+length+11,y+width*10+4,0);

  control->ctl_type = CTL_LISTBOX;
  control->ctl_ptr  = listbox;

  listbox->sb->parent = control;

  add_control(win,listbox,CTL_LISTBOX,id);

  return listbox;
}

int listbox_add(ctl_listbox_t *l, int data, unsigned char *str)
{
  item_listbox_t *item;
  struct list *li;

  item = (item_listbox_t*) valloc(sizeof(item_listbox_t));
  item->str = (unsigned char*)valloc(strlen(str)+1);

  strcpy(item->str, str);
  item->data = data;

  l->count++;
  l->sb->max++;

  if (l->selected == 0)
  {
    l->selected = 1;
    l->firstshow = 1;
    l->sb->pos = 1;
  }

  l->items = (struct list*)add_item(l->items,item ,sizeof(item_listbox_t) + strlen(str)+1);

  if ((l->firstshow + l->width) > l->count)
  {
  hide_cursor();
  draw_listbox(&screen_rect, win_list[znum-1], l, 1);
  show_cursor();
  }
}

void listbox_del_all(ctl_listbox_t* l)
{
  item_listbox_t *item;
  int i=0;

  l->count = l->selected = l->firstshow = 0;
  l->enabled = 1;

  l->sb->min = l->sb->max = l->sb->pos = 0;

  if (l->items ==NULL) return;

  item=(struct item_listbox_t*)get_item(l->items,i++);
  while (item!=NULL)
  {
    l->items=(struct list*)del_item(l->items,i-1);
    i=0;
    item=(struct item_listbox_t*)get_item(l->items,i++);
  }
  l->items = NULL;
}


ctl_tree_t *create_tree(window_t *win, int x, int y, int length, int width, int id)
{
  int i;
  ctl_tree_t *tree = (ctl_tree_t*) valloc(sizeof(ctl_tree_t));
  control_t *control = (control_t*)valloc(sizeof(control_t));

  set_rect(&tree->rect,x,y,x+length,y+width*10+4);

//  tree->items = NULL;
//  tree->count = tree->selected = tree->firstshow = 0;
//  tree->width = width;

//  create the scrollbar

  tree->sb = create_sb(win, x + length+1, y, x+length+11,y+width*10+4,0);

  control->ctl_type = CTL_TREE;
  control->ctl_ptr  = tree;

  tree->sb->parent = control;

  add_control(win,tree,CTL_TREE,id);

  return tree;
}

ctl_input_t *create_input(window_t *win, int x, int y, int length, int width, int id)
{
   ctl_input_t *input = (ctl_input_t*)valloc(sizeof(ctl_input_t));

   set_rect(&input->rect,x,y,x + length,y + width);
   memset(input->data, 0, 255);

   add_control(win, input, CTL_INPUT, id);

   return input;
}

/************************************************************************/
/************************************************************************/
/************************************************************************/

/*   Menu Control Functions

/************************************************************************/
/************************************************************************/
/************************************************************************/

ctl_menu_t *create_menu(window_t *win, int id)
{
  ctl_menu_t *menu = (ctl_menu_t*)valloc(sizeof(ctl_menu_t));

  menu->child = NULL;
  menu->menutitle = NULL;

  add_control(win, menu, CTL_MENU, id);

  return menu;
}

void add_menuitem(window_t *win, ctl_menu_t *menu, char *name, int id, int prev_id)
{
  ctl_menuitem_t *mitem, *mitem2, *newitem =
    (ctl_menuitem_t*)valloc(sizeof(ctl_menuitem_t));
  int pos = 2;

  newitem->next = NULL;
  newitem->child = NULL;
  newitem->items = 0;
  newitem->active_child = NULL;
  strcpy(newitem->name, name);
  newitem->id = id;
  newitem->prev_id = prev_id;
  newitem->selected = 0;

  if (name[0] == '_')
    newitem->seperator = 1;
  else newitem->seperator = 0;

  if (menu->child == NULL)       // First Top Level Menuitem
  {
    newitem->prev = newitem;
    menu->child = newitem;
  }

  else if (prev_id == 0)         // Top Level Menuitem
  {
    newitem->prev = menu->child->prev;
    menu->child->prev->next = newitem;
    menu->child->prev = newitem;
  }
  else                           // child item
  {
    mitem = menu->child;

    while ((mitem) && (mitem->id != prev_id))
      mitem=mitem->next;

    if (mitem)                   // parent item found
    {
      if (mitem->child == NULL)  // First Child Menuitem
      {
        newitem->prev = newitem;
        mitem->child = newitem;

        mitem->items++;
      }

      else                       // Insert child item
      {
        newitem->prev = mitem->child->prev;
        mitem->child->prev->next = newitem;
        mitem->child->prev = newitem;

        mitem->items++;
      }
    }

  }

  mitem = menu->child;        // Calculate rect of the menuitem
  if (prev_id == 0)           // Top level menu
  {
    while (mitem->next)
    {
      if (mitem->prev_id == 0)
        pos += strlen(mitem->name)*8+16;

      mitem = mitem->next;
    }

    // set the rect for the menuitem
    set_rect(&newitem->rect,pos,16,pos+strlen(newitem->name)*8+16,28);

    hide_cursor();
    draw_menu(&win->rect,win,menu,0);
    show_cursor();
  }
}

/*void add_menuitem(window_t *win, ctl_menu_t *menu, char *name, int id, int father_id)
{
  ctl_menuitem_t *mitem, *mitem2, *menuitem = (ctl_menuitem_t*)valloc(sizeof(ctl_menuitem_t));
  int pos=2;

  menuitem->id = id;
  menuitem->prev_id = prev_id;
  menuitem->next = NULL;
  menuitem->prev = NULL;
  menuitem->child = NULL;
  strcpy(menuitem->name, name);

  if (menu->child == NULL)
  {
    menuitem->prev = menuitem;
    menu->menuitems = menuitem;
  }
  else
  {
    menuitem->prev = menu->child->prev;
    menu->child->prev->next = menuitem;
    menu->child->prev = menuitem;
  }

  mitem = menu->child;
  if (prev_id == 0)  // Top level menu
  {
    while (mitem->next)
    {
      if (mitem->prev_id == 0)
        pos += strlen(mitem->name)*8+16;

      mitem = mitem->next;
    }

    // set the rect for the menuitem
    set_rect(&menuitem->rect,pos,16,pos+strlen(menuitem->name)*8+16,28);
  }
  else
  {
    do
    {
      mitem = mitem->next;
    } while (mitem->next && (mitem->id != prev_id));

    mitem2 = mitem;
    mitem = menu->menuitems;
    mitem = mitem->prev;
    do
    {
      mitem = mitem->prev;
    } while ((mitem != menu->menuitems) && (mitem->prev_id != prev_id));

    if (mitem != menu->menuitems)
      set_rect(&menuitem->rect,mitem->rect.x1,mitem->rect.y2 + 2,mitem->rect.x1 + strlen(menuitem->name)*8,mitem->rect.y2 + 12);
    else
      set_rect(&menuitem->rect,mitem2->rect.x1 + 5,32,mitem2->rect.x1 + 5 + strlen(menuitem->name)*8,42);
  }

  hide_cursor();
  draw_menu(&win->rect,win,menu,0);
//  draw_menu(&screen_rect,win,menu,0);
  show_cursor();

  return menuitem;
}

/*
  This function draws the menu. If an item is selected it would be highligthe
 */
void draw_menu(rect_t *rect, window_t *win, ctl_menu_t *menu, int focus_set)
{
  ctl_menuitem_t *mitem = menu->child;
  rect_t mrect;
  int i=0;

  set_screen_rect(&mrect,win->rect.x1+2,win->rect.y1+16,win->rect.x2-2,win->rect.y1+30);
  if (fit_rect(&mrect,rect))
    return;

  hide_cursor();

  cdraw_hline(&mrect, win->rect.x1+2, win->rect.x2-2, win->rect.y1+29, COLOR_GRAY);
  cdraw_hline(&mrect, win->rect.x1+2, win->rect.x2-2, win->rect.y1+30, COLOR_WHITE);

  while (mitem)
  {
      if (mitem->selected)   // item is selected
      {
        cdraw_fill_rect(&mrect, win->rect.x1+3+i, win->rect.y1+16, win->rect.x1+14+i+strlen(mitem->name)*8, win->rect.y1+28, COLOR_LIGHTBLUE);
        if (mitem->expand)
          draw_menufield(&screen_rect, win, menu, mitem, 1);
      }

      else
        cdraw_fill_rect(&mrect, win->rect.x1+2+i, win->rect.y1+16, win->rect.x1+14+i+strlen(mitem->name)*8, win->rect.y1+28, COLOR_LIGHTGRAY);

      couts(&mrect,win->rect.x1+8+i,win->rect.y1+19,mitem->name,COLOR_BLACK,255);
      i += strlen(mitem->name)*8+16;

      mitem = mitem->next;
  }

  show_cursor();
}

/*
  this function closes a menu
  */
void close_menu(ctl_menu_t *menu)
{
    if (menu == NULL) return;
    restore_menufield(win_list[znum-1],menu, menu->active_item);

    if (menu->active_item)
    {
      menu->active_item->selected = 0;
      menu->active_item->active_child = NULL;
      menu->active_item->expand = 0;
      menu->active_item = NULL;
    }

    win_list[znum-1]->menu_active = 0;

    hide_cursor();
    draw_menu(&screen_rect,win_list[znum-1],menu,1);
    show_cursor();
}

/*
  This function draws a pulldown menu
*/
void draw_menufield(rect_t *rect, window_t *win, ctl_menu_t *menu, ctl_menuitem_t *menuitem, int focus_set)
{
  ctl_menuitem_t *mitem = menuitem->child;
  int x1, y1, x2, y2,lenx=5,leny=2;
  int pos=0,i=0;

  while (mitem)
  {
    if ((strlen(mitem->name)*8) > lenx)
      lenx = strlen(mitem->name)*8;
    leny += 12;

    mitem = mitem->next;
  }

  x1 = win->rect.x1 + menuitem->rect.x1;
  y1 = win->rect.y1 + 31;
  x2 = win->rect.x1 + 12 + lenx + menuitem->rect.x1;
  y2 = win->rect.y1 + 32 + leny;

  hide_cursor();
  mask_rect(x1,y1,x2,y2,1);
  cdraw_fill_rect(rect, x1+1, y1+1, x2-1, y2-1, COLOR_LIGHTGRAY);

  cdraw_hline(rect, x1, x2, y1, COLOR_WHITE);
  cdraw_vline(rect, x1, y1, y2, COLOR_WHITE);
  cdraw_hline(rect, x1, x2, y2, COLOR_BLACK);
  cdraw_vline(rect, x2, y1, y2, COLOR_BLACK);

  pos = 0;
  mitem = menuitem->child;

  i = 0;

  while (mitem)
  {
    if (mitem == menuitem->active_child)
    {
      cdraw_fill_rect(rect, x1+1, y1+1+(i*12), x2-1, y1+1+(i*12)+11, COLOR_LIGHTBLUE);
    }

    if (mitem->seperator)
    {
      cdraw_hline(rect, x1+2, x2-2, y1+2+(i*12)+4, COLOR_GRAY);
      cdraw_hline(rect, x1+2, x2-2, y1+2+(i*12)+5, COLOR_WHITE);
    }
    else
    {
      couts(rect, x1+2,
                y1+2+(i*12),
                mitem->name,COLOR_BLACK,255);
    }
    mitem = mitem->next;
    i++;
  }
  show_cursor();
}

void restore_menufield(window_t *win, ctl_menu_t *menu, ctl_menuitem_t *menuitem)
{
  ctl_menuitem_t *mitem;
  int x1, y1, x2, y2,lenx=5,leny=2;

  if (!menu) return;
  if (!menuitem) return;

  mitem = menuitem->child;

  while (mitem)
  {
    if ((strlen(mitem->name)*8) > lenx)
      lenx = strlen(mitem->name)*8;
    leny += 12;

    mitem = mitem->next;
  }

  x1 = win->rect.x1 + menuitem->rect.x1;
  y1 = win->rect.y1 + 31;
  x2 = win->rect.x1 + 12 + lenx + menuitem->rect.x1;
  y2 = win->rect.y1 + 32 + leny;

  redraw_rect(x1,y1,x2,y2,1);
}

void draw_input(rect_t *rect, window_t *win, ctl_input_t *input, int focus_set)
{
  int x1,y1,x2,y2;
  rect_t brect, trect;

  set_screen_rect(&brect,win->client_rect.x1,win->client_rect.y1,win->client_rect.x2,win->client_rect.y2);
  if (fit_rect(&brect,rect))
    return;

  x1 = input->rect.x1 + win->client_rect.x1;
  y1 = input->rect.y1 + win->client_rect.y1;
  x2 = input->rect.x2 + win->client_rect.x1;
  y2 = input->rect.y2 + win->client_rect.y1;

  cdraw_fill_rect(&brect, x1+2, y1+2, x2-1, y2-1, COLOR_WHITE);

  cdraw_hline(&brect, x1+1, x2, y2, COLOR_WHITE);
  cdraw_vline(&brect, x2, y1+1, y2, COLOR_WHITE);
  cdraw_hline(&brect, x1+2, x2-1, y2-1, COLOR_LIGHTGRAY);
  cdraw_vline(&brect, x2-1, y1+2, y2-1, COLOR_LIGHTGRAY);

  cdraw_hline(&brect, x1, x2, y1, COLOR_BLACK);
  cdraw_hline(&brect, x1+1, x2-1, y1+1, COLOR_GRAY);
  cdraw_vline(&brect, x1, y1, y2, COLOR_BLACK);
  cdraw_vline(&brect, x1+1, y1+1, y2-1, COLOR_GRAY);

  set_screen_rect(&trect,x1,y1,x2-5,y2);
  if (fit_rect(&trect,&brect))
    return;
  y1 += (input->rect.y2-input->rect.y1)/2 - 4;

  couts(&trect,x1+5,y1+1,input->data,COLOR_BLACK,255);

  if (focus_set)
    couts(&trect,x1+5+strlen(input->data)*8,y1+1,"_",COLOR_BLACK,255);
}

void wdraw_input(rect_t *rect, window_t *win, ctl_input_t *input, int focus_set)
{
  int x1,y1,x2,y2;
  rect_t brect, trect;

  set_screen_rect(&brect,win->rect.x1+3,win->rect.y1+17,win->rect.x2-3,win->rect.y2-3);
  if (fit_rect(&brect,rect))
    return;

  x1 = input->rect.x1 + win->rect.x1+3;
  y1 = input->rect.y1 + win->rect.y1+17;
  x2 = input->rect.x2 + win->rect.x1+3;
  y2 = input->rect.y2 + win->rect.y1+17;

  cdraw_fill_rect(&brect, x1+2, y1+2, x2-1, y2-1, COLOR_LIGHTGRAY);

  cdraw_hline(&brect, x1+1, x2, y2, COLOR_WHITE);
  cdraw_vline(&brect, x2, y1+1, y2, COLOR_WHITE);

  cdraw_hline(&brect, x1, x2, y1, COLOR_BLACK);
  cdraw_hline(&brect, x1+1, x2-1, y1+1, COLOR_GRAY);
  cdraw_vline(&brect, x1, y1, y2, COLOR_BLACK);
  cdraw_vline(&brect, x1+1, y1+1, y2-1, COLOR_GRAY);

  set_screen_rect(&trect,x1,y1,x2-5,y2);
  if (fit_rect(&trect,&brect))
    return;
  y1 += (input->rect.y2-input->rect.y1)/2 - 4;

  couts(&trect,x1+5,y1+1,input->data,COLOR_BLACK,255);

  if (focus_set)
    couts(&trect,x1+5+strlen(input->data)*8,y1+1,"_",COLOR_BLACK,255);
}

void draw_scrollbar(rect_t *rect, window_t *win, ctl_sb_t *sb)
{
  int y1, x2, y2;
  int sb_items = 0;
  int sb_size = 0;
  int sb_y = 0;
  int sb_length = 0;
  double sb_inc = 0;

  double d1, d2, d3,d4;

  y1 = sb->rect.y1 + win->client_rect.y1;
  x2 = sb->rect.x2 + win->client_rect.x1;
  y2 = sb->rect.y2 + win->client_rect.y1;


  // draw the scroll up button
  draw_button_direct(rect, x2-10, y1, x2, y1+10);
  cdraw_line(rect, x2-7, y1+6, x2-5,y1+4,COLOR_BLACK);
  cdraw_line(rect, x2-5, y1+4, x2-3,y1+6,COLOR_BLACK);

  // draw the scroll down button
  draw_button_direct(rect, x2-10, y2-10, x2, y2);
  cdraw_line(rect, x2-7, y2-6, x2-5,y2-4,COLOR_BLACK);
  cdraw_line(rect, x2-5, y2-4, x2-3,y2-6,COLOR_BLACK);

  // now draw the slide button
  cdraw_fill_rect(rect, x2-10, y1+11, x2, y2-11, COLOR_GRAY);
  cdraw_vline(rect, x2, y1+11, y2-11, COLOR_BLACK);

  if (sb->max != 0)
  {
    sb_length = y2 - y1 - 22;
    sb_size = sb_length / sb->max;

    if (sb_size < 3)
    {
      if (sb_size > 0)
        sb_length -= sb_size;
      else
        sb_length -= 3;

      sb_size = 3;
    }

    (double)d1 = sb_length;
    (double)d2 = sb->max;
    (double)d3 = sb->pos - 1;


    (double)sb_inc = (double)((double)d1 / (double)d2);

    (double)d4 = (double)((double)sb_inc * (double)d3);


    (double)sb_y = (double)d4;

    sb_y += y1;
    sb_y += 11;

    draw_button_direct(rect, x2-10, sb_y, x2, sb_y + sb_size);
  }
}

void draw_listbox(rect_t *rect, window_t *win, ctl_listbox_t *listbox, int focus_set)
{
  int x1,y1,x2,y2;
  rect_t brect, trect;
  int i=0,j;
  int show;
  int hidden_items;

  int i1,i2;

  item_listbox_t *item = (item_listbox_t*)get_item(listbox->items,i++);

  set_screen_rect(&brect,win->client_rect.x1,win->client_rect.y1,win->client_rect.x2,win->client_rect.y2);
  if (fit_rect(&brect,rect))
    return;

  x1 = listbox->rect.x1 + win->client_rect.x1;
  y1 = listbox->rect.y1 + win->client_rect.y1;
  x2 = listbox->rect.x2 + win->client_rect.x1;
  y2 = listbox->rect.y2 + win->client_rect.y1;

  hide_cursor();
//  cdraw_fill_rect(&brect, x1+2, y1+2, x2-1, y2-1, COLOR_WHITE);

  cdraw_hline(&brect, x1+1, x2, y2, COLOR_WHITE);
  cdraw_vline(&brect, x2, y1+1, y2, COLOR_WHITE);
  cdraw_hline(&brect, x1+2, x2-1, y2-1, COLOR_LIGHTGRAY);
  cdraw_vline(&brect, x2-1, y1+2, y2-1, COLOR_LIGHTGRAY);

  cdraw_hline(&brect, x1, x2, y1, COLOR_BLACK);
  cdraw_hline(&brect, x1+1, x2-1, y1+1, COLOR_GRAY);
  cdraw_vline(&brect, x1, y1, y2, COLOR_BLACK);
  cdraw_vline(&brect, x1+1, y1+1, y2-1, COLOR_GRAY);

  show_cursor();
  if (!listbox->enabled) return;
  hide_cursor();

  draw_scrollbar(&brect, win, listbox->sb);

  set_screen_rect(&trect,x1,y1,x2-2,y2-2);
  show_cursor();
  if (fit_rect(&trect,&brect))
    return;
  hide_cursor();

  show = 0;

  hidden_items = listbox->firstshow;

  while (hidden_items > 1)
  {
     show++;
     hidden_items--;
     item=(item_listbox_t*)get_item(listbox->items,i++);
  }

  j = 0;
  while (item!=NULL)
  {
    show++;
    if (listbox->selected == show)
    {
      cdraw_fill_rect(&trect, x1+2, y1+2+(j)*10-1, x2, y1+2+(j)*10+9, COLOR_BLUE);
      couts(&trect,x1+5,y1+2+(j)*10,item->str,COLOR_WHITE,255);
    }
    else
    {
      cdraw_fill_rect(&trect, x1+2, y1+2+(j)*10-1, x2, y1+2+(j)*10+9, COLOR_WHITE);
      couts(&trect,x1+5,y1+2+(j)*10,item->str,COLOR_BLACK,255);
    }
    item=(item_listbox_t*)get_item(listbox->items,i++);

    j++;
    if (j>listbox->width) break;
  }
  if (j<listbox->width)
  {
     for (;j<=listbox->width;j++)
      cdraw_fill_rect(&trect, x1+2, y1+2+(j)*10-1, x2, y1+2+(j)*10+9, COLOR_WHITE);
  }


  show_cursor();

//  if (focus_set)
//    couts(&trect,x1+5,y1+1,"_",COLOR_BLACK,255);
}

void draw_tree(rect_t *rect, window_t *win, ctl_tree_t *tree, int focus_set)
{
  int x1,y1,x2,y2;
  rect_t brect, trect;
  int i=0,j;
  int show;
  int hidden_items;

  int i1,i2;

  item_tree_t *item = (item_tree_t*)tree->child;

  set_screen_rect(&brect,win->client_rect.x1,win->client_rect.y1,win->client_rect.x2,win->client_rect.y2);
  if (fit_rect(&brect,rect))
    return;

  x1 = tree->rect.x1 + win->client_rect.x1;
  y1 = tree->rect.y1 + win->client_rect.y1;
  x2 = tree->rect.x2 + win->client_rect.x1;
  y2 = tree->rect.y2 + win->client_rect.y1;

  hide_cursor();
  cdraw_fill_rect(&brect, x1+2, y1+2, x2-1, y2-1, COLOR_WHITE);

  cdraw_hline(&brect, x1+1, x2, y2, COLOR_WHITE);
  cdraw_vline(&brect, x2, y1+1, y2, COLOR_WHITE);
  cdraw_hline(&brect, x1+2, x2-1, y2-1, COLOR_LIGHTGRAY);
  cdraw_vline(&brect, x2-1, y1+2, y2-1, COLOR_LIGHTGRAY);

  cdraw_hline(&brect, x1, x2, y1, COLOR_BLACK);
  cdraw_hline(&brect, x1+1, x2-1, y1+1, COLOR_GRAY);
  cdraw_vline(&brect, x1, y1, y2, COLOR_BLACK);
  cdraw_vline(&brect, x1+1, y1+1, y2-1, COLOR_GRAY);

  draw_scrollbar(&brect, win, tree->sb);

  set_screen_rect(&trect,x1,y1,x2-2,y2-2);
  show_cursor();
  if (fit_rect(&trect,&brect))
    return;
/*  hide_cursor();

  show = 0;

  hidden_items = tree->firstshow;

  while (hidden_items > 1)
  {
     show++;
     hidden_items--;
     item=(item_tree_t*)get_item(tree->items,i++);
  }

  j = 0;
  while (item!=NULL)
  {
    show++;
    if (tree->selected == show)
    {
      cdraw_fill_rect(&trect, x1+2, y1+2+(j)*10-1, x2, y1+2+(j)*10+9, COLOR_BLUE);
      couts(&trect,x1+5,y1+2+(j)*10,item->str,COLOR_WHITE,255);
    }
    else
      couts(&trect,x1+5,y1+2+(j)*10,item->str,COLOR_BLACK,255);

    item=(item_tree_t*)get_item(tree->items,i++);

    j++;
    if (j>tree->width) break;
  }

  show_cursor();

//  if (focus_set)
//    couts(&trect,x1+5,y1+1,"_",COLOR_BLACK,255);
  */
}

void draw_button_direct(rect_t *brect, int x1, int y1, int x2, int y2)
{
  cdraw_fill_rect(brect, x1+1, y1+1, x2-2, y2-2, COLOR_LIGHTGRAY);


  cdraw_hline(brect, x1, x2-1, y1, COLOR_WHITE);
  cdraw_vline(brect, x1, y1, y2-1, COLOR_WHITE);

  cdraw_hline(brect, x1, x2, y2, COLOR_BLACK);
  cdraw_hline(brect, x1+1, x2-1, y2-1, COLOR_GRAY);
  cdraw_vline(brect, x2, y1, y2, COLOR_BLACK);
  cdraw_vline(brect, x2-1, y1+1, y2-1, COLOR_GRAY);
}

void draw_button(rect_t *rect, window_t *win, ctl_button_t *button, int focus_set)
{
  int x1,y1,x2,y2;
  rect_t brect, trect;

  set_screen_rect(&brect,win->client_rect.x1,win->client_rect.y1,win->client_rect.x2,win->client_rect.y2);
  if (fit_rect(&brect,rect))
    return;

  x1 = button->rect.x1 + win->client_rect.x1;
  y1 = button->rect.y1 + win->client_rect.y1;
  x2 = button->rect.x2 + win->client_rect.x1;
  y2 = button->rect.y2 + win->client_rect.y1;

  cdraw_fill_rect(&brect, x1+1, y1+1, x2-2, y2-2, COLOR_LIGHTGRAY);

  cdraw_hline(&brect, x1, x2-1, y1, COLOR_WHITE);
  cdraw_vline(&brect, x1, y1, y2-1, COLOR_WHITE);

  cdraw_hline(&brect, x1, x2, y2, COLOR_BLACK);
  cdraw_hline(&brect, x1+1, x2-1, y2-1, COLOR_GRAY);
  cdraw_vline(&brect, x2, y1, y2, COLOR_BLACK);
  cdraw_vline(&brect, x2-1, y1+1, y2-1, COLOR_GRAY);

  if (focus_set) cdraw_rect(&brect, x1 +3, y1 +3, x2-3, y2-3,COLOR_GRAY);

  set_screen_rect(&trect,x1+2,y1+2,x2-2,y2-2);
  if (fit_rect(&trect,&brect))
    return;

  x1 += (button->rect.x2-button->rect.x1)/2 - (strlen(button->name)*4);
  y1 += (button->rect.y2-button->rect.y1)/2 - 4;

  couts(&trect,x1+1,y1,button->name,COLOR_BLACK,255);
}

void draw_text(rect_t *rect, window_t *win, ctl_text_t *text, int focus_set)
{
  int x1,y1,x2,y2;
  rect_t brect, trect;

  set_screen_rect(&brect,win->client_rect.x1,win->client_rect.y1,win->client_rect.x2,win->client_rect.y2);
  if (fit_rect(&brect,rect))
    return;

  x1 = text->rect.x1 + win->client_rect.x1;
  y1 = text->rect.y1 + win->client_rect.y1;
  x2 = text->rect.x2 + win->client_rect.x1;
  y2 = text->rect.y2 + win->client_rect.y1;

  set_screen_rect(&trect,x1+2,y1+2,x2-2,y2-2);
  if (fit_rect(&trect,&brect))
    return;

  switch (text->align)
  {
    case TEXT_ALIGN_LEFT:
         x1 += 3;
         break;
    case TEXT_ALIGN_MIDDLE:
         x1 += (text->rect.x2-text->rect.x1)/2 - (strlen(text->name)*4);
         break;
    case TEXT_ALIGN_RIGHT:
         x1 += (text->rect.x2-text->rect.x1) - (strlen(text->name)*8);
         break;
  }

  y1 += (text->rect.y2-text->rect.y1)/2 - 4;

  couts(&trect,x1+1,y1,text->name,COLOR_BLACK,255);
}

void draw_check(rect_t *rect, window_t *win, ctl_check_t *check, int focus_set)
{
  int x1,y1,x2,y2;
  rect_t brect, trect;

  set_screen_rect(&brect,win->client_rect.x1,win->client_rect.y1,win->client_rect.x2,win->client_rect.y2);
  if (fit_rect(&brect,rect))
    return;

  x1 = check->rect.x1 + win->client_rect.x1;
  y1 = check->rect.y1 + win->client_rect.y1;
  x2 = check->rect.x2 + win->client_rect.x1;
  y2 = check->rect.y2 + win->client_rect.y1;

  set_screen_rect(&trect,x1+2,y1+2,x2-2,y2-2);
  if (fit_rect(&trect,&brect))
    return;

  y1 += (check->rect.y2-check->rect.y1)/2 - 4;

  couts(&trect,x1+15,y1+3,check->name,COLOR_BLACK,255);


  cdraw_fill_rect(&trect, x1+2, y1+2, x1 + 12, y1 + 12, COLOR_WHITE);

  cdraw_hline(&brect, x1+1, x1+12, y1+1, COLOR_BLACK);
  cdraw_vline(&brect, x1+1, y1+1, y1+12, COLOR_BLACK);

  cdraw_hline(&brect, x1, x1+13, y1, COLOR_GRAY);
  cdraw_vline(&brect, x1, y1, y1+13, COLOR_GRAY);

  cdraw_hline(&brect, x1+1, x1+13, y1+13, COLOR_LIGHTGRAY);
  cdraw_vline(&brect, x1+13, y1+1, y1+13, COLOR_LIGHTGRAY);

  cdraw_hline(&brect, x1, x1+14, y1+14, COLOR_WHITE);
  cdraw_vline(&brect, x1+14, y1, y1+14, COLOR_WHITE);

  if (check->state == 1)
    couts(&trect,x1+4,y1+2,"x",COLOR_BLACK,255);

  cdraw_circlefill(&brect, x1+6,y1+6,4,2);

}

void draw_button_p(rect_t *rect, window_t *win, ctl_button_t *button, int focus_set)
{
  int x1,y1,x2,y2;
  rect_t brect, trect;

  set_screen_rect(&brect,win->client_rect.x1,win->client_rect.y1,win->client_rect.x2,win->client_rect.y2);
  if (fit_rect(&brect,rect))
    return;

  x1 = button->rect.x1 + win->client_rect.x1;
  y1 = button->rect.y1 + win->client_rect.y1;
  x2 = button->rect.x2 + win->client_rect.x1;
  y2 = button->rect.y2 + win->client_rect.y1;

  cdraw_fill_rect(&brect, x1+2, y1+2, x2-1, y2-1, COLOR_LIGHTGRAY);

  cdraw_hline(&brect, x1+1, x2, y2, COLOR_WHITE);
  cdraw_vline(&brect, x2, y1+1, y2, COLOR_WHITE);

  cdraw_hline(&brect, x1, x2, y1, COLOR_BLACK);
  cdraw_hline(&brect, x1+1, x2-1, y1+1, COLOR_GRAY);
  cdraw_vline(&brect, x1, y1, y2, COLOR_BLACK);
  cdraw_vline(&brect, x1+1, y1+1, y2-1, COLOR_GRAY);

  set_screen_rect(&trect,x1+2,y1+2,x2-2,y2-2);
  if (fit_rect(&trect,&brect))
    return;

  x1 += (button->rect.x2-button->rect.x1)/2 - (strlen(button->name)*4);
  y1 += (button->rect.y2-button->rect.y1)/2 - 4;

  couts(&trect,x1+2,y1+1,button->name,COLOR_BLACK,255);
}

void GetItemText_input(window_t *win, ctl_input_t *ctl, unsigned char *buffer)
{
  strcpy(buffer, ctl->data);
}

control_t *get_control(window_t *win, void *cptr)
{
  control_t *ctl = win->controls;

  while (ctl)
  {
    if (ctl->ctl_ptr == cptr)
      return ctl;

    ctl = ctl->next;
  }

  return NULL;
}

void SetItemText_input(window_t *win, ctl_input_t *ctl_ipt, unsigned char *buffer)
{
  control_t *ctl = get_control(win,ctl_ipt);

  strcpy(ctl_ipt->data, buffer);
  hide_cursor();
  wdraw_control(&screen_rect, win, ctl, 0);
  show_cursor();
}

unsigned int listbox_GetSelectedItemData(ctl_listbox_t *l)
{
  int i = 0;

  item_listbox_t *item = (item_listbox_t*)get_item(l->items,i++);

  while (i!= l->selected)
  {
    item = (item_listbox_t*)get_item(l->items,i++);
  }

  return item->data;
}







