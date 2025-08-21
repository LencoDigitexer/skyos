/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\module.c
/* Last Update: 25.01.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus from :
/************************************************************************/
/* Definition:
/*   Loadable Module Support for COFF files.
/*   To see which file is defined as a module, look in module.h
/*
/* To do:
/*   - Mapfilesize is limited to 50000bytes.
/************************************************************************/
#include "sched.h"
#include "error.h"
#include "coff.h"
#include "fcntl.h"

#define MD_FILHDR 1
#define MD_SCNHDR 2
#define MD_SYMENT 4
#define MD_RELOC  8

#define PACKED __attribute__((packed))

struct map_file_entry
{
  unsigned int addr           PACKED;
  unsigned char name[60]      PACKED;
};

struct list *map_list = NULL;                        // map list

unsigned int module_debug = MD_SYMENT;
#define MD module_debug

unsigned int module_address;
unsigned char module_name[20];
unsigned int im;
extern void guire(void);
extern void show_msg(unsigned char *);


/************************************************************************/
/* Loads a section from a modulefile into memory to a specificated
/* address
/************************************************************************/
unsigned int module_load_section(unsigned int destination, unsigned int handle,
                         unsigned int fileoffset, unsigned int size)
{
   unsigned int oldoffset;
   unsigned char *buffer;

   if (size == 0) return;

/* Remember old file offset address */
   oldoffset = sys_tell(handle);

/* set pointer to destination address in memory, must already be allocated */
   buffer = (unsigned char*)destination;

   sys_seek(handle, fileoffset, 0);

   sys_read(handle, buffer, size);

   sys_seek(handle, oldoffset, 0);

   return 0;
}

/************************************************************************/
/* Loads the fileheader of the module file.
/************************************************************************/
unsigned int module_load_fileheader(unsigned int handle, COFF_FILHDR *coff_filhdr)
{
  if (MD & MD_FILHDR)
    show_msgf("module.c: Reading header... ");

/* Read COFF File Header */
  if (sys_read(handle, coff_filhdr, COFF_FILHSZ) < 0) return -1;

  if (MD & MD_FILHDR)
  {
    show_msgf("Analysing header:");
    show_msgf("COFF File              : %s",(COFF_SHORT_L(coff_filhdr->f_magic) == COFF_I386MAGIC)?"YES":"NO");
    show_msgf("Sections               : %d",COFF_SHORT_L(coff_filhdr->f_nscns));
    show_msgf("Symbol Table Offset    : %d",COFF_LONG_L(coff_filhdr->f_symptr));
    show_msgf("Symbol Numbers         : %d",COFF_LONG_L(coff_filhdr->f_nsyms));
    show_msgf("Optional Header        : %d",COFF_SHORT_L(coff_filhdr->f_opthdr));

    if (COFF_SHORT_L(coff_filhdr->f_flags) & COFF_F_RELFLG)
     show_msgf("Flag                   : No relocation information. Executable");
    if (COFF_SHORT_L(coff_filhdr->f_flags) & COFF_F_EXEC)
     show_msgf("Flag                   : All symbols resolved. Executable");
    if (COFF_SHORT_L(coff_filhdr->f_flags) & COFF_F_AR32WR)
     show_msgf("Flag                   : 32-Bit little endian.");
  }
}

/************************************************************************/
/* Loads one symbol from the module file.
/************************************************************************/
void module_load_symbol(unsigned int handle, unsigned int fileoffset,
                        unsigned int symnr, COFF_SYMENT *syment,
                        unsigned char *string_table, unsigned char *symname)
{
   unsigned int oldoffset;
   unsigned int address;

   oldoffset = sys_tell(handle);

   sys_seek(handle, fileoffset + (symnr*COFF_SYMESZ), 0);

   sys_read(handle, syment, COFF_SYMESZ);

   if (COFF_LONG_L(syment->e.e.e_zeroes) == 0)                // symbole name not inlined
   {
//      show_msgf("module.c: symbolname (string_table): %s",string_table + COFF_LONG_L(syment->e.e.e_offset));
//      show_msgf("module.c: stringoffset: %d",COFF_LONG_L(syment->e.e.e_offset));
      strcpy(symname,string_table + COFF_LONG_L(syment->e.e.e_offset));
   }
   else
   {
      memcpy(symname, syment->e.e_name, 8);
      if (symname[7] != '\0') symname[8] = '0';

      if (MD & MD_SYMENT)
      {
        //show_msgf("module.c: symbolname: %s",symname);
      }
   }

   sys_seek(handle, oldoffset, 0);
}

/************************************************************************/
/* Load a module into memory, relocate it and create a task
/************************************************************************/
int load_module(unsigned char *device, unsigned char *name)
{
 int size;
 unsigned char str[255];
 COFF_FILHDR coff_filhdr;
 COFF_SCNHDR scnhdr;
 COFF_SYMENT symbol;
 COFF_RELOC reloc;
 int sections;
 int sectionptr;
 int relocs;
 unsigned char ch;
 int filepos;
 int filepos2;
 int value;
 int ret;
 unsigned char *relocate_to_address;
 unsigned int section_data;
 void *pointer;
 unsigned int address = -1;
 unsigned int *lp;
 unsigned int i;
 unsigned int init_module = -1;
 unsigned int last_section_size = 0;

 unsigned char *string_table;
 unsigned int string_table_size = 0;
 unsigned char symname[255] = {0};
 int handle = 0;


 sys_actdev(device);

 handle = sys_open(name, O_RDONLY);

 if (handle < 0)
 {
    show_msg("module.c: Failed to open module");
    error(handle);
    return -1;
 }

  relocate_to_address = (unsigned char *)valloc(1000000);

  show_msgf("Loading Module %s .... ",name);
  show_msgf("Relocating to address: %00000008X",(unsigned int)relocate_to_address);


/* Read COFF File Header */
  if (module_load_fileheader(handle, &coff_filhdr) < 0)
  {
     show_msgf("module.c: Error while reading module header.");
     return -1;
  }
  sections = COFF_SHORT_L(coff_filhdr.f_nscns);

/* Read String Table */
  filepos = sys_tell(handle);

  sys_seek(handle, COFF_LONG_L(coff_filhdr.f_nsyms) * COFF_SYMESZ +
        COFF_LONG_L(coff_filhdr.f_symptr), 0);

  sys_read(handle, &i, 4);
  string_table = (unsigned char *)valloc(i);
  memset(string_table, 0, 4);
  ret = sys_read(handle, string_table + 4 , i);

  sys_seek(handle, filepos, 0);

  show_msgf("module.c: String table size: %dbytes, read: %d",i, ret);
  show_msgf("module.c: String table offset: 0x%00000008X, %d",string_table, string_table);
  mem_dump(string_table, 200);


/* Search module entry point */
  for (i=0; i < COFF_LONG_L(coff_filhdr.f_nsyms); i++)
  {
     module_load_symbol(handle, COFF_LONG_L(coff_filhdr.f_symptr),
                        i, &symbol, string_table, symname);

     if (!strcmp(symname,"_init"))
     {
       init_module = COFF_LONG_L(symbol.e_value);

       show_msgf("module.c: Module Entry Point: %00000008X",init_module);

       im = init_module;
       show_msgf("module.c: Adjusted module Entry Point: %00000008X",(unsigned int)relocate_to_address + init_module);

       break;
     }
  }
  if (init_module == -1)
  {
     show_msg("module.c: No Module entry point found.");
     sys_close(handle);
     return -1;
  }

/* Read Sections */
  last_section_size = 0;

  while (sections--)
  {
/* Read one section header */
     sys_read(handle, &scnhdr, COFF_SCNHSZ);

     sectionptr = COFF_LONG_L(scnhdr.s_scnptr);

     if (MD & MD_SCNHDR)
     {
       show_msgf("Section Nummer               : %d",sections+1);
       show_msgf("Section Name                 : %s",scnhdr.s_name);
       show_msgf("Physical/Virtual Address     : %d",COFF_LONG_L(scnhdr.s_paddr));
       show_msgf("Size                         : %d",COFF_LONG_L(scnhdr.s_size));
       show_msgf("Section data pointer         : %d",COFF_LONG_L(scnhdr.s_scnptr));
       show_msgf("Relocation data pointer      : %d",COFF_LONG_L(scnhdr.s_relptr));
       show_msgf("Line number table pointer    : %d",COFF_LONG_L(scnhdr.s_lnnoptr));
       show_msgf("Number of relocation entries : %d",COFF_SHORT_L(scnhdr.s_nreloc));
       show_msgf("Number of line number entries: %d",COFF_SHORT_L(scnhdr.s_nlnno));
     }

/* Load section row date into memory */

     module_load_section((unsigned int)relocate_to_address + last_section_size,
                         handle, sectionptr, COFF_LONG_L(scnhdr.s_size));

     last_section_size += COFF_LONG_L(scnhdr.s_size);

     relocs = COFF_SHORT_L(scnhdr.s_nreloc);

     // remember old filepos
     filepos = sys_tell(handle);

     sys_seek(handle,COFF_LONG_L(scnhdr.s_relptr), 0);

     while (relocs--)
     {
       sys_read(handle, &reloc, COFF_RELSZ);
       if (MD & MD_RELOC)
       {
         show_msgf("Relocationaddress         : %d",COFF_LONG_L(reloc.r_vaddr));
         show_msgf("Symbol to adjust for      : %d",COFF_LONG_L(reloc.r_symndx));
         show_msgf("Type                      : %s",(COFF_SHORT_L(reloc.r_type)==6)?"absolute":"relative");
       }

       module_load_symbol(handle, COFF_LONG_L(coff_filhdr.f_symptr),
                          COFF_LONG_L(reloc.r_symndx), &symbol, string_table, symname);
       address = -1;

       if (COFF_SHORT_L(reloc.r_type)==6) // absolute address
       {
          show_msgf("module.c: absolute address: %00000008X",COFF_LONG_L(symbol.e_value));
          lp = (unsigned int *)(relocate_to_address + COFF_LONG_L(reloc.r_vaddr));

          show_msgf("module.c: class : %d",symbol.e_sclass[0]);
          if (symbol.e_sclass[0] == 2)
          {
             struct map_file_entry *entry;
             int map_count = 0;

             show_msgf("module.c: relocate extern...");
/*             entry = (struct map_file_entry*)get_item(map_list,map_count++);
             while (entry != NULL)
             {
               if (!strcmp(symname, entry->name))
                 address = (unsigned int)entry->addr;
               entry=(struct map_file_entry*)get_item(map_list,map_count++);
             }*/
             if ((!strcmp(symname, "_show_msgf")) || (!strcmp(symname, "show_msgf")))
             {
                entry->addr = show_msgf;
                address = entry->addr;
             }
          }
          else
          {
            address = (unsigned int)relocate_to_address;
            address += COFF_LONG_L(symbol.e_value);
          }

          show_msgf("module.c: adjusted link address: %00000008X", address);
          *lp = address;
      }
      else
      {
           /* search for a kernel symbol */
           struct map_file_entry *entry;
           int map_count = 0;

             if ((!strcmp(entry->name, "_show_msgf")) || (!strcmp(entry->name, "show_msgf")))
             {
                entry->addr = show_msgf;
                address = entry->addr;
             }
/*           entry = (struct map_file_entry*)get_item(map_list,map_count++);
           while (entry != NULL)
           {
             if (!strcmp(symname, entry->name))
               address = (unsigned int)entry->addr;
             entry=(struct map_file_entry*)get_item(map_list,map_count++);
           }*/

           if (address == -1)
           {
              show_msgf("module.c: unable to resolve link %s",symname);
              sys_close(handle);
              return;
           }

           show_msgf("module.c: kernel link address: %00000008X",address);
           lp = (unsigned int *)(relocate_to_address + COFF_LONG_L(reloc.r_vaddr));
           show_msgf("module.c: module location address: %00000008X",*lp);

           address += *lp;
           address -= (unsigned int) relocate_to_address;

           show_msgf("module.c: adjusted link address: %00000008X", address);
           *lp = address;
      }
     }
     // set old filepos
     sys_seek(handle,filepos, 0);
   }
   show_msg("Module loaded and relocated.");
   strcpy(module_name, name);
   module_address = (unsigned int)relocate_to_address;
   show_msgf("Module entry point: %00000008X",module_address + im);
   sys_close(handle);
}

/************************************************************************/
/* Starts the loaded module.
/************************************************************************/
void execute_module(void)
{
   show_msg("Executing module...");
   CreateKernelTask((unsigned int)(module_address), module_name, NP, im);
}

/************************************************************************/
/* Load and start a module.
/************************************************************************/
void start_module(unsigned char *device, unsigned char *name)
{
  load_module(device, name);
  execute_module();
}

/************************************************************************/
/* Load the kernel map file from a device into memory.
/************************************************************************/
void load_map_file(unsigned char* device, unsigned char *name)
{
/*  struct map_file_entry *entry;
  unsigned char *buffer;
  int ret = 0;
  int count = 0;
  int maxcount = 0;
  int symbols = 0;
  int handle;

  show_msgf("module.c: mapfile size limited to 50000 byte.");
  buffer = (unsigned char*)valloc(50000);

  sys_actdev(device);

  handle = sys_open(name, O_RDONLY);
  if ( handle < 0)
  {
     show_msgf("module.c: load_map_file unable to open file");
     show_msgf("          %s on device %s.",name, device);
     return;
  }

  show_msgf("module.c: loading...");
  while (!ret)
  {
    count = sys_read(handle, buffer + maxcount, 512);
//    mem_dump(buffer + maxcount, 512);
    maxcount+=count;
    if (count != 512) ret = 1;
  }

  count = 0;
  while (maxcount)
  {
     entry = (struct map_file_entry*)(buffer + count);
//     show_msgf("%c 0x%00000008X %s",(entry->type == 1)?'T':'D',
//              entry->addr, entry->name);

     map_list=(struct list*)add_item(map_list,entry,sizeof(struct map_file_entry));
     symbols++;

     maxcount -= sizeof(struct map_file_entry);
     count += sizeof(struct map_file_entry);
  }

  show_msgf("module.c: %d resolvable symbols loaded.",symbols);

  sys_close(handle);*/
}

