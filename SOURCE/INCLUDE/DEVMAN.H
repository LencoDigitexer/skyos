/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\devmen.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Include file for devman.c
/************************************************************************/
struct s_port
{
  unsigned int from;
  unsigned int to;
  struct s_port* next;
};

struct s_memory
{
  unsigned int from;
  unsigned int to;
  struct s_memory* next;
};

struct s_irq
{
   unsigned int nummer;
   struct s_irq *next;
};

struct s_dma
{
   unsigned int nummer;
   struct s_dma *next;
};

struct s_irq_usage
{
  unsigned int used;
  struct s_hardware *hardware;
};

struct s_hardware
{
  unsigned char name[255];
  unsigned int id;

  unsigned int devclass;

  unsigned int driver_pid;
  int (*configure_driver)(void);

  unsigned int pci;
  unsigned int pnp;

  struct list *port;
  struct list *memory;
  struct list *irq;
  struct list *dma;

  struct s_hardware *next;
};


enum {
    DEVICE_IDENT_OTHER,
    DEVICE_IDENT_STORAGE_DEVICE,
    DEVICE_IDENT_NETWORK,
    DEVICE_IDENT_DISPLAY,
    DEVICE_IDENT_MULTIMEDIA,
    DEVICE_IDENT_MEMORY,
    DEVICE_IDENT_BRIDGE_CONTROLLER,
    DEVICE_IDENT_COMMUNICATIONS,
    DEVICE_IDENT_SYSTEM_PHERIPHERAL,
    DEVICE_IDENT_INPUT,
    DEVICE_IDENT_DOCKING,
    DEVICE_IDENT_CPU,
    DEVICE_IDENT_SERIAL_BUS
};

