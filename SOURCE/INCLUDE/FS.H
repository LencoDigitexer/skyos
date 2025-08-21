/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\fs.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Include file for msdos.c
/************************************************************************/
#include "msdos.h"

#define MAX_MOUNT 3
#define MAX_FILES 10

#define F_OPEN   1
#define F_CREATE 2

struct s_bootblock
{
 unsigned char Jump[3];
 unsigned char FileSystem[8];
 unsigned char MediaTyp;
 unsigned char SectorSize;
 unsigned char MaxTrack;
 unsigned char MaxHead;
 unsigned char MaxSector;
 unsigned char BeginTrack;
 unsigned char BeginHead;
 unsigned char BeginSector;
 unsigned char EndTrack;
 unsigned char EndHead;
 unsigned char EndSector;
 unsigned char BlockSize;
 unsigned char Routine[487];
 unsigned short ID_Code;
};

struct s_superblock
{
  int RootCatalog;
  int FirstINode;
  int FirstDataBlock;
  unsigned char reserved[512-4-4-4];
};

struct s_inode
{
  unsigned int size;
  unsigned char typ;
  unsigned int pointer1;
  unsigned int indirect1;
  unsigned int indirect2;
  unsigned char reserved[128 - 4 - 4 - 4 - 4 - 1];
};

struct s_catentry
{
  unsigned char name[14];
  unsigned short inode;
};

struct s_catalog
{
  struct s_catentry catentry[32];
};




/*****************************************************************************
/*****************************************************************************
/*****************************************************************************/
/* VFS Types */


struct super_block
{
  unsigned char devicename[10];
  int flags;
  struct vfs_inode i_mounted;
  struct msdos_super_block msdos;
};

struct mounttab
{
  int device;                       /* Device */
  unsigned char name[20];           /* Name of Filesystem */
  unsigned char path[255];          /* Path where device is mounted */
  int flags;                        /* Flags with which device is mounted */
  struct super_block super;
};

struct file_system_type
{
  unsigned char name[15];

  unsigned int (*init) (void);
  int (*read_super) (struct super_block *super, int device);
  int (*fread) (struct super_block *super, struct vfs_inode *inode, unsigned int bytes, unsigned char *buffer);
  int (*fwrite) (struct super_block *super, struct vfs_inode *inode, unsigned int bytes, unsigned char *buffer);
  int (*fopen) (struct super_block *super, struct vfs_inode *vinode, unsigned char *name, int flags);
  int (*fseek) (struct vfs_inode *inode, unsigned int pos);
  int (*find_entry) (struct super_block * super, struct vfs_inode *inode, unsigned char * dir,int nr, int type);
};

struct file
{
  int fp;
  struct vfs_inode inode;
};

