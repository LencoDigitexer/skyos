/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\block\ide\ide.cpp
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      : Internet
/************************************************************************/
/* Definition:
/*   IDE device driver.
/*   Supports up to 4 IDE HARDDISKs on Controller 1/2 and Master/Slave
/*   Registers a device for each harddisk
/*   Controller 1 Master: hd0  Device Number: 10
/*   Controller 1 Slave : hd1  Device Number: 11
/*   Controller 2 Master: hd2  Device Number: 12
/*   Controller 2 Slave : hd3  Device Number: 13
/* To do:
/*   Add support for IDE CDROMs.
/************************************************************************/
#include "stdlib.h"
#include "vdm.h"
#include "hdd.h"
#include "sched.h"
#include "devices.h"
#include "devman.h"

extern CVDM vdm;

extern "C" void ide_interrupt(void);

#define MAX_INTERFACES 4                // IDE supports 4 devices

/* ATA/ATAPI drive register file */
#define	ATA_REG_DATA	0		/* data (16-bit) */
#define	ATA_REG_FEAT	1		/* write: feature reg */
#define	ATA_REG_ERR	ATA_REG_FEAT	/* read: error */
#define	ATA_REG_CNT	2		/* ATA: sector count */
#define	ATA_REG_REASON	ATA_REG_CNT	/* ATAPI: interrupt reason */
#define	ATA_REG_SECT	3		/* sector */
#define	ATA_REG_LOCYL	4		/* ATA: LSB of cylinder */
#define	ATA_REG_LOCNT	ATA_REG_LOCYL	/* ATAPI: LSB of transfer count */
#define	ATA_REG_HICYL	5		/* ATA: MSB of cylinder */
#define	ATA_REG_HICNT	ATA_REG_HICYL	/* ATAPI: MSB of transfer count */
#define	ATA_REG_DRVHD	6		/* drive select; head */
#define	ATA_REG_CMD	7		/* write: drive command */
#define	ATA_REG_STAT	7		/* read: status and error flags */
#define	ATA_REG_SLCT	0x206		/* write: device control */
#define	ATA_REG_ALTST	0x206		/* read: alternate status/error */

/* ATA drive command bytes */
#define	ATA_CMD_RD	0x20		/* read one sector */
#define	ATA_CMD_WR	0x30		/* write one sector */
#define	ATA_CMD_PKT	0xA0		/* ATAPI packet cmd */
#define	ATA_CMD_PID	0xA1		/* ATAPI identify */
#define	ATA_CMD_RDMUL	0xC4		/* read multiple sectors */
#define	ATA_CMD_WRMUL	0xC5		/* write multiple sectors */
#define	ATA_CMD_ID	0xEC		/* ATA identify */

#define	ATA_FLG_ATAPI	0x0001		/* ATAPI drive */
#define ATA_FLG_LBA	0x0002		/* LBA-capable */
#define ATA_FLG_DMA	0x0004		/* DMA-capable */

/* ATA sector size */
#define	ATA_LG_SECTSIZE		9	/* 512 byte ATA drive sectors */
#define	ATA_SECTSIZE		(1 << (ATA_LG_SECTSIZE))


#define PORT ide_interfaces[minor].port
//uncomment next line to enable IDE Debugmode
//#define IDE_DEBUG 0

unsigned int ide_pid;                   // ProcessID of ide driver
volatile unsigned int ide_interrupt_occured;

struct s_ide_interfaces                 // one structure for each drive
{
  unsigned short detect;                // drive connected?
  unsigned short drive_nr;              // 10 for hd0, 11 for hd1,....
  unsigned char serial_number[20];
  unsigned char model_number[40];
  unsigned char firmware[8];
  unsigned short port;

  unsigned short flags;

  unsigned short max_cyl;               // cylinders of drive
  unsigned short max_head;              // heads of drive
  unsigned short max_sec;               // sectors of drive
};

struct s_drive_para
{
  unsigned int start_cyl;
  unsigned int start_head;
  unsigned int start_sec;
  unsigned int start_block;

  unsigned int end_cyl;
  unsigned int end_head;
  unsigned int end_sec;
  unsigned int end_block;

  unsigned short flags;
};


struct partition
{
  unsigned char active;
  unsigned char start_head;
  unsigned short start_sec_cyl;
  unsigned char code;
  unsigned char end_head;
  unsigned short end_sec_cyl;
  unsigned long distance;
  unsigned long count;
};


struct s_ide_interfaces ide_interfaces[MAX_INTERFACES]={0};
struct s_drive_para     drive_para[MAX_INTERFACES*4+4+20]={0};

int part_id = 4;

// Prototypes
void gafb(unsigned short *buffer, unsigned char *str, unsigned int start, unsigned int size);
void block2hcs(int block,unsigned short interface, unsigned short *head,unsigned short *cyl,unsigned short *sector);


void CHardDisk::ide_wait_ns(int ns)
{
  int i;
  for (i=0;i<=ns*10;i++);
}

/************************************************************************/
/* Select a drive on a port
/************************************************************************/
/* Input: port = 0x170 or 0x1F0
/*        Sel = 0xA0 or 0xB0 (master, slave)
/************************************************************************/
int CHardDisk::ata_Select(unsigned short port, unsigned char Sel)
{
        unsigned short Temp;

	Temp = inportb(port + ATA_REG_DRVHD);
	if(((Temp ^ Sel) & 0x10) == 0)
        {
          //printk("already selected!");
          return(0); /* already selected */
        }
	outportb(port + ATA_REG_DRVHD, Sel);
	ide_wait_ns(20000);

	for(Temp=30000; Temp; Temp--)
	{
	  if((inportb(port + ATA_REG_STAT) & 0x80) == 0) break;
		ide_wait_ns(2000);
        }
	return(Temp == 0);
}

void CHardDisk::ata_cmd(char CmdByte, unsigned short drive, unsigned int block, unsigned int count)
{
    unsigned char Sect, DrvHd;
    unsigned short Cyl;
    unsigned int Temp;
    unsigned short port;

    port = ide_interfaces[drive].port;
/* compute CHS or LBA register values */
    Temp = block;

    if(ide_interfaces[drive].flags & ATA_FLG_LBA)
    {
      Sect=Temp;		        /* 28-bit sector adr: low byte */
      Cyl=Temp >> 8;		        /* middle bytes */
      DrvHd=Temp >> 24;	                /* high nybble */
      DrvHd=(DrvHd & 0x0F) | 0x40;      /* b6 enables LBA */
//      printk("ide.c: ata_cmd(): LBA=%d\n", Temp);
    }
    else
    {
      Sect=Temp % ide_interfaces[drive].max_sec + 1;
      Temp /= ide_interfaces[drive].max_sec;
      DrvHd=Temp % ide_interfaces[drive].max_head;
      Cyl=Temp / ide_interfaces[drive].max_head;
//      printk("ide.c: ata_cmd(): CHS=%d:%d:%d\n",Cyl, DrvHd, Sect);
     }
     DrvHd |= ide_interfaces[drive].drive_nr;

     outportb(port + ATA_REG_CNT, count);
     outportb(port + ATA_REG_SECT, Sect);
     outportb(port + ATA_REG_LOCYL, Cyl);

     Cyl >>= 8;
     outportb(port + ATA_REG_HICYL, Cyl);// >> 8);
     outportb(port + ATA_REG_DRVHD, DrvHd);

     ide_interrupt_occured = 0;
     outportb(port + ATA_REG_CMD, CmdByte);
}

int CHardDisk::ata_read(unsigned short drive, unsigned int block, unsigned int count, unsigned short *buffer)
{
  unsigned short port;
  unsigned char stat;
  int i;
  int j;


  port = ide_interfaces[drive].port;

  block--;              // 0 relativ
/* select the drive */
  if(ata_Select(port, ide_interfaces[drive].drive_nr))
  {
     printk("File: ide.c  Function: ata_read\n\n%s",
           "Could not select drive");
    return(-1);
  }
   i = 0;
   while(count)
   {
     /* compute CHS or LBA register values and write them, along with CmdByte */
     ata_cmd(ATA_CMD_RD, drive, block, count);

     /* await read interrupt */
     //printk("ide.c: ata_read() waiting for interrupt...");
     while (!ide_interrupt_occured);

     /* check status */
     stat=inportb(port + ATA_REG_STAT);
     if((stat & (0x81 | 0x58)) != 0x58)
     {
          printk("ide.cpp: ata_read() Bad status %d",stat);
  	  return(-4);
     }
     else
     {
       for (j=0;j<256;j++)
         buffer[i+j] = inportw(port + ATA_REG_DATA);

       i = i + j + 1;
       count--;
     }
   }
   return 0;
}

/************************************************************************/
/* Identify a drive
/* Fills the ide_interface structure
/************************************************************************/
/* Input: drivenr (1,2,3,4)
/************************************************************************/
void CHardDisk::ata_identify(unsigned short drivenr)
{
   unsigned short port;
   unsigned short Temp;
   unsigned char  Temp1, Temp2;
   unsigned short buffer[256];
   int i;

   port = ide_interfaces[drivenr].port;

   // Check if drive there
   if ((inportb(port + ATA_REG_CNT) != 0x01) ||
       (inportb(port + ATA_REG_SECT) != 0x01))
   {
   // printk("ide.c: ata_identify() no drive found?!?!.");
     ide_interfaces[drivenr].detect = 0;
     return;
   }
   // printk("identifing on port %x, drive %d",port, drivenr);

   Temp1 = inportb(port + ATA_REG_LOCYL);
   Temp2 = inportb(port + ATA_REG_HICYL);
   Temp  = inportb(port + ATA_REG_STAT);

   if(Temp1 == 0x14 && Temp2 == 0xEB)
   {
//      printk("ide.c: Drive %d: ATAPI CD-ROM found. ",drivenr);
      ide_interfaces[drivenr].flags |= ATA_FLG_ATAPI;
   }
   else if(Temp1 == 0 && Temp2 == 0 && Temp)
   {
//      printk("ide.c: Drive %d: IDE-ATA hard drive found. ",drivenr);
   }
   else
   {
     ide_interfaces[drivenr].detect = 0;
     return;
   }

   /* issue ATA 'identify drive' command */
   ide_interrupt_occured = 0;
   outportb(port + ATA_REG_CMD, ATA_CMD_ID);

/* ATA or ATAPI: get results of of identify */
   ide_wait_ns(2000);
   i=0;

   while (!ide_interrupt_occured)
   {
     i++;
     if (i==10000000)
     {
        alert("File: ide.c  Function: ata_identify\n\n%s",
              "Timeout...");
       ide_interfaces[drivenr].detect = 0;
       return;
     }
   }

   // read sector information
   inportb(port + ATA_REG_STAT);

   for (i=0;i<256;i++)
     buffer[i] = inportw(port + ATA_REG_DATA);

   gafb(buffer, ide_interfaces[drivenr].serial_number, 0x0A, 20);
   gafb(buffer, ide_interfaces[drivenr].model_number, 0x1B, 40);
   gafb(buffer, ide_interfaces[drivenr].firmware, 0x17, 8);

   if(buffer[31] & 1)
   {
	ide_interfaces[drivenr].flags |= ATA_FLG_DMA;
   }
   if(buffer[31] & 2)
   {
	ide_interfaces[drivenr].flags |= ATA_FLG_LBA;
   }

   if (ide_interfaces[drivenr].flags & ATA_FLG_ATAPI)
   {
//     printk("                %s %s %s",ide_interfaces[drivenr].model_number,
//     ide_interfaces[drivenr].firmware,ide_interfaces[drivenr].serial_number);
   }
   else
   {
//     printk("                %s CHS(%d/%d/%d)",ide_interfaces[drivenr].model_number,

     ide_interfaces[drivenr].max_cyl = buffer[1];
     ide_interfaces[drivenr].max_head = buffer[3];
     ide_interfaces[drivenr].max_sec = buffer[6];


   }
}

/************************************************************************/
/* Search for IDE Devices on the system.
/* Fills the structure ide_interfaces with controller requests
/************************************************************************/
/* Input: unsigned char nrdrives
/*        a value return by the BIOS at startup time
/************************************************************************/
void CHardDisk::ide_detect(char nrdrives)
{
  unsigned char loop;
  int i;
  unsigned short buffer[256];
  unsigned char str[256];
  int timeout=0;
  int found = 0;
  int ret;
  unsigned short port;
  unsigned short drive;

  printk("ide.c: Detecting and initializing ide devices...");

  ide_wait_ns(1000000);
  ide_interfaces[0].port = ide_interfaces[1].port = 0x1F0;
  ide_interfaces[2].port = ide_interfaces[3].port = 0x170;

  ide_interfaces[0].drive_nr = ide_interfaces[2].drive_nr = 0xA0;
  ide_interfaces[1].drive_nr = ide_interfaces[3].drive_nr = 0xB0;

  for (loop = 0; loop < MAX_INTERFACES; loop+=2)  /* Loop through drives */
  {
    port = ide_interfaces[loop].port;
    drive = ide_interfaces[loop].drive_nr;

    outportb(port + ATA_REG_CNT,  0x55);
    outportb(port + ATA_REG_SECT, 0xAA);

    if ((inportb(port + ATA_REG_CNT) != 0x55) ||
        (inportb(port + ATA_REG_SECT) != 0xAA))
    {
      // no master detected, maybe slave installed
      // printk("ide.c: No drive on port 0x%x!",port);
    }
    else
    {
      /* soft reset both drives on this I/F (selects master) */
      outportb(port + ATA_REG_SLCT, 0x0E);
      ide_wait_ns(20000);

      /* release soft reset AND enable interrupts from drive */
      outportb(port + ATA_REG_SLCT, 0x08);
      ide_wait_ns(20000);

      /* wait for master */
      for(i = 20000; i>0; i--)
      {
        if((inportb(port + ATA_REG_STAT) & 0x80) == 0) break;
        ide_wait_ns(2000);
       }
      if(i > 0)
      {
        // printk("ide.c: master found on I/F 0x%03X",port);
        ide_interfaces[loop].detect = 1;
        ata_identify(loop);

        // check slave on port
	if(ata_Select(port, 0xB0))
        // no response from slave: re-select master
        {
          ata_Select(port, 0xA0);
  	  ide_interfaces[loop+1].detect = 0;
          // printk("ide.c: ide_detect() no slave on I/F 0x%03X\n",port);
        }
        else                    // identify slave
        {
          ide_interfaces[loop+1].detect = 1;
  	  ata_identify(loop + 1);
        }
      }
      else
      {
        // check slave on port
	if(ata_Select(port, 0xB0))
        // no response from slave: re-select master
        {
          ata_Select(port, 0xA0);
  	  ide_interfaces[loop+1].detect = 0;
          // printk("ide.c: ide_detect() no slave on I/F 0x%03X\n",port);
        }
        else                    // identify slave
        {
          ide_interfaces[loop+1].detect = 1;
  	  ata_identify(loop + 1);
        }


      }
    }
  }
}

/*****************************************************************************
	name:	partProbe
	action:	analyzes partitions on ATA drives
*****************************************************************************/
void CHardDisk::ata_partProbe(void)
{
  struct partition part_entry;
  unsigned char Buffer[512], drive, part;
  unsigned int ex_part;
  unsigned char Buffer2[512];
  char LHeads, Heads, Sects, Temp3;
  unsigned int Temp, Track, Temp2;
  unsigned short Offset, Cyls;
  char* cp;
  unsigned int i;
  char str[255];
  unsigned int extended = 0;
  int name = 0;
  unsigned int extended_start = 0;

  unsigned int d_cyl, d_head, d_sec;

  printk("ide.c: Partition probe:\n");

  for(drive=0; drive < MAX_INTERFACES; drive++)
  {
    name = 0;
    if (ide_interfaces[drive].detect == 0) continue;
    if (ide_interfaces[drive].flags & ATA_FLG_ATAPI) continue;
/* load sector 1 (the partition table) */
 	 printk("ide.cpp: reading partition table of drive %d",drive);
    Temp = ata_read((unsigned short)drive, (unsigned int)1, (unsigned int)1, (unsigned short*)Buffer);

    if(Temp < 0) 
 	 {
		printk("ide.cpp: ata_read returned NULL. ???");
		continue;
	}
/* see if it's valid */
    if(Buffer[0x1FE] != 0x55 || Buffer[0x1FF] != 0xAA)
    {
        printk("Invalid parition table on drive hd%d",drive);
        printk("%02X %02X %02X %02X",(unsigned char*)Buffer[0],(unsigned char*)Buffer[1],
               (unsigned char*)Buffer[0x1FE],(unsigned char*)Buffer[0x1FF]);
        continue;
    }

/* check all four primary partitions for highest Heads value */
    LHeads=ide_interfaces[drive].max_head;

    for(part=0; part < 4; part++)
    {
/* now print geometry info for all primary partitions. CHS values in each
partition record may be faked for the benefit of MS-DOS/Win, so we ignore
them and use the 32-bit size-of-partition and sectors-preceding-partition
fields to compute CHS */
       extended = 0;

       Offset=0x1BE + 16 * part;
/* get 32-bit sectors-preceding-partition; skip if undefined partition */
       Temp=*(unsigned int *)(Buffer + Offset + 8);
       if(Temp == 0) continue;
/* convert to CHS, using LARGE mode value of Heads if necessary */
       Sects=Temp % ide_interfaces[drive].max_sec + 1;
       Track=Temp / ide_interfaces[drive].max_sec;
       Heads=Track % LHeads;
       Cyls=Track / LHeads;
       Temp2=*(unsigned int *)(Buffer + Offset + 12);

       Temp3=*(unsigned char *)(Buffer + Offset + 4);

       switch (Temp3)
       {
         case 0x01: sprintf(str,"FAT12"); break;
         case 0x04: sprintf(str,"FAT16"); break;
         case 0x05: sprintf(str,"extended"); extended = 1; break;
         case 0x06: sprintf(str,"FAT16 >32MB"); break;
         case 0x07: sprintf(str,"HPFS"); break;
         case 0x0B: sprintf(str,"FAT32"); break;
         case 0x0C: sprintf(str,"FAT32"); break;
         case 0x0E: sprintf(str,"FAT16"); break;
       }

       if (extended)
       {
         printk("ide.cpp: (extended) hd%d%c: start LBA=%d, start CHS=%d:"
  			  "%d:%d, Size: %dMB %s", drive, 'a' + name,
    		  Temp, Cyls, Heads, Sects, Temp2 * 512/1024/1024, str);
         extended_start = Temp;
       }
       else
       {
         printk("ide.cpp: (primary) hd%d%c: start LBA=%d, start CHS=%d:"
  			  "%d:%d, Size: %dMB %s", drive, 'a' + name,
    		  Temp, Cyls, Heads, Sects, Temp2 * 512/1024/1024, str);
       }

       drive_para[part_id].start_block = Temp;
       sprintf(str,"hd%d%c",drive,'a' + name);
       register_blkdev(DEVICE_DISK_MAJOR+part_id, str, ide_pid);
       vdm.AddName(this, str, part_id);
       part_id++;
       name++;

       if (extended)
       {
         printk("ide.cpp: Extended partition found... checking...");

         Temp2 = Temp;
         Temp2++;
         Temp = ata_read((unsigned short)drive, (unsigned int)Temp2, (unsigned int)1, (unsigned short*)Buffer2);

         if(Temp < 0)
         {
		      printk("ide.cpp: ata_read returned NULL. ???");
 		      continue;
         }

         for(ex_part=0; ex_part < 4; ex_part++)
         {
           Offset=0x1BE + 16 * ex_part;
// get 32-bit sectors-preceding-partition; skip if undefined partition
           Temp=*(unsigned int *)(Buffer2 + Offset + 8);
           if(Temp == 0) continue;
// convert to CHS, using LARGE mode value of Heads if necessary
           Sects=Temp % ide_interfaces[drive].max_sec + 1;
           Track=Temp / ide_interfaces[drive].max_sec;
           Heads=Track % LHeads;
           Cyls=Track / LHeads;
           Temp2=*(unsigned int *)(Buffer2 + Offset + 12);
           Temp3=*(unsigned char *)(Buffer2 + Offset + 4);

           switch (Temp3)
           {
             case 0x01: sprintf(str,"FAT12"); break;
             case 0x04: sprintf(str,"FAT16"); break;
             case 0x05: sprintf(str,"extended, not supported"); break;
             case 0x06: sprintf(str,"FAT16 >32MB"); break;
             case 0x07: sprintf(str,"HPFS"); break;
             case 0x0B: sprintf(str,"FAT32"); break;
             case 0x0C: sprintf(str,"FAT32"); break;
             case 0x0E: sprintf(str,"FAT16"); break;
          }

         Temp+=extended_start;
         printk("ide.cpp: hd%d%c (in extended): start LBA=%d, start CHS=%d:"
				"%d:%d, Size: %dMB %s", drive, 'a' + name,
				Temp, Cyls, Heads, Sects, Temp2 * 512/1024/1024, str);

         drive_para[part_id].start_block = Temp;
         sprintf(str,"hd%d%c",drive,'a' + name);
         register_blkdev(DEVICE_DISK_MAJOR+part_id, str, ide_pid);
         vdm.AddName(this, str, part_id);
         part_id++;
         name++;
       }
     }
    }
  }
}

void CHardDisk::Thread(void)
{
  struct ipc_message *m;
  unsigned short drive;
  unsigned int block;
  unsigned int minor;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
/*
        case MSG_READ_BLOCK:
             {
                
                m->type = MSG_READ_BLOCK_REPLY;
                send_msg(m->sender, m);
                break;
             }*/
	 case MSG_READ_BLOCK:
//		   show_msgf("hdd.cpp: reading from harddisk hd..");
		   
			minor = m->MSGT_READ_BLOCK_MINOR;
            block = m->MSGT_READ_BLOCK_BLOCKNUMMER + drive_para[minor].start_block;

            if (minor >= 4)         // read from a partition on disk
                m->MSGT_READ_BLOCK_RESULT = ata_read((unsigned int)minor/4 - 1, (unsigned int)block, (unsigned int)1, (unsigned short*)m->MSGT_READ_BLOCK_BUFFER);
            else                    // read from a disk
                m->MSGT_READ_BLOCK_RESULT = ata_read((unsigned int)minor, (unsigned int)block, (unsigned int)1, (unsigned short*)m->MSGT_READ_BLOCK_BUFFER);

		    m->type = MSG_READ_BLOCK_REPLY;
//          send_msg(m->source->pid, m);
          send_msg(m->sender, m);
		    break;

     }
  }
}

void ide_thread(void)
{
	CDevice *d;

	d = vdm.GetRegistrationDevice("ide0001");		/* return the CDevice object from the virtual device manager */

	if (d == NULL)
	{
		printk("hdd.cpp: No hd device found. Ooops!!");
		printk("         hdd thread already created. Please unregister device");
		printk("         and kill thread.");
		while(1);
	}
	d->Thread();
}

void ide_interrupt(void)
{
  ide_interrupt_occured = 1;

  // task_control(ide_pid, TASK_READY);
  outportb(0x20,0x20);
  outportb(0xA0, 0x20);
}


unsigned int CHardDisk::ReadBlock(unsigned int, char*)
{
}

unsigned int CHardDisk::WriteBlock(unsigned int, char*)
{
}



CHardDisk::CHardDisk() : CBlockDevice()
{
}

unsigned int CHardDisk::Initialize(void)
{
  int i=0;
  int found=0;
  char str[255];
  int id;
  char nrdrives = 0;

  // detect all ide drives in system...

  ide_detect(nrdrives);

  // check if there are someone
  while (ide_interfaces[i].detect == 0)
  {
    i++;
    if (i>=MAX_INTERFACES)
    {
      printk("ide.c: No IDE drives found. Giving Up!");
      return;
    }
  }

  id = register_hardware(DEVICE_IDENT_STORAGE_DEVICE, "IDE Harddisk Controller 1", ide_pid);
  register_irq(id, 14);
  register_port(id, 0x170, 0x177);
  id = register_hardware(DEVICE_IDENT_STORAGE_DEVICE, "IDE Harddisk Controller 2", ide_pid);
  register_port(id, 0x1F0, 0x1F7);
  register_irq(id, 15);

  // generate a device for each drive
  ide_pid = ((struct task_struct*)CreateKernelTask((unsigned int)ide_thread, "ide", RTP,0))->pid;

  SetPID(ide_pid);
  SetRegistrationName("ide0001");
  vdm.RegisterDevice(this);
  
  for (i=0;i<MAX_INTERFACES;i++)
  {
    if (ide_interfaces[i].detect)
    {
      if (ide_interfaces[i].flags == ATA_FLG_ATAPI)             // IDE CD
      {
         printk("ide.c: Drive %d: ATAPI CD-ROM found.",i);
         sprintf(str,"cd%d\0",i);
         register_blkdev(DEVICE_DISK_MAJOR+part_id, str, ide_pid);
         register_hardware(DEVICE_IDENT_STORAGE_DEVICE, "IDE ATAPI CD-ROM", ide_pid);
      	vdm.AddName(this, str, part_id);
         part_id++;
      }
      else                                                      // IDE HD
      {
         if (ide_interfaces[i].flags & ATA_FLG_LBA)
           sprintf(str,"LBA");
         else sprintf(str,"CHS");

         printk("ide.c: Drive %d: %s Mode: %s CHS(%d/%d/%d) %dMB",i,
           ide_interfaces[i].model_number,
           str,
           ide_interfaces[i].max_cyl,
           ide_interfaces[i].max_head,
           ide_interfaces[i].max_sec,
           ide_interfaces[i].max_cyl*ide_interfaces[i].max_head*ide_interfaces[i].max_sec *512 / 1024/1024);

         sprintf(str,"hd%d\0",i);
         register_blkdev(DEVICE_DISK_MAJOR+part_id, str, ide_pid);
  		   vdm.AddName(this, str, part_id);

         sprintf(str,"IDE Harddisk %s",ide_interfaces[i].model_number);
         register_hardware(DEVICE_IDENT_STORAGE_DEVICE, str, ide_pid);

//         register_hardware(DEVICE_IDENT_STORAGE_DEVICE, str, ide_pid);

         // build drive table
         drive_para[part_id].flags = ide_interfaces[i].flags;
         drive_para[part_id].start_block = 0;
         part_id++;
      }
    }
  }
  ata_partProbe();
}

// get ascii from buffer
void CHardDisk::gafb(unsigned short *buffer, unsigned char *str, unsigned int start, unsigned int size)
{
  unsigned char str2[256];
  unsigned char ch;
  unsigned char *pointer;
  int i;
  int j;

  memset(str2, 0, size);
  memcpy(str2, buffer+start, size);

  for (i=0;i<size;i+=2)
  {
    ch = str2[i];
    str2[i] = str2[i+1];
    str2[i+1] = ch;
  }

  i = size-2;

  while (str2[i] == ' ') i--;
  str2[i+1] = '\0';

  pointer = (unsigned char*)str2;
  while ((*pointer) == 0x20) pointer++;

  i = 0;
  while (*pointer != '\0')
  {
    str[i] = *pointer;
    pointer++;
    i++;
  }
  str[i] = '\0';

}

