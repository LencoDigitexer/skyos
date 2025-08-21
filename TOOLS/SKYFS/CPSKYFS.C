#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <bios.h>
#include "cpskyfs.h"

int device = -1;
int partition = -1;
int start_head, start_track, start_sector;
int head, track, sector;
int end_head, end_track, end_sector;

void next_sector();
void add_blocknr_to_inode(unsigned long blocknr, struct skyfs_inode *inode);

void readblock(int device,unsigned long blocknr,void *buffer)
{
 int i;

 head = start_head;
 track = start_track;
 sector = start_sector;

 for (i=1;i<blocknr;i++) next_sector();

 for (i=0;i<MAX_TRY;i++)
 {
  if (biosdisk(2,device,head,track,sector,1,buffer) == 0) return;
  biosdisk(0,device,head,track,sector,1,buffer);
 }
 printf("\n\nERROR: Not able to read sector\n");
 exit(1);
}

void writeblock(int device,unsigned long blocknr,void *buffer)
{
 int i;

 head = start_head;
 track = start_track;
 sector = start_sector;

 for (i=1;i<blocknr;i++) next_sector();

// printf("%d:%d:%d\n",head,track,sector);

 for (i=0;i<MAX_TRY;i++)
 {
  if (biosdisk(3,device,head,track,sector,1,buffer) == 0) return;
  biosdisk(0,device,head,track,sector,1,buffer);
 }
 printf("\n\nERROR: Not able to write sector\n");
 exit(1);
}

void next_sector()
{
 union REGS regs;

 track=track+(((sector&0x00C0)<<2)|((head&0x00C0)<<4));
 head=head&0x003F;
 sector=sector&0x003F;
 sector++;
 if (sector>(end_sector&0x003F))
 {
  sector=1;
  head++;
  if (head>(end_head&0x003F))
  {
   head=0;
   track++;
   if (track>(end_track+(((end_sector&0x00C0)<<2)|((end_head&0x00C0)<<4))))
   {
    printf("\n\nERROR: Not enough diskspace\n");
    exit(1);
   }
  }
 }
 head=head+((track>>4)&0x00C0);
 sector=sector+((track>>2)&0x00C0);
 track=track&0x00FF;
}

void block_alloc(int count,unsigned long *block)
{
  unsigned long buffer[128];

  // read freeblock.sys
  readblock(device,4,buffer);

  // volume full?
  if ((buffer[0] + count) >= buffer[1]+1)
  {
    printf("\n\nERROR: Not enough diskspace (logically)\n");
    exit(1);
  }

  *block = buffer[0];

  // signed block as used
  buffer[0] = buffer[0] + count;
  writeblock(device,4,buffer);
}

void skyfs_write(int device, unsigned char *filename)
{
  int handle;
  unsigned char buffer[512] = {0};
  unsigned long blocknr, blocknr2;
  int i, block_count;
  struct skyfs_inode inode[4];
  float per;
  float act;
  float max;
  int y;

  // open source file
  if ((handle = open(filename, O_RDONLY | O_BINARY)) == -1)
  {
    printf("\nERROR: Cannot open file \"%s\"\n",filename);
    exit(1);
  }

  // alloc needed blocks
  block_count = filelength(handle) / 512;
  if (filelength(handle) % 512) block_count++;
  block_count++;

  block_alloc(block_count,&blocknr);

  // add new block to allocation list of rootinode
  readblock(device,2,inode);
  add_blocknr_to_inode(blocknr,inode);
  add_blocknr_to_inode(0,inode);
  writeblock(device,2,inode);

  // write inode and content
  blocknr2 = blocknr;
  memcpy(inode,buffer,512);
  strcpy(inode[0].name,filename);
  i = read(handle,buffer,512);

  max = filelength(handle);
  if (max == 0) max = 1;
  act = 512;
  while (i > 0)
  {
    per = 100 / max * act;
    printf("Written: %003.1f\%\n",per);
    y = wherey();
    gotoxy(1,y-1);
    writeblock(device,++blocknr,buffer);
    add_blocknr_to_inode(blocknr,inode);
    add_blocknr_to_inode(0,inode);
    i = read(handle,buffer,512);
    act += i;
  }
  inode[0].size = filelength(handle);
  writeblock(device,blocknr2,inode);

  // close source file
  close(handle);
}

void add_blocknr_to_inode(unsigned long blocknr, struct skyfs_inode *inode)
{
  unsigned long blocknr2;
  unsigned long blocknr_buffer[128];
  int offset = inode->size / 512;

  if (blocknr) inode[0].size = inode[0].size + 512;

  if (offset < 8)
  {
    inode->direct[offset] = blocknr;
  }
  else if (offset < 136)
  {
    offset = offset - 8;
    if (!inode->indirect[0])
    {
      block_alloc(1,&blocknr2);
      inode->indirect[0] = blocknr2;
    }
    blocknr2 = inode->indirect[0];
    readblock(device,blocknr2,blocknr_buffer);
    blocknr_buffer[offset] = blocknr;
    writeblock(device,blocknr2,blocknr_buffer);
  }
  else if (offset < 264)
  {
    offset = offset - 136;
    if (!inode->indirect[1])
    {
      block_alloc(1,&blocknr2);
      inode->indirect[1] = blocknr2;
    }
    blocknr2 = inode->indirect[1];
    readblock(device,blocknr2,blocknr_buffer);
    blocknr_buffer[offset] = blocknr;
    writeblock(device,blocknr2,blocknr_buffer);
  }
  else if (offset < 392)
  {
    offset = offset - 264;
    if (!inode->indirect[2])
    {
      block_alloc(1,&blocknr2);
      inode->indirect[2] = blocknr2;
    }
    blocknr2 = inode->indirect[2];
    readblock(device,blocknr2,blocknr_buffer);
    blocknr_buffer[offset] = blocknr;
    writeblock(device,blocknr2,blocknr_buffer);
  }
}

void parse(unsigned char *name)
{
  unsigned char buffer[512];

  if (!strncmp(name,"fd1",3)) device = 0x00;
  if (!strncmp(name,"fd2",3)) device = 0x01;
  if (!strncmp(name,"hd0",3)) device = 0x80;
  if (!strncmp(name,"hd1",3)) device = 0x81;
  if (device == -1)
  {
    printf("\nInvalid device\n");
    exit(1);
  }
  if (!strncmp(name,"fd",2))
  {
    start_head = 0; head = 0; end_head = 1;
    start_track = 0; track = 0; end_track = 79;
    start_sector = 1; sector = 1; end_sector = 18;
  }
  if (!strncmp(name,"hd",2))
  {
    start_head = 0; head = 0; end_head = 0;
    start_track = 0; track = 0; end_track = 0;
    start_sector = 1; sector = 1; end_sector = 1;

    readblock(device,1,buffer);
    if (name[3]=='a') partition = 1;
    if (name[3]=='b') partition = 2;
    if (name[3]=='c') partition = 3;
    if (name[3]=='d') partition = 4;
    if (partition == -1)
    {
      printf("\nInvalid partition\n");
      exit(1);
    }
    head = buffer[0x1AF+partition*0x10];
    track = buffer[0x1B1+partition*0x10];
    sector = buffer[0x1B0+partition*0x10];
    track = track + ((sector & 0xC0) << 2);
    track = track + ((head & 0xC0) << 4);
    sector = sector & 0x3F;
    head = head & 0x3F;
    start_head = head;
    start_track = track;
    start_sector = sector;
    end_head = buffer[0x1B3+partition*0x10];
    end_track = buffer[0x1B5+partition*0x10];
    end_sector = buffer[0x1B4+partition*0x10];
    end_track = end_track + ((end_sector & 0xC0) << 2);
    end_track = end_track + ((end_head & 0xC0) << 4);
    end_sector = end_sector & 0x3F;
    end_head = end_head & 0x3F;

    if (sector == 0)
    {
      printf("\nPartition does not exist\n");
      exit(1);
    }

    readblock(device,1,buffer);
    if (strncmp(&buffer[3],"SKYFS",5))
    {
      printf("\nPartition is no SKY partition. Run MKSKYFS.EXE first!\n");
      exit(1);
    }
    printf("\nIndicator SKYFS found\n");
  }
}

void testoutput(unsigned long blocknr, unsigned long blocknr2)
{
  struct skyfs_inode inode[4];
  unsigned long bb[128] = {0};

  readblock(device,blocknr,inode);
  readblock(device,blocknr2,bb);
  printf("\nInode");
  printf("\n  Name: %s",inode[0].name);
  printf("\n  Size: %u",inode[0].size);
  printf("\n  DP:  %u %u %u %u %u %u %u %u",inode[0].direct[0],inode[0].direct[1],inode[0].direct[2],inode[0].direct[3],inode[0].direct[4],inode[0].direct[5],inode[0].direct[6],inode[0].direct[7]);
  printf("\n  IP:  %u %u %u",inode[0].indirect[0],inode[0].indirect[1],inode[0].indirect[2]);
  printf("\nBlockbuffer: %u %u %u %u %u %u   %u",bb[0],bb[1],bb[2],bb[3],bb[4],bb[5],bb[127]);
}

void main(int argc,char *argv[])
{
  unsigned long blocknr, blocknr2;
  char string[20];

  if (argc<3)
  {
    printf("\nUsage: cpskyfs <file> <device>\n");
    printf("\ndevice: fd1, fd2, hd0a, hd0b, hd0c, hd0d, hd1a, hd1b, hd1c, hd1d\n");
    exit(0);
  }

  parse(argv[2]);

  printf("\nWriting \"%s\" on device %s (0x%.2X)",argv[1],argv[2],device);
  if (partition != -1) printf(" partition %d",partition);
  printf("...   \n");

  skyfs_write(device,argv[1]);

/*  testoutput(2,1);
  do
  {
    printf("\nBlocknr: ");
    gets(string);
    blocknr = atoi(string);
    printf("\nBlocknr: ");
    gets(string);
    blocknr2 = atoi(string);
    if (blocknr) testoutput(blocknr,blocknr2);
  }while (blocknr);*/

  printf("success\n");
}
