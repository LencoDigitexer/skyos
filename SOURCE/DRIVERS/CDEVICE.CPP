/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\cdevice.cpp
/* Last Update: 15.04.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the CDevice class.
/************************************************************************/
#include "stdlib.h"
#include "vdm.h"


CDevice::CDevice(void)
{
	for (int i=0;i<30;i++) name[i] = 0;

}

unsigned int CDevice::Initialize(void)
{
}

void CDevice::Thread(void)
{
}


void CDevice::SetRegistrationName(char *str)
{
	strcpy(name, str);
}

char *CDevice::GetRegistrationName(void)
{
	return name;
}


void CDevice::SetPID(unsigned int p)
{
	thread_pid = p;
}

unsigned int CDevice::GetPID(void)
{
	return thread_pid;
}

int CDevice::GetData(void)
{
	return data;
}


CBlockDevice::CBlockDevice() : CDevice()
{
}
