/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\taskman.c
/* Last Update: 03.02.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the In-Time Task Creater.
/************************************************************************/
#include "head.h"
#include "newgui.h"
#include "sched.h"
#include "controls.h"
#include "module.h"
#include "msg_ctl.h"
#include "eflags.h"
#include "fcntl.h"
#include "coff.h"

#define ID_EXIT         1
#define ID_EXECUTE      2
#define ID_CREATE       3
#define ID_LOAD         4

#define ID_MENU_FILE        5
#define ID_MENU_ABOUT       6
#define ID_MENU_FILE_EXIT   7
#define ID_MENU_ABOUT_ABOUT 8

#define ID_INPUT_TASKNAME               50
#define ID_INPUT_PID                    51
#define ID_INPUT_STATE                  52
#define ID_INPUT_CS                     53
#define ID_INPUT_DS                     54
#define ID_INPUT_ES                     55
#define ID_INPUT_FS                     56
#define ID_INPUT_GS                     57
#define ID_INPUT_SS                     58
#define ID_INPUT_ESP                    59
#define ID_INPUT_CR3                    60
#define ID_INPUT_IF                     61
#define ID_INPUT_EIP                    62
#define ID_INPUT_TYPE                   63
#define ID_INPUT_LDT                    64
#define ID_INPUT_FILE                   65


window_t *win_taskman;
ctl_button_t *button_exit;
ctl_button_t *button_execute;
ctl_button_t *button_create;
ctl_button_t *button_load;


ctl_input_t *input_cr3;
ctl_input_t *input_taskname;
ctl_input_t *input_pid;
ctl_input_t *input_state;
ctl_input_t *input_cs;
ctl_input_t *input_ds;
ctl_input_t *input_es;
ctl_input_t *input_fs;
ctl_input_t *input_gs;
ctl_input_t *input_ss;
ctl_input_t *input_esp;
ctl_input_t *input_cr3;
ctl_input_t *input_if;
ctl_input_t *input_eip;
ctl_input_t *input_type;
ctl_input_t *input_ldt;
ctl_input_t *input_file;

ctl_menu_t *menu;

extern void loop();

#define win_out(a,b,c,d) win_outs(win_taskman, a, b, c, d, COLOR_LIGHTGRAY)

struct task_struct t = {0};

unsigned int taskman_page_table = 0;
unsigned int taskman_user_space = 0;

extern int next_pid;

unsigned char taskman_helpmsg[255] = {0};

extern unsigned int kernel_page_dir;

/*
#define _syscall0(type,name)				\
type name(void)								\
{											\
  long __res;								\
   __asm__ volatile ("int $0x80"			\
	                  : "=a" (__res)		\
	                  : "0" (__NR_##name)); \
  if (__res >= 0)							\
	return (type) __res;					\
  errno = -__res;							\
  return -1;								\
}
*/

void sc_test()
{
  long res;

  __asm__ volatile ("int $0x80"			
                    : "=a" (res)
   					: "0" (1234), "b" (5678));
}


int _create_window(int x, int y, char *name)
{
  int res;

  __asm__ volatile ("int $0x80"			
                    : "=a" (res)
  					: "0" (1), "b" (x), "c" (y), "d" (name));

  return res;
}

int _show_msg(char *str)
{
  long res;

  __asm__ volatile ("int $0x80"			
                    : "=a" (res)
   					: "0" (2), "b" (str));

  return res;
}

int _draw_hline(unsigned int win, int x, int y)
{
  long res;

  __asm__ volatile ("int $0x80"			
                    : "=a" (res)
  					: "0" (3), "b" (win), "c" (x), "d" (y));

  return res;
}

void user_task()
{
  int win, i=0;

//  win = _create_window(100,200,"User Window");
  _show_msg("test");

  while (1) 
  {
//	i = i;

//	_draw_hline(win,10,i);
	if (++i> 200) i = 0;
  }

}

void user_task2()
{
  int win,j=0;

//  _show_msg("user2");

//  j++;

  while (1)
  {
	if (++j> 200) j = 0;
	j = j;
  }
}


void taskman_Create(void)
{
  unsigned int *p;
  unsigned int addr;
  unsigned int oldaddr;
  unsigned int newaddr;

  struct task_struct *newtask;
  unsigned int flags;
  unsigned int page;
  unsigned char str[255];

  int i;



/* now set dialog items */

/* general */
}



void taskman_Load(void)
{
  unsigned char name[255];
  unsigned int entry;

  CreateUserTask(user_task2,"usertask2",NP);

/*
  GetItemText_input(win_taskman, input_file, name);
  memset(taskman_user_space, 0, 512);
  entry = binary_aout_load(taskman_user_space, name);

  t.tss.eip = entry;

  show_msgf("taskman.c: EIP: %d", entry);
  show_msgf("taskman.c: absolute kernel entry: %d",taskman_user_space + entry);

  sprintf(name,"0x%00000008X / %d",t.tss.eip, t.tss.eip);
  SetItemText_input(win_taskman, input_eip, name);
*/
}

void taskman_Execute(void)
{
  unsigned int flags;

  CreateUserTask(loop,"usertask",NP);
/*
  save_flags(flags);
  cli();

  AddTaskToQueue(&t,NP);

  restore_flags(flags);
*/
}

/* GUI functions */

void redraw_taskman(void)
{
  unsigned char str[255];

  win_out(20, 20, "Taskname:",COLOR_BLACK);
  win_out(20, 40, "PID:",COLOR_BLACK);
  win_out(20, 60, "State:",COLOR_BLACK);
  win_out(20, 80, "Type:",COLOR_BLACK);
  win_out(20, 120, "CS:",COLOR_BLACK);
  win_out(20, 140, "DS:",COLOR_BLACK);
  win_out(20, 160, "ES:",COLOR_BLACK);
  win_out(20, 180, "FS:",COLOR_BLACK);
  win_out(20, 200, "GS:",COLOR_BLACK);
  win_out(20, 220, "SS:",COLOR_BLACK);
  win_out(20, 240, "ESP:",COLOR_BLACK);
  win_out(280, 120, "CR3:", COLOR_BLACK);
  win_out(280, 140, "IF:", COLOR_BLACK);
  win_out(280, 160, "EIP:", COLOR_BLACK);
  win_out(280, 180, "LDT:", COLOR_BLACK);
  win_out(280, 220, "File:", COLOR_BLACK);

  win_out(20,250, taskman_helpmsg, COLOR_BLACK);
}

void task_taskman()
{
  struct ipc_message *m;
  unsigned int flags;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  win_taskman = create_window("SKY In-Time Task Creater",50,50,520,350,WF_STANDARD);

  /* !!! id should be > 0  !!! */
  button_exit = create_button(win_taskman,600,270,80,25,"Exit",ID_EXIT);
  button_execute = create_button(win_taskman,100,270,80,25,"Execute",ID_EXECUTE);
  button_create  = create_button(win_taskman,230,270,80,25,"Create",ID_CREATE);
  button_load  = create_button(win_taskman,360,270,80,25,"Load",ID_LOAD);

  input_taskname = create_input(win_taskman, 110, 18, 100, 14, ID_INPUT_TASKNAME);
  input_pid = create_input(win_taskman, 110, 38, 100, 14, ID_INPUT_PID);

  input_state = create_input(win_taskman, 110, 58, 100, 14, ID_INPUT_STATE);
  input_type = create_input(win_taskman, 110, 78, 100, 14, ID_INPUT_TYPE);

  input_cs = create_input(win_taskman, 60, 118, 180, 14, ID_INPUT_CS);
  input_ds = create_input(win_taskman, 60, 138, 180, 14, ID_INPUT_DS);
  input_es = create_input(win_taskman, 60, 158, 180, 14, ID_INPUT_ES);
  input_fs = create_input(win_taskman, 60, 178, 180, 14, ID_INPUT_FS);
  input_gs = create_input(win_taskman, 60, 198, 180, 14, ID_INPUT_GS);
  input_ss = create_input(win_taskman, 60, 218, 180, 14, ID_INPUT_SS);
  input_esp = create_input(win_taskman, 60, 238, 180, 14, ID_INPUT_ESP);
  input_cr3 = create_input(win_taskman, 330, 118, 180, 14, ID_INPUT_CR3);
  input_if = create_input(win_taskman, 330, 138, 180, 14, ID_INPUT_IF);
  input_eip = create_input(win_taskman, 330, 158, 180, 14, ID_INPUT_EIP);
  input_ldt = create_input(win_taskman, 330, 178, 180, 14, ID_INPUT_LDT);
  input_file = create_input(win_taskman, 330, 218, 180, 14, ID_INPUT_FILE);

  menu = create_menu(win_taskman,4);
  add_menuitem(win_taskman, menu, "File", ID_MENU_FILE, 0);
  add_menuitem(win_taskman, menu, "About", ID_MENU_ABOUT, 0);

  add_menuitem(win_taskman, menu, "Exit", ID_MENU_FILE_EXIT, ID_MENU_FILE);
  add_menuitem(win_taskman, menu, "(c) '99 by Szeleney Robert", ID_MENU_ABOUT_ABOUT, ID_MENU_ABOUT);

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            switch (m->MSGT_GUI_CTL_ID)
            {
               case ID_CREATE:
                      taskman_Create();
/*                      set_wflag(win_taskman, WF_STOP_OUTPUT, 0);
                      set_wflag(win_taskman, WF_MSG_GUI_REDRAW, 0);
                      redraw_taskman();*/
                      break;

                case ID_EXIT:
                      destroy_app(win_taskman);
                      break;

                case ID_LOAD:
                       taskman_Load();
                       break;

                case ID_EXECUTE:
                       taskman_Execute();
                       break;
            }
            break;

      case MSG_GUI_CTL_MENU_ITEM:
           switch (m->MSGT_GUI_CTL_ID)
           {
              case ID_MENU_FILE_EXIT:
                      destroy_app(win_taskman);
                      break;
           }
           break;

      case MSG_GUI_TEXT_CLICKED:
           {
          }

//       case MSG_GUI_TEXT_CHANGED:
//            switch (m->MSGT_GUI_TEXT_CHANGED_ITEMID)
//            {
//            }
//            break;


       case MSG_GUI_REDRAW:
            set_wflag(win_taskman, WF_STOP_OUTPUT, 0);
            set_wflag(win_taskman, WF_MSG_GUI_REDRAW, 0);
            redraw_taskman();
            break;

       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_taskman);
            break;

     }
  }
}

void app_taskman()
{
  if (!valid_app(win_taskman))
  {
    CreateKernelTask(task_taskman,"taskman",NP,0);
  }
}

#if (MODULE_TASKMAN == 1)
void init(void)
{
   app_taskman();
   exit(1);
   while(1);
}
#endif

