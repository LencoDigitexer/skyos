/************************************************************************/
/* Sky Operating System V2
/* Copyright (c)1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\tasks\ps.c
/* Last Update: 26.12.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*  Tasks which shows the current process status
/************************************************************************/
#include "sched.h"
#include "error.h"
#include "newgui.h"

#include "controls.h"

#define win_out(a,b,c,d) win_outs(ps_win, a, b, c, d, COLOR_LIGHTGRAY)

#define ID_INPUT_TASKNAME  100
#define ID_INPUT_CS        101
#define ID_INPUT_DS        102
#define ID_INPUT_ES        103
#define ID_INPUT_FS        104
#define ID_INPUT_GS        105
#define ID_INPUT_EAX       110
#define ID_INPUT_EBX       111
#define ID_INPUT_ECX       112
#define ID_INPUT_EDX       113
#define ID_INPUT_CR3       115
#define ID_INPUT_EIP       116
#define ID_INPUT_SS        120
#define ID_INPUT_ESP       121
#define ID_INPUT_ESP0      122
#define ID_INPUT_START_ESP 123
#define ID_INPUT_CODE_START   130
#define ID_INPUT_CODE_END     131

#define ID_INPUT_TIME_CPU     150
#define ID_INPUT_TIME_LAST    151

#define ID_INPUT_STATE        200

#define ID_IDLE                  300
#define ID_RUNNING               301
#define ID_READY                 302
#define ID_STOPPED               303
#define ID_DEAD                  304
#define ID_CREATED               305
#define ID_WAITING               306
#define ID_SLEEPING              307

#define ID_NEXT 1
#define ID_PREV 2
#define ID_REFRESH 3

#define ID_MENU_FILE        5
#define ID_MENU_ABOUT       6
#define ID_MENU_FILE_EXIT   7
#define ID_MENU_ABOUT_ABOUT 8

ctl_input_t *ps_input_taskname;
ctl_input_t *ps_input_cs;
ctl_input_t *ps_input_ds;
ctl_input_t *ps_input_es;
ctl_input_t *ps_input_fs;
ctl_input_t *ps_input_gs;
ctl_input_t *ps_input_eax;
ctl_input_t *ps_input_ebx;
ctl_input_t *ps_input_ecx;
ctl_input_t *ps_input_edx;
ctl_input_t *ps_input_cr3;
ctl_input_t *ps_input_eip;
ctl_input_t *ps_input_ss;
ctl_input_t *ps_input_esp;
ctl_input_t *ps_input_esp0;
ctl_input_t *ps_input_start_esp;
ctl_input_t *ps_input_code_start;
ctl_input_t *ps_input_code_end;
ctl_input_t *ps_input_state;
ctl_input_t *ps_input_time_cpu;
ctl_input_t *ps_input_time_last;

ctl_button_t *state_idle;
ctl_button_t *state_running;
ctl_button_t *state_ready;
ctl_button_t *state_created;
ctl_button_t *state_dead;
ctl_button_t *state_waiting;
ctl_button_t *state_sleeping;
ctl_button_t *state_stopped;

// #define FTP_DEBUG 1
struct window_t *ps_win;

int ps_redraw(unsigned int tasknr)
{

  volatile struct task_struct *t;
  volatile struct tss_struct *tss;
  struct focus_struct *f;
  int i,j,k;
  char str[255];
  char str2[255];
  int num;
  int flags;
  unsigned int mem;
  unsigned int messages;
  struct ipc_message *p;
  int lc = 0;
  int pid = -1;


  win_out(20, 40, "Taskname:",COLOR_BLACK);
  win_out(20, 100, "CS:",COLOR_BLACK);
  win_out(20, 120, "DS:",COLOR_BLACK);
  win_out(20, 140, "ES:",COLOR_BLACK);
  win_out(20, 160, "FS:",COLOR_BLACK);
  win_out(20, 180, "GS:",COLOR_BLACK);
  win_out(20, 200, "EAX:",COLOR_BLACK);
  win_out(20, 220, "EBX:",COLOR_BLACK);
  win_out(20, 240, "ECX:",COLOR_BLACK);
  win_out(20, 260, "EDX:",COLOR_BLACK);
  win_out(20, 280, "CR3:",COLOR_BLACK);
  win_out(20, 300, "EIP:",COLOR_BLACK);
  win_out(20, 360, "SS:",COLOR_BLACK);
  win_out(20, 380, "ESP:",COLOR_BLACK);
  win_out(20, 400, "ESP0:",COLOR_BLACK);
  win_out(20, 420, "Start ESP:",COLOR_BLACK);
  win_out(20, 500, "State:",COLOR_BLACK);
  win_out(450, 100, "CPU Time:",COLOR_BLACK);
  win_out(450, 120, "Last Time:",COLOR_BLACK);

  cli();

  t = GetTask(tasknr);

  if (t == NULL) t = GetTask(0);

  SetItemText_input(ps_win, ps_input_taskname, t->name);

  sprintf(str,"%00000008X", t->tss.cs);
  SetItemText_input(ps_win, ps_input_cs, str);
  sprintf(str,"%00000008X", t->tss.ds);
  SetItemText_input(ps_win, ps_input_ds, str);
  sprintf(str,"%00000008X", t->tss.es);
  SetItemText_input(ps_win, ps_input_es, str);
  sprintf(str,"%00000008X", t->tss.fs);
  SetItemText_input(ps_win, ps_input_fs, str);
  sprintf(str,"%00000008X", t->tss.gs);
  SetItemText_input(ps_win, ps_input_gs, str);

  sprintf(str,"%00000008X", t->tss.eax);
  SetItemText_input(ps_win, ps_input_eax, str);
  sprintf(str,"%00000008X", t->tss.ebx);
  SetItemText_input(ps_win, ps_input_ebx, str);
  sprintf(str,"%00000008X", t->tss.ecx);
  SetItemText_input(ps_win, ps_input_ecx, str);
  sprintf(str,"%00000008X", t->tss.edx);
  SetItemText_input(ps_win, ps_input_edx, str);
  sprintf(str,"%00000008X", t->tss.cr3);
  SetItemText_input(ps_win, ps_input_cr3, str);
  sprintf(str,"%00000008X", t->tss.eip);
  SetItemText_input(ps_win, ps_input_eip, str);

  sprintf(str,"%00000008X", t->tss.ss);
  SetItemText_input(ps_win, ps_input_ss, str);
  sprintf(str,"%00000008X", t->tss.esp);
  SetItemText_input(ps_win, ps_input_esp, str);
  sprintf(str,"%00000008X", t->tss.esp0);
  SetItemText_input(ps_win, ps_input_esp0, str);
  sprintf(str,"%00000008X", t->stack_start);
  SetItemText_input(ps_win, ps_input_start_esp, str);
  sprintf(str,"%00000008X", t->state);
  SetItemText_input(ps_win, ps_input_state, str);

  sprintf(str,"%d", t->cpu_time);
  SetItemText_input(ps_win, ps_input_time_cpu, str);
  sprintf(str,"%d", t->last_time);
  SetItemText_input(ps_win, ps_input_time_last, str);

  sti();

/*
  sprintf(str,"PID  | Priority | Task name                | Task state | WF | CPU Time | Memory | Messages");
  win_outs(ps_win, 10, 10+(lc++)*10, str,8, 255);

  win_draw_fill_rect(ps_win,10,60,1000,400,7);

  save_flags(flags);
  cli();

  for (i=0;i<NR_PRIORITIES;i++)
  {
    t = tasks[i];
    if (!t) continue;
    do
    {
       p =(struct ipc_message*)t->msgbuf;
       messages = 0;

       while (p)
       {
         messages++;
         p = p->next;
       }

       mem = getmem_pid(t->pid);
       sprintf(str," %02d  | %8s | %20s | %10s | %02d | %8d | %6d | %3d",
               t->pid, PR_NAMES[t->pr], t->name, TASK_STATS[t->state], t->waitingfor,
               t->cpu_time, mem, messages);

       if ((y > (10+lc*10)) && (y < (10+(lc+1)*10)))
           pid = t->pid;

       win_outs(ps_win, 10, 10+(lc++)*10, str,8, 255);

       t = t->next;
    } while (t != tasks[i]);
  }

  restore_flags(flags);

  return pid;*/
}

int show_messages(int pid)
{
/*  struct task_struct *t;
  struct ipc_message *msgbuf;
  unsigned char str[255] = {0};
  unsigned char str2[255] = {0};
  unsigned int flags;

  save_flags(flags);
  cli();

  t = (struct task_struct *)GetTask(pid);

  msgbuf = t->msgbuf;

  while (msgbuf)
  {
    sprintf(str, "%d ",msgbuf->type);
    strcat(str2, str);
    msgbuf = msgbuf->next;
  }

  alert("Messages:                               \n\n%s\n",str2);

  restore_flags(flags);*/
}

void ps_task(void)
{
  struct ipc_message *m;
  unsigned int ret;
  unsigned int y;
  int i=0;
  int task;
  ctl_button_t *button_next;
  ctl_button_t *button_prev;
  ctl_button_t *button_refresh;

  ctl_menu_t *menu;


  int tasknr = 0;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  ps_win = create_window("Process Status",20,100,1000,680,WF_STANDARD);

  /* !!! id should be > 0  !!! */
  button_next = create_button(ps_win,100,650,80,25,"Prev",ID_PREV);
  button_prev = create_button(ps_win,300,650,80,25,"Refresh",ID_REFRESH);
  button_refresh = create_button(ps_win,500,650,80,25,"Next",ID_NEXT);

  ps_input_taskname = create_input(ps_win, 120, 18, 200, 14, ID_INPUT_TASKNAME);
  ps_input_cs  = create_input(ps_win, 120, 98, 200, 14, ID_INPUT_CS);
  ps_input_ds  = create_input(ps_win, 120, 118, 200, 14, ID_INPUT_DS);
  ps_input_es  = create_input(ps_win, 120, 138, 200, 14, ID_INPUT_ES);
  ps_input_fs  = create_input(ps_win, 120, 158, 200, 14, ID_INPUT_FS);
  ps_input_gs  = create_input(ps_win, 120, 178, 200, 14, ID_INPUT_GS);
  ps_input_eax  = create_input(ps_win, 120, 198, 200, 14, ID_INPUT_EAX);
  ps_input_ebx  = create_input(ps_win, 120, 218, 200, 14, ID_INPUT_EBX);
  ps_input_ecx  = create_input(ps_win, 120, 238, 200, 14, ID_INPUT_ECX);
  ps_input_edx  = create_input(ps_win, 120, 258, 200, 14, ID_INPUT_EDX);
  ps_input_cr3  = create_input(ps_win, 120, 278, 200, 14, ID_INPUT_CR3);
  ps_input_eip  = create_input(ps_win, 120, 298, 200, 14, ID_INPUT_EIP);

  ps_input_ss  = create_input(ps_win, 120, 358, 200, 14, ID_INPUT_SS);
  ps_input_esp  = create_input(ps_win, 120, 378, 200, 14, ID_INPUT_ESP);
  ps_input_esp0  = create_input(ps_win, 120, 398, 200, 14, ID_INPUT_ESP0);
  ps_input_start_esp  = create_input(ps_win, 120, 418, 200, 14, ID_INPUT_START_ESP);

  ps_input_time_cpu  = create_input(ps_win, 570, 98, 200, 14, ID_INPUT_TIME_CPU);
  ps_input_time_last = create_input(ps_win, 570, 118, 200, 14, ID_INPUT_TIME_LAST);

  ps_input_state  = create_input(ps_win, 120, 498, 200, 14, ID_INPUT_STATE);
  state_idle     = create_button(ps_win, 340,498, 100, 14,"idle",ID_IDLE);
  state_running  = create_button(ps_win, 340,514, 100, 14,"running",ID_RUNNING);
  state_ready    = create_button(ps_win, 340,530, 100, 14,"ready",ID_READY);
  state_stopped  = create_button(ps_win, 340,546, 100, 14,"stopped",ID_STOPPED);
  state_dead     = create_button(ps_win, 340,562, 100, 14,"dead",ID_DEAD);
  state_created  = create_button(ps_win, 340,578, 100, 14,"created",ID_CREATED);
  state_waiting  = create_button(ps_win, 340,594, 100, 14,"waiting",ID_WAITING);
  state_sleeping = create_button(ps_win, 340,610, 100, 14,"sleeping",ID_SLEEPING);

  menu = create_menu(ps_win,4);
  add_menuitem(ps_win, menu, "File", ID_MENU_FILE, 0);
  add_menuitem(ps_win, menu, "About", ID_MENU_ABOUT, 0);

  add_menuitem(ps_win, menu, "Exit", ID_MENU_FILE_EXIT, ID_MENU_FILE);
  add_menuitem(ps_win, menu, "(c) '99 by Szeleney Robert", ID_MENU_ABOUT_ABOUT, ID_MENU_ABOUT);

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            switch (m->MSGT_GUI_CTL_ID)
            {
               case ID_NEXT:
                      tasknr++;
                      set_wflag(ps_win, WF_STOP_OUTPUT, 0);
	              set_wflag(ps_win, WF_MSG_GUI_REDRAW, 0);
                      ps_redraw(tasknr);
                      break;

                case ID_PREV:
                      tasknr--;
                      set_wflag(ps_win, WF_STOP_OUTPUT, 0);
	              set_wflag(ps_win, WF_MSG_GUI_REDRAW, 0);
                      ps_redraw(tasknr);
                      break;

                case ID_REFRESH:
                      set_wflag(ps_win, WF_STOP_OUTPUT, 0);
	              set_wflag(ps_win, WF_MSG_GUI_REDRAW, 0);
                      ps_redraw(tasknr);
                      break;

                case ID_IDLE:
                case ID_RUNNING:
                case ID_READY:
                case ID_STOPPED:
                case ID_DEAD:
                case ID_CREATED:
                case ID_WAITING:
                case ID_SLEEPING:
                     {
                       task_control(tasknr,m->MSGT_GUI_CTL_ID - 300);
                     }
                     break;
            }
            break;

       case MSG_GUI_REDRAW:
            set_wflag(ps_win, WF_STOP_OUTPUT, 0);
	    set_wflag(ps_win, WF_MSG_GUI_REDRAW, 0);
            ps_redraw(tasknr);
            break;

       case MSG_GUI_WINDOW_CLOSE:
            destroy_app(ps_win);
            break;
     }
  }
}

void ps(void)
{
  /* Console Application */
  if (!valid_app(ps_win))
    CreateKernelTask(ps_task,"ps",NP,0);
}


