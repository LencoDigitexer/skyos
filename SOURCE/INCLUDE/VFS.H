/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\vfs.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Virtual File system include file.
/************************************************************************/
#define BLOCKSIZE          1
#define MAX_OPEN_FILES    20

#include "msdos.h"
#include "skyfs.h"

struct superblock
{
 union
 {
   struct fat_superblock fat_sb;
 } u;
};

struct inode
{
 // do not insert data here! skyfs copys data into the first byte of this structure
 // in the mount function
 union
 {
   struct skyfs_inode skyfs_inode;
   struct fat_dir_entry fat_dir_entry;
 }u;
 struct superblock sb;

 unsigned char *cache;
 unsigned int cache_index;

 unsigned char *device;

};

struct mount_item
{
  unsigned char *device;        // name of the device
  unsigned char *protocol;      // protocol for the device
  unsigned char *rootpath;      // complete path of the root
  struct inode *rootinode;      // inode of the root
  unsigned char *actpath;       // complete path of the actual directory
  struct inode *actinode;       // inode of the actual directory
  char *superblock;             // buffer to store superblock infos
};

struct protocol_item
{
  unsigned char *protocol;      // name of the protocol
  unsigned int pid;             // process id of the task
};

struct file_item
{
  unsigned int fd;              // filedescriptor (unique number)

  struct inode *inode;          // inode of the file
  struct inode *rootinode;      // root-inode of the file
  unsigned int fp;              // filepointer of the file
  unsigned int size;

  unsigned int binary;
  unsigned char *device;

  unsigned char *fn;            // filename (complete path and name)
  int mode;                     // openmode of the file
  int pid;                      // pid of the task who has opened the file
};




