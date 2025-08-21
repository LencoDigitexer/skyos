/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\critical.c
/* Last Update: 25.07.1999
/* Version    : beta
/* Coded by   : Gregory Hayrapetian
/* Docus from :
/************************************************************************/
/* Definition:
/*   Critical Section implementation
/************************************************************************/

#include "sched.h"
#include "critical.h"

#define NULL		(void*)0

extern struct task_struct *current;


critical_section_t *critical_sections = NULL;


void create_critical_section(critical_section_t *csect)
{
  critical_section_t *cs;
  int flags;

  csect->in_critical = 0;
  csect->ccount = 0;
  csect->next = NULL;

  save_flags(flags);
  cli();

  if (!critical_sections)
  {
    critical_sections = csect;
  }
  else
  {
    cs = critical_sections;
    while (cs->next) 
	  cs = cs->next;

	cs->next = csect;
  }

  restore_flags(flags);
}

int remove_critical_section(critical_section_t *csect)
{
  return 1;
}

void enter_critical_section(critical_section_t *csect)
{
  struct ipc_message m;
  int flags;

  save_flags(flags);
  cli();

  if (!csect->in_critical)             /* is a task in a critical section? */
  {
    csect->in_critical = current->pid; /* task in critical section */
    restore_flags(flags);
    return;
  }

  /* if a task is in a critical section, add it to the critical path */
  if (csect->ccount == MAX_CRITICAL)
  {
    show_msgf("critical.c: max critical sections reached");
    restore_flags(flags);
    return;
  }
  csect->critical_path[csect->ccount++] = current->pid;

  task_control(current->pid,TASK_SLEEPING); /* wait until critical path is free */
  Scheduler();
  restore_flags(flags);
}

void leave_critical_section(critical_section_t *csect)
{
  int flags;

  save_flags(flags);
  cli();
  
  csect->in_critical = 0;

  if (csect->ccount > 0)
  {
    int i;

	task_control(csect->critical_path[0], TASK_READY); /* wake up task which is waiting in critical path */
	
    csect->ccount--;
    for (i=0;i<csect->ccount;i++)
      csect->critical_path[i] = csect->critical_path[i+1];

    csect->in_critical = csect->critical_path[0];
  }

  restore_flags(flags);
}

critical_section_t *task_in_critical_section(int pid)
{
  critical_section_t *csect = critical_sections;
  int flags;

  save_flags(flags);
  cli();

  while (csect)
  {
    if (pid == csect->in_critical)
	{
      restore_flags(flags);
      return csect;
	}

	csect = csect->next;
  }

  restore_flags(flags);
  return NULL;
}

void remove_critical_task(int pid)
{
  critical_section_t *csect = critical_sections;
  unsigned int flags;
  int i;

  save_flags(flags);
  cli();

  while (csect)
  {
    for (i=0;i<csect->ccount;i++)
      if (csect->critical_path[i] == pid)
	  {
        csect->ccount--;
        for (;i<csect->ccount;i++)
          csect->critical_path[i] = csect->critical_path[i+1];

        restore_flags(flags);
        return 1;
	  }
	csect = csect->next;
  }

  restore_flags(flags);
  return 0;
}

void exit_critical_task(int pid)
{
  critical_section_t *csect;

  if (csect = task_in_critical_section(pid))
    leave_critical_section(csect);
  else
    remove_critical_task(pid);
}


/*
int task_in_critical_path(int pid)
{
  unsigned int flags;
  int i;

  save_flags(flags);
  cli();

  for (i=0;i<ccount;i++)
    if (critical_path[i] == pid)
    {
      restore_flags(flags);
      return 1;
    }

  restore_flags(flags);
  return 0;
}
*/




