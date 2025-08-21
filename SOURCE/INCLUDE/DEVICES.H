/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\devices.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Includes all major and minor numbers for the devices.
/************************************************************************/
#define MAX_DEVICES                     40

#define ROOT_DEV DEVICE_RAMDISK_MAJOR

#define DEVICE_FLOPPY_MAJOR              1
#define DEVICE_FLOPPY_NAME               "fd1"

// DISKS reaches from 10 up to 20
#define DEVICE_DISK_MAJOR                10

#define DEVICE_KEYBOARD_MAJOR            30
#define DEVICE_KEYBOARD_NAME             "keyboard"

struct s_devices
{
  int major;
  unsigned char name[20];
  int pid;
};

struct file_operations
{
       int (*lseek) (void);
       int (*read) (int, unsigned char*);
       int (*write) (int, unsigned char*);
       int (*readdir) (void);
       int (*select) (void);
       int (*ioctl) (void);
       int (*mmap) (void);
       int (*open) (void);
       void (*release) (void);
       int (*fsync) (void);
};

