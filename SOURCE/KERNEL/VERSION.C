/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\version.c
/* Last Update: 12.11.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus from :
/************************************************************************/
/* Definition:
/*   This file contains datas, which a version dependend.
/************************************************************************/
#include "version.h"

void kernelinfo(void)
{
   console_clear();

   printk("SKY Operating System V2.0j is booting...\n");
   printk("(c) 1998 by Szeleney Robert, Gregory Hayrapetian and Resl Christian\n");
   printk("Build: %s / %s    Kernel Version: %s",__DATE__,__TIME__, KERNEL_VERSION);
   printk("For more informations or donwload see http://members.xoom.com/skyos/\n\n");
   printk("");
}

