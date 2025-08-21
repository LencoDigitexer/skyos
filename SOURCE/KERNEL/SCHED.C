/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\sched.c
/* Last Update: 09.12.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus from : Linux, Moderne Betriebssysteme
/************************************************************************/
/* Definition:
/*   Main Kernel file. Scheduling and task handling functions.
/************************************************************************/
#include "eflags.h"
#include "sched.h"
#include "head.h"
#include "rtc.h"
#include "newgui.h"

#define CLOCK_TICK_RATE	1193180         /* Underlying HZ */
#define CLOCK_TICK_FACTOR	20	/* Factor of both 1000000 and CLOCK_TICK_RATE */
#define LATCH  ((CLOCK_TICK_RATE + hz/2) / hz)	/* For divider */
#define NULL (void*)0

unsigned int scheduler_debug_wait = 0;
unsigned int system_idle = 0;
unsigned char task_time = 4;
struct task_struct *tasks[NR_PRIORITIES];
struct task_struct *current_pr_task[NR_PRIORITIES];

extern struct window_t *ps_win;
extern unsigned int kernel_page_dir;

unsigned char tasks_ready[NR_PRIORITIES] = {0};

struct task_struct dummy ={0};

struct task_struct *current = NULL;
struct task_struct *focus = NULL;

struct focus_struct *focus_list = NULL;
struct focus_struct *focus_task = NULL;

struct task_struct *GetTask(int pid);
extern void loop();

int shell1_pid = 0;

extern int MAXX;
extern int MAXY;
unsigned char *TASK_STATS[] = {
   "",
   "idle",
   "running",
   "ready",
   "stopped",
   "dead",
   "created",
   "waiting",
   "sleeping"};

unsigned char *PR_NAMES[] = {
   "Realtime",
   "High",
   "Normal",
   "Idle"};

void kernel_debug_shell(void);
void kernel_idle(void);
void floppy_task(void);
void devices_task(void);
void dump_tasks(void);

struct s_timer
{
  unsigned int global_time;
  unsigned int time;
  int pid;
  struct s_timer *next;
  struct s_timer *prev;
};

struct s_timer *sleeptimer = NULL;
struct s_timer *timer      = NULL;

int timerticks=0;
unsigned int idleticks=0;

void Scheduler(void);

int next_pid;

int tssdesc[NR_TASKS]={0};

int tasksactive = 0;

void task_call(unsigned int nr)
{
  unsigned int sel[2];
  sel[0] = 0;
  sel[1] = nr;
 __asm__("lcall %0; clts"::"m" (*sel));
}

void idle_task(void)
{
   while(1);
}

/*void task2f(void)
{
  int index = 0;
  int c = 0;
  int i;
  int j=0;
  unsigned char str[] = "sky operating system\0";
  unsigned char stime[255] = {0};
  struct time t;
  struct time t2;

  outs(550, 15, 15, str);

  while(1)
  {
     get_time(&t);
     sprintf(buf3,"%02d:%02d:%02d",t.hour, t.min, t.sec);

     if (j==0)
     {
       for (i=0;i<=1000000;i++);

       outs(550+index*8,15,1,"%c",str[index]);

       if (str[index] != 32)
         str[index] -= 32;

       outs(550+index*8,15,15,"%c",str[index]);
       index++;
       if (str[index] == '\0')
       {
         index = 0;
         j=1;
       }
     }

     else if (j==1)
     {
       for (i=0;i<=1000000;i++);

       outs(550+index*8,15,1,"%c",str[index]);

       if (str[index] != 32)
         str[index] += 32;

       outs(550+index*8,15,15,"%c",str[index]);
       index++;
       if (str[index] == '\0')
       {
         index = 0;
         j=0;
       }
     }
  }
} */


void time_task(void)
{
  int i;
  int j;
  unsigned char str[255] = {0};
  unsigned char str2[255] = {0};
  unsigned char str3[255] = {0};
  struct time t = {0};
  struct time t2 = {0};
  struct task_struct *tas;
  struct tss_struct *ts;

  while(1)
  {
     sleep(17);
     get_time(&t);

     outs(MAXX - 250,15,1,"%s",str);
     outs(MAXX - 250,MAXY - 16,7,"%s",str2);
     outs(150,MAXY - 16,7,"%s",str3);

     sprintf(str,"%02d.%02d.%0004d  %02d:%02d:%02d",t.day, t.mon, 1900 + t.year, t.hour, t.min, t.sec);
     sprintf(str2,"Free Memory: %d",get_freemem());
     sprintf(str3,"Application: %s",focus_task->task->name);

     outs(150, MAXY - 16,15,"%s",str3);
     outs(MAXX - 250,MAXY - 16,15,"%s",str2);
     outs(MAXX - 250,15,15,"%s",str);

/*     box(550,200,799,400,1,1,1);
     for (i=0;i<=20;i++)
     {
       tas = GetTask(i);
       if (tas)
       {
         ts = (struct tss_struct*) &(tas->tss);
         sprintf(str,"%02d: 0x%00000008x",tas->pid,ts->esp);
         outs(550,200+i*10, 15,"%s",str);
       }
    }*/
 }
}

/************************************************************************/
/* Sleep function
/* Sets a task into sleep until sleeptime reached.
/* Input: timerticks to sleep for, 18 means 1 second
/************************************************************************/
void sleep(unsigned int time)
{
  struct s_timer *p;
  struct s_timer *newp;
  unsigned int flags;

  save_flags(flags);
  cli();

  p = (struct s_timer*)sleeptimer;

  if (p == NULL)
  {
    sleeptimer = (struct s_timer*)valloc(sizeof(struct s_timer));
    sleeptimer->pid = current->pid;
    sleeptimer->time = timerticks + time;
    sleeptimer->next = sleeptimer->prev = NULL;
  }

  else
  {
    while ((p->time < (time + timerticks)) && (p))
      p = p->next;

    newp = (struct s_timer*)valloc(sizeof(struct s_timer));

    p->prev->next = newp;
    newp->next = p;
    newp->prev = p->prev;
    p->prev = newp;

    newp->time = timerticks + time;
    newp->pid = current->pid;
  }

  restore_flags(flags);

  task_control(current->pid, TASK_SLEEPING);
  Scheduler();

//  while (current->state == TASK_SLEEPING);
}

/************************************************************************/
/* Periodically Timer Message Sender
/* Input: timerticks between each messages
/************************************************************************/
void settimer(unsigned int time)
{
  struct s_timer *p;
  struct s_timer *newp;
  unsigned int flags;

  save_flags(flags);
  cli();

  p = (struct s_timer*)timer;

  if (p == NULL)
  {
    timer = (struct s_timer*)valloc(sizeof(struct s_timer));
    timer->pid = current->pid;
    timer->time = timerticks + time;
    timer->global_time = time;
    timer->next = timer->prev = NULL;
  }

  else
  {
    while (p->next)
      p = p->next;

    newp = (struct s_timer*)valloc(sizeof(struct s_timer));

    p->next = newp;
    newp->next = NULL;
    newp->prev = p;

    newp->time = timerticks + time;
    newp->global_time = time;
    newp->pid = current->pid;
  }

  restore_flags(flags);
}

void cleartimer(void)
{
   unsigned int flags;
   struct s_timer *p;
   struct s_timer *p2;

   save_flags(flags);
   cli();

   p = (struct s_timer*)timer;

   while (p->pid != current->pid)
   {
      p = p->next;
      if (!p) return;
   }

   if (p->prev == NULL)         // First Element
   {
      timer = p->next;
      vfree(p);
   }
   else if (p->next == NULL)    // Last Element
   {
     p2 = p;
     p->prev->next = NULL;
     vfree(p2);
   }
   else
   {
     p2 = p;
     p->prev->next = p->next;
     p->next->prev = p->prev;
     vfree(p2);
   }

   restore_flags(flags);
}

void cleartimer_pid(int pid)
{
   unsigned int flags;
   struct s_timer *p;
   struct s_timer *p2;

   save_flags(flags);
   cli();

   p = (struct s_timer*)timer;

   while (p->pid != pid)
   {
      p = p->next;
      if (!p) return;
   }

   if (p->prev == NULL)         // First Element
   {
      timer = p->next;
      vfree(p);
   }
   else if (p->next == NULL)    // Last Element
   {
     p2 = p;
     p->prev->next = NULL;
     vfree(p2);
   }
   else
   {
     p2 = p;
     p->prev->next = p->next;
     p->next->prev = p->prev;
     vfree(p2);
   }

   restore_flags(flags);
}


/************************************************************************/
/* Adds a task to the tasklist                                          */
/************************************************************************/
struct task_struct* AddTaskToQueue(struct task_struct *newtask, int pr)
{
   struct task_struct *task;

   task = tasks[pr];    // task = first task of priority pr queue

   if (!task)           // First Task in Queue
   {
     task = newtask;
     task->next = task->prev = task;
     tasks[pr] = task;

     return task;
   }

   // Add To Queue
   task->prev->next = newtask;
   newtask->prev = task->prev;
   newtask->next = task;
   task->prev = newtask;

   tasks[pr] = task;

   return newtask;
}

/************************************************************************/
/* Removes a task from the tasklist                                     */
/************************************************************************/
int RemoveTask(struct task_struct *oldtask)
{
   struct task_struct *t;
   struct task_struct *start;
   int i;
   int flags;

   save_flags(flags);
   cli();
   i = 0;

next_priority:

   if (i==NR_PRIORITIES)
   {
     vfree(oldtask->stack);
     vfree(oldtask);
     restore_flags(flags);
     return;
   }

   start = tasks[i];
   t = tasks[i];

   do
   {
     if (!t)
     {
       i++;
       goto next_priority;
     }

     if (t == oldtask)
     {
       goto remove_task;
     }
     t = t->next;
   } while (t != start);
   i++;
   if (i < NR_PRIORITIES) goto next_priority;

remove_task:
  if (t->next == t)
  {
    current_pr_task[i] = NULL;
    tasks[i] = NULL;
  }
  else
  {
    t->prev->next = t->next;
    t->next->prev = t->prev;
  }
  vfree(oldtask->stack);
  vfree(oldtask);

  restore_flags(flags);
  return 0;
}

/************************************************************************/
/* Set Taskstate to TASK_DEAD. Task will be cleared by the scheduler    */
/************************************************************************/
void TerminateTask(struct task_struct *t)
{
  t->state = TASK_DEAD;
}

/************************************************************************/
/* Creates a new kernel task (dynamically) and adds it to the queue     */
/************************************************************************/
struct task_struct* CreateKernelTask(unsigned int address, unsigned char *name, int pr, int app)
{
  struct task_struct *newtask;
  struct task_struct *t;
  struct focus_struct *newfocus;
  unsigned int stack;
  unsigned int flags;

  save_flags(flags);
  cli();

  // Attention!!! TSS must be 4byte aligned (think so...)
  t = (struct task_struct*)valloc(sizeof(struct task_struct));

  memset(t,0,sizeof(struct task_struct));

  t->pid = next_pid;
  t->pr  = pr;
  t->type = TASK_TYPE_KERNEL;

  t->state = TASK_CREATED;
  strcpy(t->name, name);

  t->tss.ds = KERNEL_DS;
  t->tss.es = KERNEL_DS;
  t->tss.fs = KERNEL_DS;
  t->tss.gs = KERNEL_DS;
  t->tss.ss = KERNEL_DS;
  t->tss.cs = KERNEL_CS;
  t->tss.cr3 = 0x1000;

  t->tss.eip = address + app;

  // Allocate Stack and align to a 4 byte boundary
  stack = (unsigned int)valloc(8192*4);
  t->stack = stack;
  stack = stack + 8192*4;
  stack = stack - (stack % 4);
  t->tss.esp = ((unsigned int)(stack)) - 32;
  t->tss.esp0 = ((unsigned int)(stack)) - 32;

  t->tss.eflags = 0x200;

  t->msgbuf = t->window = t->fl = NULL;

  newtask = AddTaskToQueue(t,pr);

  // This is old unused code, but some calls to this functions uses the app
  // parameter
  if (app)              // task has a window
  {
    if (focus_list == NULL)          // first window_task
    {
      newfocus = (struct focus_struct*)valloc(sizeof(struct focus_struct));
      newfocus->task = t;
      newfocus->next = newfocus;
      newfocus->prev = newfocus;
      focus_list = newfocus;
      focus_task = focus_list;
    }
    else
    {
      // Add To Queue
      newfocus = (struct focus_struct*)valloc(sizeof(struct focus_struct));
      newfocus->task = t;

      focus_list->prev->next = newfocus;
      newfocus->prev = focus_list->prev;
      newfocus->next = focus_list;
      focus_list->prev = newfocus;
    }
  }

  next_pid++;

  restore_flags(flags);
  return newtask;
}

/************************************************************************/
/* Creates a new   user task (dynamically) and adds it to the queue     */
/************************************************************************/
struct task_struct* CreateUserTask(void)
{
  unsigned int *p;
  struct task_struct *t;
  unsigned int addr;
  unsigned int oldaddr;
  unsigned int newaddr;

  struct task_struct *newtask;
  unsigned int flags;
  unsigned int page;
  unsigned char str[255];

  int i;

  // Attention!!! TSS must be 4byte aligned (think so...)
  t = (struct task_struct*)valloc(sizeof(struct task_struct));


  memset(t,0,sizeof(struct task_struct));

/* Now allocate a new page directory */

  show_msgf("sched.c: allocation page directory");

  addr = palloc(1);
  show_msgf("sched.c: page directory at 0x%00000008x",addr);

  t->tss.cr3 = addr;             // set pagedir address in tss

/* copy kernel page directory */
  memcpy(t->tss.cr3, 0x1000, 4096);

/* now allocate first page table */
  show_msgf("sched.c: allocation first page table");
  addr = palloc(1);
  show_msgf("sched.c: page table at 0x%00000008x",addr);

  newaddr = addr;

/* insert page table address into pagedir */
  p = t->tss.cr3;

  *p = (unsigned int) addr + 0x7;
//  *p = (unsigned int) newaddr + 0x3;

/* allocate user space */
  addr = palloc(100);

/* insert page address into page table */
  p = (unsigned int*)newaddr;

  for (i=0;i<100;i++)
  {
    *p = (unsigned int) addr + 0x7 + i*4096;
    p++;
  }

  t->code_start = addr;

/* copy dummy function to user area */
  memcpy(addr, loop, 500);

// insert kernel page tables
  p = t->tss.cr3;

  p+=768;
  *p = 0x2007;

  t->state = TASK_CREATED;

  t->pid = next_pid++;
  t->pr  = NP;
  t->type = TASK_TYPE_USER;

  t->tss.ds = USER_DS;
  t->tss.es = USER_DS;
  t->tss.fs = USER_DS;
  t->tss.gs = USER_DS;
  t->tss.ss = USER_DS;

  t->tss.ss0 = KERNEL_DS;

  t->tss.cs = USER_CS;

  t->tss.eip = 0;

  t->tss.ldt = 0;
  t->tss.bitmap = 103;

  t->tss.eflags = 0x1200;

  t->tss.ebp = t->tss.esp = 80*4096;
  t->tss.esp0 = 80*4096;

  t->stack_start = 80*4096;

  strcpy(t->name, "usertask");

  t->msgbuf = t->window = t->fl = NULL;

  kernel_page_dir = t->tss.cr3;

  AddTaskToQueue(t,NP);
  return t;

}




/************************************************************************/
/* Initialize the scheduler                                             */
/************************************************************************/
int sched_init(void)
{
  int i;

  printk("sched.c: Initializing multitasking environment...");

  next_pid = 0;

  for (i=0;i<NR_PRIORITIES;i++)
    tasks[i] = NULL;

//  shell1_pid = (CreateKernelTask((unsigned int)kernel_debug_shell , "Shell", NP,1))->pid;
//  CreateKernelTask((unsigned int)time_task , "Clock", NP,0);
  CreateKernelTask((unsigned int)idle_task , "Idle", IP,0);

  current_pr_task[RTP] = NULL;
  current_pr_task[HP] =  NULL;
  current_pr_task[NP] =  NULL;
  current_pr_task[IP] =  NULL;

  current = tasks[RTP];
  focus = (struct task_struct*) GetTask(shell1_pid);

  SetTSSDesc(&gdt[FIRST_TSS_ENTRY],&(dummy.tss),103,0x89,0);
  load_TR(0);

  set_clock(100);               // set timer clock to 100hz
}

void Scheduler(void)
{
    struct task_struct *t,*tsk;
    int i = 0;
    int j;

    // if task was running, set it to ready
    if (current->state == TASK_RUNNING)
    {
       current->state = TASK_READY;
       t = current->next;
    }

next_priority:
    if (current_pr_task[i] == NULL)   // first time to serve this queue
    {
       current_pr_task[i] = tasks[i];
       t = tasks[i];
    }
    else t = current_pr_task[i]->next;

    while (t->state != TASK_READY)             // if task not ready
    {
       if (t->state == TASK_CREATED)              // task was created
       {
           t->state = TASK_READY;

 /*          printk("Activating Task: %s PR: %d",t->name, t->pr);
           printk("Prev Task: %s", t->prev->name);
           printk("Prev Prev Task: %s", t->prev->prev->name);
           printk("Prev Next Task: %s", t->prev->next->name);
           printk("");
           printk("Next Task: %s", t->next->name);
           printk("Next Next Task: %s", t->next->next->name);
           printk("Next Prev Task: %s", t->next->prev->name);
           for (j=0;j<scheduler_debug_wait;j++);*/

           goto activate_task;
        }
        else if (t->state == TASK_DEAD)
        {
           
	   tsk = t->next;
           RemoveTask(t);
           t = tsk;
        
           if (tasks[i]==NULL)          // no more tasks in queue
           {
              i++;
              goto next_priority;
           }
        }
        else if (t->next == current_pr_task[i])      // no other task to serve
        {
           if (t->next->state == TASK_READY)   // current task still ready
           {
              t= t->next;
              goto activate_task;
           }
           i++;

           goto next_priority;
        }
        else t = t->next;
      }
      goto activate_task;

activate_task:
/*   if (system_idle)
    {
      if (i != IP)
      {
        system_idle = 0;
        outs(MAXX/2 - 30,MAXY - 16, 7, "System Idle");
      }
      else
      {
        outs(MAXX/2 - 30,MAXY - 16, 15, "System Idle");
      }
    }
    if (i==IP)
    {
      system_idle = 1;
      outs(MAXX/2 - 30,MAXY - 16, 15, "System Idle");
    }*/

  if (i == IP) idleticks++;
  current_pr_task[i] = t;
  current = t;
  current->state = TASK_RUNNING;

  current->cpu_time++;

  if (current->type == 1)               // Kernel Task
  {
    SetTSSDesc(&gdt[FIRST_TSS_ENTRY],&(current->tss),103,0x89,0);
    task_call((FIRST_TSS_ENTRY)*8);
  }
  else
  {
    SetTSSDesc(&gdt[FIRST_TSS_ENTRY],&(current->tss),103,0xe9,0);
    task_call(((FIRST_TSS_ENTRY)*8)+3);
  }

  asm("pushf ; pop %%eax ; andl $0xBFFF, %%eax ; push %%eax ;; popf":::"ax");
}

void do_timer(void)
{
  struct s_timer *p;
  unsigned int flags;
  struct ipc_message m;

  sti();
  timerticks++;

  if (sleeptimer)
  {
    if (sleeptimer->time < timerticks)
    {
      save_flags(flags);
      cli();

      p = sleeptimer;
      task_control(p->pid, TASK_READY);
      sleeptimer = sleeptimer->next;
      if (vfree(p) == -1)
      {
        printk("sched.c: OOOPS!! unable to free sleeptimer");
        printk("         System halted!");
        while(1);
      }

      restore_flags(flags);
     }
    }


  if (timer)
  {
    p = timer;
    while (p)
    {
      if (p->time < timerticks)
      {
        save_flags(flags);
        cli();

        m.type = MSG_TIMER;
        send_msg(p->pid, &m);

        p->time = timerticks + p->global_time;
      }
      p = p->next;
    }
  }

  if (!(timerticks % 18)) rtq_update();

  if (tasksactive) Scheduler();
}

void task_control(int pid, int state)
{
   struct task_struct *t;
   struct task_struct *start;
   int i;
   int flags;

   save_flags(flags);
   cli();
   i = 0;

next_priority:

   if (i==NR_PRIORITIES)
   {
     restore_flags(flags);
     return;
   }

   start = tasks[i];
   t = tasks[i];

   do
   {
     if (!t)
     {
       i++;
       goto next_priority;
     }

     if (t->pid == pid)
     {
       t->state = state;
       if (state == TASK_READY)
         tasks_ready[i] = 1;

       goto tc_finished;
     }
     t = t->next;
   } while (t != start);
   i++;
   if (i < NR_PRIORITIES) goto next_priority;

tc_finished:
  restore_flags(flags);
}

struct task_struct *GetTask(int pid)
{
   struct task_struct *t;
   struct task_struct *start;
   int i;
   int flags;

   save_flags(flags);
   cli();
   i = 0;

next_priority:

   if (i==NR_PRIORITIES)
   {
     restore_flags(flags);
     return NULL;
   }

   start = tasks[i];
   t = tasks[i];

   do
   {
     if (!t)
     {
       i++;
       goto next_priority;
     }

     if (t->pid == pid)
     {
       goto task_found;
     }
     t = t->next;
   } while (t != start);
   i++;
   if (i < NR_PRIORITIES) goto next_priority;
   return NULL;

task_found:
   restore_flags(flags);
   return t;
}

void exit(int code)
{
  current->state = TASK_DEAD;
  while(1);      // wait until we get terminated
}


unsigned int exec(unsigned char *device, unsigned char *name)
{

}
