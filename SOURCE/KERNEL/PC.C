/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\pc.c
/* Last Update: 04.11.1998
/* Version    : release
/* Coded by   : Szeleney Robert
/* Docus from : PC Intern 5
/************************************************************************/
/* Definition:
/*   Functions to programm a pc. (INTEL)
/*   outb, inb, outw, inw, real time clock access,...
/************************************************************************/

#include "devman.h"
#include "rtc.h"
#include "cpuid.h"
#include "eflags.h"

#define X86_VENDOR_INTEL 0
#define X86_VENDOR_CYRIX 1
#define X86_VENDOR_AMD 2
#define X86_VENDOR_UMC 3
#define X86_VENDOR_NEXGEN 4
#define X86_VENDOR_CENTAUR 5
#define X86_VENDOR_UNKNOWN 0xff

struct cpu_model_info {
	int vendor;
	int x86;
	char *model_names[16];
};

#define NULL (void*)0

static struct cpu_model_info cpu_models[] = {
	{ X86_VENDOR_INTEL,	4,
	  { "486 DX-25/33", "486 DX-50", "486 SX", "486 DX/2", "486 SL", 
	    "486 SX/2", NULL, "486 DX/2-WB", "486 DX/4", "486 DX/4-WB", NULL, 
	    NULL, NULL, NULL, NULL, NULL }},
	{ X86_VENDOR_INTEL,	5,
	  { "Pentium 60/66 A-step", "Pentium 60/66", "Pentium 75 - 200",
	    "OverDrive PODP5V83", "Pentium MMX", NULL, NULL,
	    "Mobile Pentium 75 - 200", "Mobile Pentium MMX", NULL, NULL, NULL,
	    NULL, NULL, NULL, NULL }},
	{ X86_VENDOR_INTEL,	6,
	  { "Pentium Pro A-step", "Pentium Pro", NULL, "Pentium II (Klamath)", 
	    NULL, "Pentium II (Deschutes)", "Celeron (Mendocino)", NULL,
	    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }},
	{ X86_VENDOR_AMD,	4,
	  { NULL, NULL, NULL, "486 DX/2", NULL, NULL, NULL, "486 DX/2-WB",
	    "486 DX/4", "486 DX/4-WB", NULL, NULL, NULL, NULL, "Am5x86-WT",
	    "Am5x86-WB" }},
	{ X86_VENDOR_AMD,	5,
	  { "K5/SSA5", "K5",
	    "K5", "K5", NULL, NULL,
	    "K6", "K6", "K6-2",
	    "K6-3", NULL, NULL, NULL, NULL, NULL, NULL }},
	{ X86_VENDOR_UMC,	4,
	  { NULL, "U5D", "U5S", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	    NULL, NULL, NULL, NULL, NULL, NULL }},
	{ X86_VENDOR_CENTAUR,	5,
	  { NULL, NULL, NULL, NULL, "C6", NULL, NULL, NULL, "C6-2", NULL, NULL,
	    NULL, NULL, NULL, NULL, NULL }},
	{ X86_VENDOR_NEXGEN,	5,
	  { "Nx586", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	    NULL, NULL, NULL, NULL, NULL, NULL, NULL }},
};


unsigned char inportb (unsigned short _port)
{
  unsigned char rv;
  __asm__ __volatile__ ("inb %1, %0"
	  : "=a" (rv)
	  : "d" (_port));
  return rv;
}

unsigned short inportw (unsigned short _port)
{
  unsigned short rv;
  __asm__ __volatile__ ("inw %1, %0"
	  : "=a" (rv)
	  : "d" (_port));
  return rv;
}

void insw(unsigned short port, unsigned short *addr, unsigned int count)
{
 __asm__ __volatile__ ("cld ; rep ; insw" \
: "=D" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count));
}

void insl(unsigned short port, unsigned int *addr, unsigned int count)
{
 __asm__ __volatile__ ("cld ; rep ; insl" \
: "=D" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count));
}

void outsw(unsigned short port, const unsigned short * addr, unsigned int count)
{
  __asm__ __volatile__ ("cld ; rep ; outsw" \
: "=S" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count));
}

void outsl(unsigned short port, const unsigned int * addr, unsigned int count)
{
  __asm__ __volatile__ ("cld ; rep ; outsl" \
: "=S" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count));
}


unsigned long inportl (unsigned short _port)
{
  unsigned long rv;
  __asm__ __volatile__ ("inl %1, %0"
	  : "=a" (rv)
	  : "d" (_port));
  return rv;
}

void outportb (unsigned short _port, unsigned char _data)
{
  __asm__ __volatile__ ("outb %1, %0"
	  :
	  : "d" (_port),
	    "a" (_data));
}

void outportw (unsigned short _port, unsigned short _data)
{
  __asm__ __volatile__ ("outw %1, %0"
	  :
	  : "d" (_port),
	    "a" (_data));
}

void outportl (unsigned short _port, unsigned long _data)
{
  __asm__ __volatile__ ("outl %1, %0"
	  :
	  : "d" (_port),
	    "a" (_data));
}

void memory_dump(int addr, int bytes)
{
  char str[255]={0};
  char str2[255]={0};
  int i = 0;
  unsigned char* pointer;

  pointer = (unsigned char*) addr;

  show_msg("Memory dump of area 0x%00000008x - 0x%00000008x\n",addr,addr+bytes);

  while (bytes)
  {
    sprintf(str, "%02X ",*pointer);
    strcat(str2, str);
    pointer++;
    bytes--;

    i++;
    if (i > 24)
    {
      i = 0;
      show_msg(str2);
      memset(str2,0,255);
    }
  }
  show_msg("\n");
}

void mem_dump_char(int addr, int bytes)
{
  char a_str[255]={0};
  char a_str2[255]={0};
  int i = 0;
  unsigned char* pointer;

  pointer = (unsigned char*) addr;

  while (bytes)
  {
    sprintf(a_str, "%c",*pointer);
    strcat(a_str2, a_str);
    pointer++;
    bytes--;

    i++;
    if (i > 30)
    {
      i = 0;
      show_msg(a_str2);
      memset(a_str2,0,255);
    }
    else if (bytes==0)
    {
      i = 0;
      show_msg(a_str2);
      memset(a_str2,0,255);
    }
  }
  show_msg("\n");
}

void mem_dump(int addr, int bytes)
{
  char str[255]={0};
  char str2[255]={0};
  char a_str[255]={0};
  char a_str2[255]={0};
  int i = 0;
  unsigned char* pointer;

  sprintf(a_str2, "    ");

  pointer = (unsigned char*) addr;

  while (bytes)
  {
    sprintf(str, "%02X ",*pointer);

    sprintf(a_str, "%c",*pointer);
    strcat(str2, str);
    strcat(a_str2, a_str);
    pointer++;
    bytes--;

    i++;
    if (i > 14)
    {
      i = 0;
      strcat(str2, a_str2);
      show_msg(str2);
      memset(str2,0,255);
      sprintf(a_str2,"    ");
    }
    else if (bytes==0)
    {
      i = 0;
      strcat(str2, a_str2);
      show_msg(str2);
      memset(str2,0,255);
      sprintf(a_str2,"    ");
    }
  }
  show_msg("\n");
}

unsigned get_eflags()
{
	unsigned eflags;
	asm volatile("
		jmp	1f
	1:	jmp	1f
	1:	jmp	1f
	1:	pushf
		jmp	1f
	1:	jmp	1f
	1:	jmp	1f
	1:	popl %0" : "=r" (eflags));
	return eflags;
}

void set_eflags(unsigned eflags)
{
	asm volatile("
		pushl %0
		jmp	1f
	1:	jmp	1f
	1:	jmp	1f
	1:	popf
		jmp	1f
	1:	jmp	1f
	1:	jmp	1f
	1:	" : : "r" (eflags));
}


void cpuid(struct cpu_info *out_id)
{
	int orig_eflags = get_eflags();

	memset(out_id, 0, sizeof(*out_id));

	/* Check for a dumb old 386 by trying to toggle the AC flag.  */
	set_eflags(orig_eflags ^ EFL_AC);

	if ((get_eflags() ^ orig_eflags) & EFL_AC)
	{
		/* It's a 486 or better.  Now try toggling the ID flag.  */
		set_eflags(orig_eflags ^ EFL_ID);
		if ((get_eflags() ^ orig_eflags) & EFL_ID)
		{
			int highest_val;

			/*
			 * CPUID is supported, so use it.
			 * First get the vendor ID string.
			 */
			asm volatile("
				cpuid
			" : "=a" (highest_val),
			    "=b" (*((int*)(out_id->vendor_id+0))),
			    "=d" (*((int*)(out_id->vendor_id+4))),
			    "=c" (*((int*)(out_id->vendor_id+8)))
			  : "a" (0));

			/* Now the feature information.  */
			if (highest_val >= 1)
			{
				asm volatile("
					cpuid
				" : "=a" (*((int*)out_id)),
				    "=d" (out_id->feature_flags)
				  : "a" (1)
				  : "ebx", "ecx");
			}

			/* Cache and TLB information.  */
//			if (highest_val >= 2)
//				get_cache_config(out_id);
		}
		else
		{
			/* No CPUID support - it's an older 486.  */
			out_id->family = CPU_FAMILY_486;

			/* XXX detect FPU */
		}
	}
	else
	{
		out_id->family = CPU_FAMILY_386;

		/* XXX detect FPU */
	}
	set_eflags(orig_eflags);
}

void dump_cpuinfo(struct cpu_info *cpu)
{
   char scpu[50];
   char sfpu[50];
   unsigned int *pidata;
   struct cpuinfo_x86 *c;

   c = (unsigned int*)0x80600;

   printk("Processor   : 1");
   printk("Vendor ID   : %s",c->x86_vendor_id[0] ? c->x86_vendor_id : "unknown");

   printk("pc.c: Detecting CPU...");

   if (cpu->family == CPU_FAMILY_386)
     sprintf(scpu,"80386-Class CPU");
   if (cpu->family == CPU_FAMILY_486)
     sprintf(scpu,"80486-Class CPU");
   if (cpu->family == CPU_FAMILY_PENTIUM)
     sprintf(scpu,"Pentium-Class(\"586\")");
   if (cpu->family == CPU_FAMILY_PENTIUM_PRO)
     sprintf(scpu,"Pentium-Pro-Class(\"686\")");

   if (cpu->family > CPU_FAMILY_486)
   {
     if (cpu->feature_flags & CPUF_ON_CHIP_FPU)
     {
       sprintf(sfpu,"Build in floating-point unit.");
       register_hardware(DEVICE_IDENT_CPU, "Floating Point Unit", 0);
     }
     else
       sprintf(sfpu,"No Build in floating-point unit.");

     printk("pc.c: %s  %s",scpu,sfpu);

   }
   else printk("pc.c: %s",scpu);

   register_hardware(DEVICE_IDENT_CPU, scpu, 0);
}

void get_time(struct time *t)
{
      t->sec = CMOS_READ(RTC_SECONDS);
      t->min = CMOS_READ(RTC_MINUTES);
      t->hour = CMOS_READ(RTC_HOURS);
      t->day = CMOS_READ(RTC_DAY_OF_MONTH);
      t->mon = CMOS_READ(RTC_MONTH);
      t->year = CMOS_READ(RTC_YEAR);
      BCD_TO_BIN(t->sec);
      BCD_TO_BIN(t->min);
      BCD_TO_BIN(t->hour);
      BCD_TO_BIN(t->day);
      BCD_TO_BIN(t->mon);
      BCD_TO_BIN(t->year);
}

unsigned int hex2dec(unsigned char *str)
{
  unsigned char len;
  unsigned short count;
  unsigned char fail;

  unsigned int mult;

  unsigned char ch;
  unsigned int dec;

  len = strlen(str)-1;
  count = len;
  fail = 0;

  mult = 1;
  dec = 0;

  do
  {
    ch = str[count];

    if ((ch >= '0') && (ch <= '9'))
      dec += mult * (ch - 0x30);
    else if ((ch >= 'a') && (ch <= 'f'))
      dec += mult * (ch - 87);
    else if ((ch >= 'A') && (ch <= 'F'))
      dec += mult * (ch - 55);
    else fail = 1;

    if (count>0)
      count--;
    else fail = 1;
    mult = mult * 16;
  } while  ((count >= 0) && (!fail));

  return dec;

}


