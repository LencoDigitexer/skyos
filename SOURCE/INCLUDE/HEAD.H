/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\head.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Definition of INTEL 80386+ segments
/************************************************************************/
typedef struct desc_struct
{
 unsigned long a,b;
} desc_table[256];

extern desc_table idt,gdt,ldt;

struct segdesc_s
{
  unsigned short limit_low;
  unsigned short base_low;
  unsigned char base_middle;
  unsigned char access;
  unsigned char granularity;
  unsigned char base_high;
};
