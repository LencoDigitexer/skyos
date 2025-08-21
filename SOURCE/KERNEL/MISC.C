/************************************************************************/
/* Sky Operating System V2
/* Copyright (c)1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\misc.c
/* Last Update: 09.01.1999
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Some useful functions.
/************************************************************************/
#include "error.h"

void error(int value)
{
  switch(value)
     {
       case SUCCESS:show_msg("success");break;
       case ERROR_DEVICE_NOT_FOUND:show_msg("device not found");break;
       case ERROR_READ_BLOCK:show_msg("error reading block");break;
       case ERROR_WRITE_BLOCK:show_msg("error writing block");break;
       case ERROR_DEVICE_MOUNTED:show_msg("device already mounted");break;
       case ERROR_DEVICE_NOT_MOUNTED:show_msg("device not mounted");break;
       case ERROR_PROTOCOL_REGISTERED:show_msg("protocol already registered");break;
       case ERROR_PROTOCOL_NOT_REGISTERED:show_msg("protocol not registered");break;
       case ERROR_WRONG_PROTOCOL:show_msg("wrong protocol for the device");break;
       case ERROR_FILE_NOT_FOUND:show_msg("file not found");break;
       case ERROR_FILE_NOT_OPEN:show_msg("file not open");break;
       case ERROR_CREATE_FILE:show_msg("error creating file");break;
       case ERROR_FILE_OPEN:show_msg("all files of the device must be closed before unmounting"); break;
       case ERROR_FILE_USED:show_msg("file already used");break;
       default:show_msgf("unknown return value %d",value);
     }
}

void parse(unsigned char *str, unsigned char *para, int nr)
{
  int i=0,j=0;
  while (str[i] == 0x20) i++;
  while (j<nr)
  {
    while ((str[i] != 0x20) && (str[i] != 0x00)) i++;
    while (str[i] == 0x20) i++;
    j++;
  }
  strcpy(para, &str[i]);
  j=i;
  i=0;
  while ((str[j] != 0x20) && (str[j] != 0x00))
  {
    j++;
    i++;
  }
  para[i]=0x00;
}
