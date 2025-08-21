/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\ccm.h
/* Last Update: 04.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Include file for common controls module.
/************************************************************************/
#define COMMON_CONTROL_CINPUT           1

void CInput_draw(void);

struct CInput
{
  void (*CInput_draw) (void);

  unsigned int type;
  unsigned int x;
  unsigned int y;
  unsigned int length;
};

