/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\sched.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Scheduler include file.
/************************************************************************/
#include "system.h"
#include "msg.h"

#define TASK_IDLE     1
#define TASK_RUNNING  2
#define TASK_READY    3
#define TASK_STOPPED  4
#define TASK_DEAD     5
#define TASK_CREATED  6
#define TASK_WAITING  7
#define TASK_SLEEPING 8

#define TASK_TYPE_KERNEL 1
#define TASK_TYPE_USER   2

#define IO_BITMAP_SIZE	32
#define NR_TASKS 10

#define MAX_KEYS_IN_BUFFER      100

struct fpu_reg {
	char sign;
	char tag;
	long exp;
	unsigned sigl;
	unsigned sigh;
};

struct i387_hard_struct {
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	st_space[20];	/* 8*10 bytes for each FP-reg = 80 bytes */
};

struct i387_soft_struct {
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long    top;
	struct fpu_reg	regs[8];	/* 8*16 bytes for each FP-reg = 128 bytes */
	unsigned char	lookahead;
	struct info	*info;
	unsigned long	entry_eip;
};

union i387_union {
	struct i387_hard_struct hard;
	struct i387_soft_struct soft;
};

struct tss_struct {
	unsigned short	back_link,__blh;
	unsigned int 	esp0;
	unsigned short	ss0,__ss0h;
	unsigned int 	esp1;
	unsigned short	ss1,__ss1h;
	unsigned int 	esp2;
	unsigned short	ss2,__ss2h;
	unsigned int 	cr3;
	unsigned int 	eip;
	unsigned int 	eflags;
	unsigned int 	eax,ecx,edx,ebx;
	unsigned int 	esp;
	unsigned int 	ebp;
	unsigned int 	esi;
	unsigned int 	edi;
	unsigned short	es, __esh;
	unsigned short	cs, __csh;
	unsigned short	ss, __ssh;
	unsigned short	ds, __dsh;
	unsigned short	fs, __fsh;
	unsigned short	gs, __gsh;
	unsigned short	ldt, __ldth;
	unsigned short	trace, bitmap;
//	unsigned long	io_bitmap[IO_BITMAP_SIZE+1];
//	unsigned long	tr;
//	unsigned long	cr2, trap_no, error_code;
//	union i387_union i387;
};

#define FIRST_TSS_ENTRY 8
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
#define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
#define _LDT(n) ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3))
#define load_TR(n) __asm__("ltr %%ax": /* no output */ :"a" (_TSS(n)))

//#define switch_to(n) __asm__("lcall $0,%%eax"::"a" (n))

#define load_ldt(n) __asm__("lldt %%ax": /* no output */ :"a" (_LDT(n)))
#define store_TR(n) \
__asm__("str %%ax\n\t" \
	"subl %2,%%eax\n\t" \
	"shrl $4,%%eax" \
	:"=a" (n) \
	:"0" (0),"i" (FIRST_TSS_ENTRY<<3))

struct task_struct
{
	struct tss_struct tss;                  // TSS struct for task
	volatile long state;                    // current state of task
        int pid;                                // processid
        int pr;                                 // priority
        int type;                               // unused
        unsigned char name[40];                 // name of task
        unsigned int savedflags;                // last saved flags of task

        struct task_struct *next;               // pointer to next task
        struct task_struct *prev;

        unsigned int waitingfor;                // task waits for xx task
        struct ipc_message *msgbuf;             // Messagebuffer for task

        unsigned char KeyBuffer[MAX_KEYS_IN_BUFFER];
        unsigned int last_code;                 // last keycode offset
        unsigned int last_read_code;            // last read keycode offset

        struct twm_window *window;              // pointer to window for task

        unsigned int cpu_time;                  // cpu_time in ticks
        unsigned int last_time;                 // ticks since last running

        struct list *fl;

	unsigned int stack;

        unsigned int code_start;
        unsigned int code_end;

        unsigned int data_start;
        unsigned int data_end;

        unsigned int stack_start;
        unsigned int stack_end;

        unsigned char actual_device[255];
};

struct focus_struct
{
  struct task_struct *task;
  struct focus_struct *next;
  struct focus_struct *prev;
};

/*
 * Per process flags
 */
#define PF_ALIGNWARN	0x00000001	/* Print alignment warning msgs */
					/* Not implemented yet, only for 486*/
#define PF_PTRACED	0x00000010	/* set if ptrace (0) has been called. */
#define PF_TRACESYS	0x00000020	/* tracing system calls */

#define NR_PRIORITIES 4
#define RTP 0
#define HP  1
#define NP  2
#define IP  3

#define KERNEL_TASK 1
#define USER_TASK   2

