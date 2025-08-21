#include <stdio.h>
#include <string.h>
#include <process.h>
#include <bios.h>
#include "mkskyfs.h"

int device = -1;
int partition = -1;
int head = 0;
int track = 0;
int sector = 1;

void readblock(int device,int head,int track,int sector,void *buffer)
{
 int i;
 for (i=0;i<MAX_TRY;i++)
 {
  if (biosdisk(2,device,head,track,sector,1,buffer) == 0) return;
  biosdisk(0,device,head,track,sector,1,buffer);
 }
 printf("\n\nERROR: Not able to read sector\n");
 exit(1);
}

void writeblock(int device,int head,int track,int sector,void *buffer)
{
 int i;
 for (i=0;i<MAX_TRY;i++)
 {
  if (biosdisk(3,device,head,track,sector,1,buffer) == 0) return;
  biosdisk(0,device,head,track,sector,1,buffer);
 }
 printf("\n\nERROR: Not able to write sector\n");
 exit(1);
}

void skyfs_mkfs(int device, unsigned char *name)
{
  int i;
  struct skyfs_superblock superblock = {0};
  unsigned long buffer[128] = {0};
  struct skyfs_inode inode[4] = {0};

  // create superblock
  strcpy(superblock.filesystem,"SKYFS");
  writeblock(device,head,track,sector,&superblock);

  // create root-inode
  strcpy(inode[0].name,name);
  inode[0].type = DIRECTORY;
  inode[0].size = 512;
  inode[0].direct[0] = 3;
  writeblock(device,head,track,sector+1,inode);

  // create inode for freeblocks.sys
  strcpy(inode[0].name,"freeblocks.sys");
  inode[0].type = FILE;
  inode[0].size = 8;
  inode[0].direct[0] = 4;
  writeblock(device,head,track,sector+2,inode);

  // create content of freeblocks.sys
  buffer[0] = 5;                        // first free block
  buffer[1] = 2880;                     // last free block (later get_device_maxblock(device))
  for (i=2;i<128;i++) buffer[i] = 0;
  writeblock(device,head,track,sector+3,buffer);
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
    printf("\ninvalid device\n");
    exit(1);
  }
  if (!strncmp(name,"hd",2))
  {
    readblock(device,0,0,1,buffer);
    if (name[3]=='a') partition = 1;
    if (name[3]=='b') partition = 2;
    if (name[3]=='c') partition = 3;
    if (name[3]=='d') partition = 4;
    if (partition == -1)
    {
      printf("\ninvalid partition\n");
      exit(1);
    }
    head = buffer[0x1AF+partition*0x10];
    track = buffer[0x1B1+partition*0x10];
    sector = buffer[0x1B0+partition*0x10];
    track = track + ((sector & 0xC0) << 2);
    track = track + ((head & 0xC0) << 4);
    sector = sector & 0x3F;
    head = head & 0x3F;
    if (sector == 0)
    {
      printf("\npartition does not exist\n");
      exit(1);
    }
  }
}

void main(int argc,char *argv[])
{
  unsigned char string[10];
  unsigned char buffer[512];
  int i;

  if (argc<3)
  {
    printf("\nUsage: mkskyfs <device> <label>\n");
    printf("\ndevice: fd1, fd2, hd0a, hd0b, hd0c, hd0d, hd1a, hd1b, hd1c, hd1d\n");
    printf("label: specifies the name of the skyfs volume\n");
    exit(0);
  }

  parse(argv[1]);

  printf("\nCreating SKYFS on device %s (0x%.2X)",argv[1],device);
  if (partition != -1) printf(" partition %d",partition);

  printf("\nAre you sure? (y/n)");
  gets(string);
  if (string[0] != 'y') exit(0);

  skyfs_mkfs(device,argv[2]);
  printf("\nSKYFS created on device %s\n",argv[1]);
}