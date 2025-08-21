/************************************************************************/
/* Sky Operating System V2
/* Copyright (c)1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\main.c
/* Last Update: 13.12.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*  Internet service interface.
/************************************************************************/
#include "module.h"

void inet_init(void)
{
  printk("inet.c: Starting inet services...");

  tftpd_init();
#if (!MODULE_FTP)
  ftpd_init();
#endif
}
