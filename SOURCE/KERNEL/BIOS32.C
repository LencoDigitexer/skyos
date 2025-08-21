/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\bios32.c
/* Last Update: 20.08.1999
/* Version    :
/* Coded by   : Szeleney Robert
/* Docus      : Linux 2.0.35 Kernel
/************************************************************************/
/* Definition:
/*   BIOS32 / PCI Services
/* Some parts of this code are taken from linux kernel 2.0.35
/************************************************************************/
#include "sched.h"
#include "head.h"

#define PACKED __attribute__((packed))
#define NULL (void*)0

/* BIOS32 signature: "_32_" */
#define BIOS32_SIGNATURE	(('_' << 0) + ('3' << 8) + ('2' << 16) + ('_' << 24))

/* PCI signature: "PCI " */
#define PCI_SIGNATURE		(('P' << 0) + ('C' << 8) + ('I' << 16) + (' ' << 24))

/* PCI service signature: "$PCI" */
#define PCI_SERVICE		(('$' << 0) + ('P' << 8) + ('C' << 16) + ('I' << 24))

#define PCIBIOS_PCI_FUNCTION_ID 	0xb1XX
#define PCIBIOS_PCI_BIOS_PRESENT 	0xb101
#define PCIBIOS_FIND_PCI_DEVICE		0xb102
#define PCIBIOS_FIND_PCI_CLASS_CODE	0xb103
#define PCIBIOS_GENERATE_SPECIAL_CYCLE	0xb106
#define PCIBIOS_READ_CONFIG_BYTE	0xb108
#define PCIBIOS_READ_CONFIG_WORD	0xb109
#define PCIBIOS_READ_CONFIG_DWORD	0xb10a
#define PCIBIOS_WRITE_CONFIG_BYTE	0xb10b
#define PCIBIOS_WRITE_CONFIG_WORD	0xb10c
#define PCIBIOS_WRITE_CONFIG_DWORD	0xb10d

#define PCIBIOS_SUCCESSFUL		0x00
#define PCIBIOS_FUNC_NOT_SUPPORTED	0x81
#define PCIBIOS_BAD_VENDOR_ID		0x83
#define PCIBIOS_DEVICE_NOT_FOUND	0x86
#define PCIBIOS_BAD_REGISTER_NUMBER	0x87
#define PCIBIOS_SET_FAILED		0x88
#define PCIBIOS_BUFFER_TOO_SMALL	0x89

union bios32
{
   struct
   {
      unsigned long magic                       PACKED;
      unsigned long entry                       PACKED;
      unsigned char revision                    PACKED;
      unsigned char length                      PACKED;
      unsigned char checksum                    PACKED;
      unsigned char reserved[5]                 PACKED;
   } fields;
   char chars[16];
};

/* BIOS32 Address */
static unsigned long bios32_entry = 0;
static struct
{
  unsigned long offset;
  unsigned short segment;
} bios32_indirect = { 0 , KERNEL_CS};

/* PCIBIOS Address */
static unsigned long pcibios_entry = 0;
static struct
{
  unsigned long offset;
  unsigned short segment;
} pci_indirect = { 0 , KERNEL_CS};

/*
 * function table for accessing PCI configuration space
 */
struct pci_access
{
    int (*find_device)(unsigned short, unsigned short, unsigned short, unsigned char *, unsigned char *);
    int (*find_class)(unsigned int, unsigned short, unsigned char *, unsigned char *);
    int (*read_config_byte)(unsigned char, unsigned char, unsigned char, unsigned char *);
    int (*read_config_word)(unsigned char, unsigned char, unsigned char, unsigned short *);
    int (*read_config_dword)(unsigned char, unsigned char, unsigned char, unsigned int *);
    int (*write_config_byte)(unsigned char, unsigned char, unsigned char, unsigned char);
    int (*write_config_word)(unsigned char, unsigned char, unsigned char, unsigned short);
    int (*write_config_dword)(unsigned char, unsigned char, unsigned char, unsigned int);
};

/*
 * pointer to selected PCI access function table
 */
static struct pci_access *access_pci = NULL;


static unsigned long bios32_service(unsigned long service)
{
	unsigned char return_code;	/* %al */
	unsigned long address;		/* %ebx */
	unsigned long length;		/* %ecx */
	unsigned long entry;		/* %edx */
	unsigned long flags;

	save_flags(flags);
        cli();

	__asm__("lcall (%%edi)"
		: "=a" (return_code),
		  "=b" (address),
		  "=c" (length),
		  "=d" (entry)
		: "0" (service),
		  "1" (0),
		  "D" (&bios32_indirect));

        restore_flags(flags);

	switch (return_code) {
		case 0:
			return address + entry;
		case 0x80:	/* Not present */
			printk("bios32.c: BIOS32 Service (0x%lx) not present", service);
			return 0;
		default: /* Shouldn't happen */
			printk("bios32.c: BIOS32 Service (0x%lx), unknown return code %d",
				service, return_code);
			return 0;
	}
}

static int check_pcibios(void)
{
  unsigned long magic;
  unsigned char present_status;
  unsigned char major;
  unsigned char minor;
  unsigned long flags;
  int pack;

  if ((pcibios_entry = bios32_service(PCI_SERVICE)))
  {
	pci_indirect.offset = pcibios_entry;

	save_flags(flags);
        cli();

	__asm__("lcall (%%edi)\n\t"
		"jc 1f\n\t"
		"xor %%ah, %%ah\n"
		"1:\tshl $8, %%eax\n\t"
		"movw %%bx, %%ax"
		: "=d" (magic),
		  "=a" (pack)
		: "1" (PCIBIOS_PCI_BIOS_PRESENT),
		  "D" (&pci_indirect)
		: "bx", "cx");

        restore_flags(flags);

	present_status = (pack >> 16) & 0xff;
	major = (pack >> 8) & 0xff;
	minor = pack & 0xff;

	if (present_status || (magic != PCI_SIGNATURE))
        {
	   printk ("bios32.c: %s : BIOS32 Service Directory says PCI BIOS is present",(magic == PCI_SIGNATURE) ?  "WARNING" : "ERROR");
           printk ("       but PCI_BIOS_PRESENT subfunction fails with present status of 0x%x",present_status);
	   printk("        and signature of 0x%08lx (%c%c%c%c).",magic,
	                   (char) (magic >>  0), (char) (magic >>  8),
			   (char) (magic >> 16), (char) (magic >> 24));


  	   if (magic != PCI_SIGNATURE)
		pcibios_entry = 0;
	}

        if (pcibios_entry)
        {
	   printk ("bios32.c: PCI BIOS revision %x.%02x entry at 0x%lx\n",
				major, minor, pcibios_entry);
  	   return 1;
        }
   }
   return 0;
}


static int pci_bios_find_class (unsigned int class_code, unsigned short index,
	unsigned char *bus, unsigned char *device_fn)
{
	unsigned long bx;
	unsigned long ret;
	unsigned long flags;

	save_flags(flags);
        cli();

	__asm__ ("lcall (%%edi)\n\t"
		"jc 1f\n\t"
		"xor %%ah, %%ah\n"
		"1:"
		: "=b" (bx),
		  "=a" (ret)
		: "1" (PCIBIOS_FIND_PCI_CLASS_CODE),
		  "c" (class_code),
		  "S" ((int) index),
		  "D" (&pci_indirect));

	restore_flags(flags);
	*bus = (bx >> 8) & 0xff;
	*device_fn = bx & 0xff;
	return (int) (ret & 0xff00) >> 8;
}


static int pci_bios_find_device (unsigned short vendor, unsigned short device_id,
	unsigned short index, unsigned char *bus, unsigned char *device_fn)
{
	unsigned short bx;
	unsigned short ret;
	unsigned long flags;

	save_flags(flags); cli();
	__asm__("lcall (%%edi)\n\t"
		"jc 1f\n\t"
		"xor %%ah, %%ah\n"
		"1:"
		: "=b" (bx),
		  "=a" (ret)
		: "1" (PCIBIOS_FIND_PCI_DEVICE),
		  "c" (device_id),
		  "d" (vendor),
		  "S" ((int) index),
		  "D" (&pci_indirect));
	restore_flags(flags);
	*bus = (bx >> 8) & 0xff;
	*device_fn = bx & 0xff;
	return (int) (ret & 0xff00) >> 8;
}

static int pci_bios_read_config_byte(unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned char *value)
{
	unsigned long ret;
	unsigned long bx = (bus << 8) | device_fn;
	unsigned long flags;

	save_flags(flags); cli();
	__asm__("lcall (%%esi)\n\t"
		"jc 1f\n\t"
		"xor %%ah, %%ah\n"
		"1:"
		: "=c" (*value),
		  "=a" (ret)
		: "1" (PCIBIOS_READ_CONFIG_BYTE),
		  "b" (bx),
		  "D" ((long) where),
		  "S" (&pci_indirect));
	restore_flags(flags);
	return (int) (ret & 0xff00) >> 8;
}

static int pci_bios_read_config_word (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned short *value)
{
	unsigned long ret;
	unsigned long bx = (bus << 8) | device_fn;
	unsigned long flags;

	save_flags(flags); cli();
	__asm__("lcall (%%esi)\n\t"
		"jc 1f\n\t"
		"xor %%ah, %%ah\n"
		"1:"
		: "=c" (*value),
		  "=a" (ret)
		: "1" (PCIBIOS_READ_CONFIG_WORD),
		  "b" (bx),
		  "D" ((long) where),
		  "S" (&pci_indirect));
	restore_flags(flags);
	return (int) (ret & 0xff00) >> 8;
}

static int pci_bios_read_config_dword (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned int *value)
{
	unsigned long ret;
	unsigned long bx = (bus << 8) | device_fn;
	unsigned long flags;

	save_flags(flags); cli();
	__asm__("lcall (%%esi)\n\t"
		"jc 1f\n\t"
		"xor %%ah, %%ah\n"
		"1:"
		: "=c" (*value),
		  "=a" (ret)
		: "1" (PCIBIOS_READ_CONFIG_DWORD),
		  "b" (bx),
		  "D" ((long) where),
		  "S" (&pci_indirect));
	restore_flags(flags);
	return (int) (ret & 0xff00) >> 8;
}

static int pci_bios_write_config_byte (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned char value)
{
	unsigned long ret;
	unsigned long bx = (bus << 8) | device_fn;
	unsigned long flags;

	save_flags(flags); cli();
	__asm__("lcall (%%esi)\n\t"
		"jc 1f\n\t"
		"xor %%ah, %%ah\n"
		"1:"
		: "=a" (ret)
		: "0" (PCIBIOS_WRITE_CONFIG_BYTE),
		  "c" (value),
		  "b" (bx),
		  "D" ((long) where),
		  "S" (&pci_indirect));
	restore_flags(flags);
	return (int) (ret & 0xff00) >> 8;
}

static int pci_bios_write_config_word (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned short value)
{
	unsigned long ret;
	unsigned long bx = (bus << 8) | device_fn;
	unsigned long flags;

	save_flags(flags); cli();
	__asm__("lcall (%%esi)\n\t"
		"jc 1f\n\t"
		"xor %%ah, %%ah\n"
		"1:"
		: "=a" (ret)
		: "0" (PCIBIOS_WRITE_CONFIG_WORD),
		  "c" (value),
		  "b" (bx),
		  "D" ((long) where),
		  "S" (&pci_indirect));
	restore_flags(flags);
	return (int) (ret & 0xff00) >> 8;
}

static int pci_bios_write_config_dword (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned int value)
{
	unsigned long ret;
	unsigned long bx = (bus << 8) | device_fn;
	unsigned long flags;

	save_flags(flags); cli();
	__asm__("lcall (%%esi)\n\t"
		"jc 1f\n\t"
		"xor %%ah, %%ah\n"
		"1:"
		: "=a" (ret)
		: "0" (PCIBIOS_WRITE_CONFIG_DWORD),
		  "c" (value),
		  "b" (bx),
		  "D" ((long) where),
		  "S" (&pci_indirect));
	restore_flags(flags);
	return (int) (ret & 0xff00) >> 8;
}
/*
 * function table for BIOS32 access
 */
static struct pci_access pci_bios_access = {
      pci_bios_find_device,
      pci_bios_find_class,
      pci_bios_read_config_byte,
      pci_bios_read_config_word,
      pci_bios_read_config_dword,
      pci_bios_write_config_byte,
      pci_bios_write_config_word,
      pci_bios_write_config_dword
};

unsigned long pcibios_init(void)
{
  union bios32 *check;
  unsigned char sum;
  int i, length;

  /* Check if a PCI Bios is present. Scan memory from 0xe0000 to 0xffff0 */
  for (check = (union bios32*) 0xe0000;
       check <= (union bios32*) 0xffff0;
       ++check)
  {
    if (check->fields.magic != BIOS32_SIGNATURE)
      continue;

    length = check->fields.length * 16;
    if (!length)
      continue;

    sum = 0;
    for (i=0; i<length;++i)
      sum += check->chars[i];
    if (sum != 0)
      continue;

    if (check->fields.revision != 0)
    {
      printk("bios32.c: Revision %d at 0x%p not supported.", check->fields.revision, check);
      continue;
    }
    printk("bios32.c: BIOS32 Service Directory structure at 0x%p",check);

    if (check->fields.entry >= 0x100000)
    {
       printk("bios32.c: entry in high memory. Direct Access!");
       printk("bios32.c: Isn't supported. Mail bertlman@gmx.at");
       /*** NOT SUPPORTED ***/
    }
    else
    {
      bios32_entry = check->fields.entry;
      printk("bios32.c: BIOS32 Service Directory entry at 0x%lx",bios32_entry);
      bios32_indirect.offset = bios32_entry;
    }
  }

  if (bios32_entry && check_pcibios())
  {
    printk("bios32.c: PCIBIOS found and initialized.");
    access_pci = &pci_bios_access;

  }
  else printk("bios32.c: PCIBIOS not found.");
}



int pcibios_present(void)
{
	return access_pci ? 1 : 0;
}

int pcibios_find_class (unsigned int class_code, unsigned short index,
	unsigned char *bus, unsigned char *device_fn)
{
   if (access_pci && access_pci->find_class)
      return access_pci->find_class(class_code, index, bus, device_fn);
    
    return PCIBIOS_FUNC_NOT_SUPPORTED;
}

int pcibios_find_device (unsigned short vendor, unsigned short device_id,
	unsigned short index, unsigned char *bus, unsigned char *device_fn)
{
    if (access_pci && access_pci->find_device)
      return access_pci->find_device(vendor, device_id, index, bus, device_fn);
    
    return PCIBIOS_FUNC_NOT_SUPPORTED;
}

int pcibios_read_config_byte (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned char *value)
{
    if (access_pci && access_pci->read_config_byte)
      return access_pci->read_config_byte(bus, device_fn, where, value);
    
    return PCIBIOS_FUNC_NOT_SUPPORTED;
}

int pcibios_read_config_word (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned short *value)
{
    if (access_pci && access_pci->read_config_word)
      return access_pci->read_config_word(bus, device_fn, where, value);
    
    return PCIBIOS_FUNC_NOT_SUPPORTED;
}

int pcibios_read_config_dword (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned int *value)
{
    if (access_pci && access_pci->read_config_dword)
      return access_pci->read_config_dword(bus, device_fn, where, value);
    
    return PCIBIOS_FUNC_NOT_SUPPORTED;
}

int pcibios_write_config_byte (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned char value)
{
    if (access_pci && access_pci->write_config_byte)
      return access_pci->write_config_byte(bus, device_fn, where, value);
    
    return PCIBIOS_FUNC_NOT_SUPPORTED;
}

int pcibios_write_config_word (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned short value)
{
    if (access_pci && access_pci->write_config_word)
      return access_pci->write_config_word(bus, device_fn, where, value);
    
    return PCIBIOS_FUNC_NOT_SUPPORTED;    
}

int pcibios_write_config_dword (unsigned char bus,
	unsigned char device_fn, unsigned char where, unsigned int value)
{
    if (access_pci && access_pci->write_config_dword)
      return access_pci->write_config_dword(bus, device_fn, where, value);
    
    return PCIBIOS_FUNC_NOT_SUPPORTED;
}

const char *pcibios_strerror (int error)
{
	static char buf[80];

	switch (error) {
		case PCIBIOS_SUCCESSFUL:
			return "SUCCESSFUL";

		case PCIBIOS_FUNC_NOT_SUPPORTED:
			return "FUNC_NOT_SUPPORTED";

		case PCIBIOS_BAD_VENDOR_ID:
			return "SUCCESSFUL";

		case PCIBIOS_DEVICE_NOT_FOUND:
			return "DEVICE_NOT_FOUND";

		case PCIBIOS_BAD_REGISTER_NUMBER:
			return "BAD_REGISTER_NUMBER";

                case PCIBIOS_SET_FAILED:          
			return "SET_FAILED";

                case PCIBIOS_BUFFER_TOO_SMALL:    
			return "BUFFER_TOO_SMALL";

		default:
			sprintf (buf, "UNKNOWN RETURN 0x%x", error);
			return buf;
	}
}


unsigned long pcibios_fixup(unsigned long mem_start, unsigned long mem_end)
{
    return mem_start;
}

