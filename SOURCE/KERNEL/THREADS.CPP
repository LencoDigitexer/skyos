
/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\threads.cpp
/* Last Update: 16.03.1999
/* Version    :
/* Coded by   : Szeleney Robert
/************************************************************************/
/* Definition:
/*   Threads Module
/************************************************************************/
#include "sched.h"
#include "page.h"
#include "stdlib.h"

extern "C" void threads_module(void);

extern unsigned int kernel_page_dir;
extern unsigned int next_pid;

class CTask
{
  private:
    struct task_struct t;
    unsigned char name[255];
    CArchPageTable *pagedir;

  public:
    CTask();
    void Create(void);
};

CTask::CTask()
{
}

void CTask::Create(void)
{
  int i;
  unsigned int userspace;

  // Allocate new page directory
  pagedir = new CArchPageTable();
  pagedir->Alloc();

  // Allocate first page table
  CArchPageTable pagetable0;
  pagetable0.Alloc();

  pagedir->Insert(0,&pagetable0);

  // Allocate pages
  CArchPage page;

  page.Alloc();
  pagetable0.Insert(0, &page);
  userspace = page.GetBase();

  for (i=1;i<100;i++)
  {
    page.Alloc();
    pagetable0.Insert(i, &page);
  }

/* now fill the new tss structure */

  t.tss.cr3 = pagedir->Format();

  t.state = TASK_CREATED;

  t.pid = next_pid++;
  t.pr  = NP;
  t.type = TASK_TYPE_USER;

  t.tss.ds = USER_DS;
  t.tss.es = USER_DS;
  t.tss.fs = USER_DS;
  t.tss.gs = USER_DS;
  t.tss.ss = USER_DS;

  t.tss.ss0 = KERNEL_DS;

  t.tss.cs = USER_CS;

  t.tss.eip = 0;

  t.tss.ldt = 0;
  t.tss.bitmap = 103;

  t.tss.eflags = 0x200;

  t.tss.ebp = t.tss.esp = 80*4096;
  t.tss.esp0 = 80*4096;

  t.stack_start = 80*4096;

  strcpy(t.name, "usertask");

  t.msgbuf = NULL;
  t.window =  NULL;
  t.fl =  NULL;

/* now load programm */

  unsigned int  entry = binary_aout_load(userspace, "a.out");

  t.tss.eip = entry;

  show_msgf("threads.cpp: EIP: %d", entry);
  show_msgf("threads.cpp: absolute kernel entry: %d",userspace + entry);

  kernel_page_dir = pagedir->GetBase();

  unsigned int flags;

  save_flags(flags);
  cli();

  AddTaskToQueue(&t,NP);

  restore_flags(flags);

}

void threads_module(void)
{
  CTask t;

  t.Create();
}




