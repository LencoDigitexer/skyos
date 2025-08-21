/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\critical.h
/* Last Update: 07.08.1999
/* Version    : alpha
/* Coded by   : Hayrapetian Gregory
/* Docus      :
/************************************************************************/

#define MAX_CRITICAL	10

typedef struct critical_section
{
  volatile int in_critical;
  int critical_path[MAX_CRITICAL];
  int ccount;

  struct critical_setcion *next;
} critical_section_t;
