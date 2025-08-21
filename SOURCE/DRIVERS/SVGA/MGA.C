/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\svga\mga.c
/* Last Update: 30.07.1999
/* Version    : beta
/* Coded by   : Gregory Hayrapetian
/* Docus      : Internet
/************************************************************************/
/* Definition:
/*   A Matrox MGA driver
/************************************************************************/
#include "svga\svgadev.h"
#include "newgui.h"

#define NULL (void*)0


int mga_detect();
void mga_init();

display_device_driver display_driver_mga =
{
    "Matrox MGA",
    "Alpha",
    mga_detect,
    mga_init,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    0,
    0,
    0,
    0
};


int mga_detect()
{


  return 0;
}



