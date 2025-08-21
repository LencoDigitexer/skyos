/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\gui_cm.c
/* Last Update: 02.04.1999
/* Version    : alpha
/* Coded by   : Hayrapetian Gregory
/* Docus      :
/************************************************************************/
/* Definition:
/*   GUI Control Manager
/************************************************************************/
/* Updated:
    260100    Added the keypressed_menu function
/************************************************************************/
#include "sched.h"
#include "newgui.h"
#include "controls.h"

extern unsigned int shift;

control_t* check_ctl_button(control_t *ctl, int x, int y)
{
  ctl_button_t *button = ctl->ctl_ptr;
  rect_t rect = button->rect;

  align_rect(&rect, win_list[znum-1]);

  if (pt_inrect(&rect,x,y))
  {
    hide_cursor();
    if (win_list[znum-1]->focus != ctl)
      draw_control(&screen_rect, win_list[znum-1], win_list[znum-1]->focus, 0);

    draw_button_p(&screen_rect,win_list[znum-1],button, 1);
    show_cursor();

    win_list[znum-1]->focus = ctl;
    return ctl;
  }

  return NULL;
}

control_t* check_ctl_menu(control_t *ctl, int x, int y)
{
  rect_t rect;
  ctl_menu_t *menu;
  ctl_menuitem_t *mitem;

  menu = ctl->ctl_ptr;
  mitem = menu->menuitems;

  while (mitem)
  {
    if (mitem->prev_id == 0)
    {
      set_rect(&rect,win_list[znum-1]->rect.x1 + mitem->rect.x1,
                     win_list[znum-1]->rect.y1 + mitem->rect.y1,
                     win_list[znum-1]->rect.x1 + mitem->rect.x2,
                     win_list[znum-1]->rect.y1 + mitem->rect.y2);

      if (pt_inrect(&rect,x,y))
      {
        menu->old_item = menu->active_item;
        menu->active_item = mitem;
        menu->menutitle = mitem;

        hide_cursor();
        draw_menu(&screen_rect,win_list[znum-1],menu,1);
        show_cursor();

        return ctl;
      }
    }

    mitem = mitem->next;
  }

  return NULL;
}

control_t* check_ctl_input(control_t *ctl, int x, int y)
{
  struct ipc_message m;
  ctl_input_t *input = ctl->ctl_ptr;
  rect_t rect = input->rect;

  align_rect(&rect,win_list[znum-1]);

  if (pt_inrect(&rect,x,y))
  {
    hide_cursor();
    if (win_list[znum-1]->focus != ctl)
      draw_control(&screen_rect, win_list[znum-1], win_list[znum-1]->focus, 0);

    draw_input(&screen_rect,win_list[znum-1],input, 1);
    show_cursor();

    win_list[znum-1]->focus = ctl;

    m.type = MSG_GUI_TEXT_CLICKED;
    m.MSGT_GUI_TEXT_CLICKED_ITEMID = ctl->ctl_id;
    spass_msg(win_list[znum-1], &m);

    return ctl;
  }

  return NULL;
}

control_t* check_ctl_listbox(control_t *ctl, int x, int y)
{
  ctl_listbox_t *listbox = ctl->ctl_ptr;
  rect_t rect = listbox->rect;
  int yrel = 0;

  align_rect(&rect,win_list[znum-1]);

  if (pt_inrect(&rect,x,y))
  {
    hide_cursor();
    if (win_list[znum-1]->focus != ctl)
      draw_control(&screen_rect, win_list[znum-1], win_list[znum-1]->focus, 0);
    show_cursor();

    win_list[znum-1]->focus = ctl;

// item selected?
    set_rect(&rect, listbox->rect.x1+1,listbox->rect.y1+1,listbox->rect.x2-12,
                    listbox->rect.y2-1);
    align_rect(&rect, win_list[znum-1]);

    if (pt_inrect(&rect, x, y))
    {
      yrel = y - rect.y1;

      if ((listbox->firstshow + yrel/10) <= listbox->count)
      {
         listbox->selected = listbox->firstshow + yrel / 10;
         listbox->sb->pos = listbox->selected;

         hide_cursor();
         draw_listbox(&screen_rect, win_list[znum-1], listbox, 1);
         show_cursor();
      }
    }
    return ctl;
  }

  return NULL;
}

control_t* check_ctl_sb(control_t *ctl, int x, int y)
{
  ctl_sb_t *sb = ctl->ctl_ptr;
  rect_t rect = sb->rect;
  int up = 0, down = 0;

  align_rect(&rect,win_list[znum-1]);

  if (pt_inrect(&rect,x,y))
  {
    hide_cursor();
    if (win_list[znum-1]->focus != ctl)
      draw_control(&screen_rect, win_list[znum-1], win_list[znum-1]->focus, 0);
    show_cursor();

    win_list[znum-1]->focus = ctl;

// button scroll-up?
    set_rect(&rect, sb->rect.x2-10,sb->rect.y1,sb->rect.x2,
                    sb->rect.y1+10);
    align_rect(&rect, win_list[znum-1]);

    if (pt_inrect(&rect, x, y))
      up = 1;

// button scroll-down?
    set_rect(&rect, sb->rect.x2-10,sb->rect.y2-10,sb->rect.x2,
                    sb->rect.y2);
    align_rect(&rect, win_list[znum-1]);

    if (pt_inrect(&rect, x, y))
      down = 1;

    switch (sb->parent->ctl_type)
    {
        case CTL_LISTBOX:
                          if (up)
                             listbox_scrollup(sb->parent->ctl_ptr);
                          if (down)
                             listbox_scrolldown(sb->parent->ctl_ptr);
                           break;
    }
  }

  return NULL;
}

/************************************************************************/
/* This function is called if a control is pressed with a mouse button
/************************************************************************/
control_t* check_control(int x, int y)
{
  control_t *ctl = win_list[znum-1]->controls;
  control_t *ret = NULL;

  while (ctl)
  {
    switch (ctl->ctl_type)
    {
      case CTL_BUTTON: ret = check_ctl_button(ctl,x,y);
                       break;
      case CTL_MENU:   ret = check_ctl_menu(ctl,x,y);
                       break;
      case CTL_INPUT:  ret = check_ctl_input(ctl,x,y);
                       break;
      case CTL_LISTBOX:  ret = check_ctl_listbox(ctl,x,y);
                       break;
      case CTL_SCROLLBAR:
                       ret = check_ctl_sb(ctl, x, y);
                       break;
    }

    if (ret) return ret;

    ctl = ctl->next;
  }

  return NULL;
}

void destroy_controls(window_t *win)
{
  control_t *ctl = win->controls;

  while (ctl)
  {
    destroy_control(win, ctl->ctl_ptr, 0);
    ctl = ctl->next;
  }
}

void handle_ctl_button(control_t *ctl, int mx, int my)
{
  ctl_button_t *button = ctl->ctl_ptr;
  rect_t rect = button->rect;

  align_rect(&rect,win_list[znum-1]);

  if (!pt_inrect(&rect,mx,my))
  {
    hide_cursor();
    draw_control(&screen_rect,win_list[znum-1], window_ctl,1);
    show_cursor();
    window_ctl = NULL;
  }
}

void handle_ctl_menu(control_t *ctl,int mx,int my)
{
  ctl_menu_t *menu = ctl->ctl_ptr;
  ctl_menuitem_t *mitem = menu->menuitems;
  rect_t rect;

  while (mitem)
  {
    set_rect(&rect,win_list[znum-1]->rect.x1 + mitem->rect.x1,
                   win_list[znum-1]->rect.y1 + mitem->rect.y1,
                   win_list[znum-1]->rect.x1 + mitem->rect.x2,
                   win_list[znum-1]->rect.y1 + mitem->rect.y2);

    if (mitem->prev_id == 0)
    {

      if ((menu->active_item != mitem) && pt_inrect(&rect,mx,my))
      {
//        menu_
        hide_cursor();
        restore_menufield(win_list[znum-1],menu,menu->menutitle);
        show_cursor();

        menu->old_item = menu->active_item;
        menu->menutitle = mitem;
        menu->active_item = mitem;

        hide_cursor();
        draw_menu(&screen_rect,win_list[znum-1],menu, 1);
        show_cursor();
      }
    }
    else if (mitem->prev_id == menu->menutitle->id)
    {

      if (pt_inrect(&rect,mx,my))
      {
        if (menu->active_item == menu->menutitle)
        {
          rect_t arect = menu->active_item->rect;

          enlarge_rect(&arect,win_list[znum-1]->rect.x1,win_list[znum-1]->rect.y1,win_list[znum-1]->rect.x1,win_list[znum-1]->rect.y1);

          hide_cursor();
          cdraw_fill_rect(&screen_rect,arect.x1,arect.y1,arect.x2,arect.y2,COLOR_LIGHTGRAY);
          couts(&screen_rect, arect.x1+6, arect.y1+3,menu->active_item->name,COLOR_BLACK,255);

          cdraw_fill_rect(&screen_rect,rect.x1,rect.y1,rect.x2,rect.y2,COLOR_LIGHTBLUE);
          couts(&screen_rect, rect.x1+1, rect.y1+1,mitem->name,COLOR_BLACK,255);
          show_cursor();

          menu->active_item = mitem;
        }
        else if (menu->active_item != mitem)
        {
          rect_t arect = menu->active_item->rect;

          enlarge_rect(&arect,win_list[znum-1]->rect.x1,win_list[znum-1]->rect.y1,win_list[znum-1]->rect.x1,win_list[znum-1]->rect.y1);

          hide_cursor();
          cdraw_fill_rect(&screen_rect,arect.x1,arect.y1,arect.x2,arect.y2,COLOR_LIGHTGRAY);
          couts(&screen_rect, win_list[znum-1]->rect.x1+menu->active_item->rect.x1+1, win_list[znum-1]->rect.y1+menu->active_item->rect.y1+1,menu->active_item->name,COLOR_BLACK,255);

          cdraw_fill_rect(&screen_rect,rect.x1,rect.y1,rect.x2,rect.y2,COLOR_LIGHTBLUE);
          couts(&screen_rect, rect.x1+1, rect.y1+1,mitem->name,COLOR_BLACK,255);
          show_cursor();

          menu->active_item = mitem;
        }
      }
    }
    mitem = mitem->next;
  }

}

/************************************************************************/
/* This function is called if a control is the mouse is moved over a control
/************************************************************************/
void handle_control(int mx,int my)
{
  ctl_menu_t *menu;
  rect_t rect;

  switch (window_ctl->ctl_type)
  {
    case CTL_BUTTON: handle_ctl_button(window_ctl,mx,my);
                     break;
    case CTL_MENU:   handle_ctl_menu(window_ctl,mx,my);
                     break;
  }
}


void perform_ctl_button(control_t *ctl)
{
  struct ipc_message m;

  m.type = MSG_GUI_CTL_BUTTON_PRESSED;
  m.MSGT_GUI_CTL_ID = (int) ctl->ctl_id;
  spass_msg(win_list[znum-1], &m);
}

void perform_ctl_menu(control_t *ctl)
{
  struct ipc_message m;
  ctl_menu_t *menu = ctl->ctl_ptr;

  hide_cursor();
  restore_menufield(win_list[znum-1],menu,menu->menutitle);
  show_cursor();

  m.type = MSG_GUI_CTL_MENU_ITEM;

  if (menu->active_item != NULL)
    m.MSGT_GUI_CTL_ID = (int) menu->active_item->id;
  else
    m.MSGT_GUI_CTL_ID = (int) 0;

  send_msg(win_list[znum-1]->ptask->pid, &m);

  menu->old_item = menu->active_item;
  menu->active_item = NULL;
  menu->menutitle = NULL;
}

/************************************************************************/
/* This function is called if a mouse button is released on a control
/************************************************************************/
void perform_control()
{
  switch (window_ctl->ctl_type)
  {
    case CTL_BUTTON: perform_ctl_button(window_ctl);
		     break;
    case CTL_MENU:   perform_ctl_menu(window_ctl);
		     break;
  }

  hide_cursor();
  draw_control(&screen_rect,win_list[znum-1], window_ctl, 1);
  show_cursor();
}

void pass_active_control(window_t *win, unsigned int shift)
{
  control_t *c;

  if (win_list[znum-1]->focus)
  {
    hide_cursor();
    if (win_list[znum-1]->focus->ctl_type == CTL_MENU)
    {
      close_menu(win_list[znum-1]->focus->ctl_ptr );
    }
    else draw_control(&screen_rect, win_list[znum-1], win_list[znum-1]->focus, 0);

    if (!shift)
    {
    	if (win_list[znum-1]->focus->next)
      {
         c = win_list[znum-1]->focus->next;
         while ((c) && (c->ctl_type == CTL_MENU))
                c=c->next;
         if (c)
           win_list[znum-1]->focus = c;
         else
         {
           c = win_list[znum-1]->controls;
           while ((c) && (c->ctl_type == CTL_MENU))
                c=c->next;
           if (c)
             win_list[znum-1]->focus = c;
         }
      }
	   else
      {
           c = win_list[znum-1]->controls;
           while ((c) && (c->ctl_type == CTL_MENU))
                c=c->next;
           if (c)
             win_list[znum-1]->focus = c;
      }
    }
    else {
	if (win_list[znum-1]->focus->prev)
          win_list[znum-1]->focus = win_list[znum-1]->focus->prev;
	else 
          win_list[znum-1]->focus = win_list[znum-1]->controls;
    }

    draw_control(&screen_rect, win_list[znum-1], win_list[znum-1]->focus, 1);
    show_cursor();
  }
}

void keypressed_button(control_t *ctl, unsigned char ch)
{
  ctl_button_t *ctl_but = (ctl_button_t*) ctl->ctl_ptr;
  struct ipc_message m;

  switch (ch)
  {
    case KEY_ENTER:  m.type = MSG_GUI_CTL_BUTTON_PRESSED;
					      m.MSGT_GUI_CTL_ID = (int) ctl->ctl_id;
                     m.MSGT_GUI_CTL_WIN = ctl_but->win->id;
					      spass_msg(win_list[znum-1], &m);
                     break;
  }

}

void keypressed_check(control_t *ctl, unsigned char ch)
{
  ctl_check_t *check = (ctl_check_t*) ctl->ctl_ptr;

  switch (ch)
  {
    case KEY_ENTER:  if (check->state) check->state = 0;
                     else check->state = 1;

                     draw_check(&screen_rect, win_list[znum-1],
                     check, 1);
                     break;
  }

}

void keypressed_input(control_t *ctl, unsigned char ch)
{
  ctl_input_t *ctl_ipt = (ctl_input_t*) ctl->ctl_ptr;
  struct ipc_message m;

  switch (ch)
  {
    case KEY_BACKSPACE:  ctl_ipt->data[strlen(ctl_ipt->data) - 1] = '\0';
						 hide_cursor();
						 draw_input(&screen_rect, win_list[znum-1], ctl_ipt, 1);
						 show_cursor();
						 break;

    case KEY_ENTER:      m.type = MSG_GUI_TEXT_CHANGED;
                         m.MSGT_GUI_TEXT_CHANGED_ITEMID = ctl->ctl_id;
                         spass_msg(win_list[znum-1], &m);
                         break;

    default:	         ctl_ipt->data[strlen(ctl_ipt->data)] = ch;
						 hide_cursor();
						 draw_input(&screen_rect, win_list[znum-1], ctl_ipt, 1);
						 show_cursor();
  }
}

void listbox_scrollup(ctl_listbox_t *ctl_l, int v)
{
    int add;

    if (ctl_l->selected > v)     // first item selected?
    {
        ctl_l->selected-=v;
        ctl_l->sb->pos-=v;

        if (ctl_l->firstshow > ctl_l->selected)  // first visible item selected?
        ctl_l->firstshow-=v;
    }

  hide_cursor();
  draw_listbox(&screen_rect, win_list[znum-1], ctl_l, 1);
  show_cursor();
}

void listbox_scrolldown(ctl_listbox_t *ctl_l, int v)
{
     if ((ctl_l->selected+v) <= ctl_l->count)  // last item selected
     {
       ctl_l->selected+=v;
       ctl_l->sb->pos+=v;

       // if last visible item was selected scroll list down
       if ((ctl_l->firstshow + ctl_l->width) <= ctl_l->selected)
        ctl_l->firstshow+=v;
     }

     hide_cursor();
     draw_listbox(&screen_rect, win_list[znum-1], ctl_l, 1);
     show_cursor();
}

void keypressed_listbox(control_t *ctl, unsigned char ch, unsigned int code)
{
  ctl_listbox_t *ctl_l = (ctl_listbox_t*) ctl->ctl_ptr;
  struct ipc_message m;

  if (code == 72)
    listbox_scrollup(ctl_l,1);
  else if (code == 80)
    listbox_scrolldown(ctl_l,1);
  else if (code == 73)           // page up
    listbox_scrollup(ctl_l,ctl_l->width);
  else if (code == 81)           // page down
    listbox_scrolldown(ctl_l,ctl_l->width);
  else if (ch==KEY_ENTER)
  {
     m.type = MSG_GUI_LISTBOX_SELECTED;
     m.MSGT_GUI_LISTBOX_ITEMID = ctl->ctl_id;
     m.MSGT_GUI_LISTBOX_DATA   = listbox_GetSelectedItemData(ctl_l);
     spass_msg(win_list[znum-1], &m);
  }
  else return;
}


void keypressed_menu(control_t *ctl, unsigned char ch, unsigned int code)
{
  rect_t rect;
  ctl_menu_t *menu;
  ctl_menuitem_t *mitem;
  struct ipc_message m;

  menu = ctl->ctl_ptr;
  mitem = menu->menuitems;

  if (ch == 56)    // ALT pressed
  {
    if (win_list[znum-1]->menu_active == 1)  // Already open, then close
    {
      close_menu(menu);
    }

    else
    {
      if (menu->active_item != NULL)     // Unselect selection
      {

        menu->active_item->active_child = 0;
        menu->active_item->expand = 0;
        menu->active_item->selected = 0;
        menu->active_item = NULL;
      }

      menu->child->selected = 1;
      menu->active_item = menu->child;

      win_list[znum-1]->menu_active = 1;

      hide_cursor();
      draw_menu(&screen_rect,win_list[znum-1],menu,1);
      show_cursor();
    }
  }

  // if menu isn't open return
  if (menu->active_item == NULL) return;

  if (code == 77)   // Cursor right
  {
    if (menu->active_item)                // An item selected?
    {
      if (menu->active_item->next)          // Another item in list?
      {
        menu->active_item->active_child = 0;
        menu->active_item->expand = 0;

        restore_menufield(win_list[znum-1],menu, menu->active_item);

        menu->active_item->selected = 0;
        menu->active_item->next->selected = 1;
        menu->active_item = menu->active_item->next;
      }

      hide_cursor();
      draw_menu(&screen_rect,win_list[znum-1],menu,1);
      show_cursor();
    }
  }
  else if (code == 75)                    // Cursor left
  {
    if (menu->active_item)                // An item selected?
    {
      if (menu->active_item->prev)          // Another item in list?
      {
        menu->active_item->active_child = 0;
        menu->active_item->expand = 0;

        restore_menufield(win_list[znum-1],menu, menu->active_item);

        menu->active_item->selected = 0;
        menu->active_item->prev->selected = 1;
        menu->active_item = menu->active_item->prev;
      }

      hide_cursor();
      draw_menu(&screen_rect,win_list[znum-1],menu,1);
      show_cursor();
    }
  }
  else if (code==80)
  {
     if (!menu->active_item) return;

     if (!menu->active_item->expand)
     {
       mitem = menu->active_item->child;
       while ((mitem) && (mitem->seperator == 1))
          mitem=mitem->next;

       if (!mitem) return;

       menu->active_item->expand = 1;
       menu->active_item->active_child = mitem;
     }

     else
     {
        if (!menu->active_item->active_child) return;

        mitem = menu->active_item->active_child->next;
        while ((mitem) && (mitem->seperator == 1))
           mitem=mitem->next;

        if (!mitem) return;

        menu->active_item->active_child = mitem;
     }

     hide_cursor();
     draw_menu(&screen_rect,win_list[znum-1],menu,1);
     show_cursor();
  }
  else if (code==72)
  {
     if (!menu->active_item) return;

     if (menu->active_item->expand)
     {
       if (!menu->active_item->active_child) return;

       mitem = menu->active_item->active_child->prev;
       while ((mitem) && (mitem->seperator == 1))
          mitem=mitem->prev;

       if (!mitem) return;

       menu->active_item->active_child = mitem;
     }

     hide_cursor();
     draw_menu(&screen_rect,win_list[znum-1],menu,1);
     show_cursor();
  }

  else if (ch == KEY_ENTER)
  {
     if (menu->active_item)
       if (menu->active_item->active_child)
       {
          m.type = MSG_GUI_CTL_MENU_ITEM;

          m.MSGT_GUI_CTL_ID = (int) menu->active_item->active_child->id;
          send_msg(win_list[znum-1]->ptask->pid, &m);

          close_menu(menu);
       }
  }

  return NULL;
}

void keypressed_control(control_t *ctl, unsigned char ch, unsigned int code)
{
  switch (ctl->ctl_type)
  {
    case CTL_BUTTON : keypressed_button(ctl,ch);
					  break;
    case CTL_CHECK  : keypressed_check(ctl,ch);
					  break;
    case CTL_INPUT :  keypressed_input(ctl,ch);
					  break;
    case CTL_LISTBOX: keypressed_listbox(ctl, ch, code);
                 break;
    case CTL_MENU   : keypressed_menu(ctl, ch, code);

  }
}
