/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\twm.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Include file for the twm.
/************************************************************************/
struct s_common_object
{
  void (*draw) (void);
  unsigned int reserved[20];
};

struct s_common_list
{
  struct s_common_object *object;
  struct s_common_list *next;
};

struct twm_window
{
  unsigned int window_id;
  unsigned short x;
  unsigned short y;
  unsigned short length;
  unsigned short heigth;
  unsigned char title[40];
  unsigned short acty;
  unsigned short actx;
  unsigned short style;

  struct s_common_list *common_list;
};


