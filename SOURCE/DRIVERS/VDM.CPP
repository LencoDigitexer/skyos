/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : drivers\vdm.cpp
/* Last Update: 15.04.1999
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   This file implements the virtual device manager object.
/************************************************************************/
#include "vdm.h"
#include "fdd.h"
#include "hdd.h"
#include "msgbox.h"

#include "stdlib.h"
#include "sched.h"
#include "error.h"

extern "C" unsigned int VDM_Initialize(void);
extern "C" void show_devices(void);
unsigned int TASK_VDM = 0;

extern unsigned int cache_active;


CVDM vdm;		/* This is the global Virtual Device Manager Object.
				   There is only one instance of this object in the system.
				   Every device must be registered in this class. */
				

CVDM::CVDM()
{
	memset(device_names, 0, MAX_DEV*255);
	for (int i=0;i<MAX_DEV;i++)
		device_objects[i] = NULL;
}


/************************************************************************/
/* Register a device													*/
/************************************************************************/
/* A pointer to the device class (e.g. CDiskFloppy) is stored in the	*/
/* VDM list.		
/* Then, the initialize function of the device is called.													*/
/************************************************************************/
unsigned int CVDM::RegisterDevice(CDevice *new_dev)
{
	int i=0;

	while (registered_devices[i] != NULL)
	{ 
		i++;
		if (i==MAX_DEV)
		{
			printk("vdm.cpp: Too much devices registered.");
			return 0;
		}
	}
	registered_devices[i] = new_dev;
}

unsigned int CVDM::AddName(CDevice *dev, char *name, int data)
{
	unsigned int i = 0;

	while (device_names[i][0] != '\0')
	{
		i++;
		if (i==MAX_DEV)
		{
			show_msgf("vdm.cpp: Too much alias names for devices registered.");
			return 0;
		}
	}
	strcpy(device_names[i], name);
	
	device_objects[i] = dev;
	device_minor[i] = data;
	
}

void CVDM::ShowRegisteredDevices(void)
{
	char str[255];

	for (int i=0;i<MAX_DEV;i++)
	{
		if (device_names[i][0] != '\0')
		{
			sprintf(str,"Devicename: %s   Data: %d     Thread: %d",device_names[i], device_minor[i], device_objects[i]->GetPID());
			show_msgf(str);
		}
	}
}

void show_devices(void)
{
	vdm.ShowRegisteredDevices();
}

/************************************************************************/
/* Get a CDevice Object													*/
/************************************************************************/
/* This functions return the CDevice object with name str
/* If no such object is found, NULL is returned
/************************************************************************/
CDevice* CVDM::GetDeviceFromName(char *str)
{
	int i=0;
	char *p;

	while (i < MAX_DEV)
	{ 
          if (!strcmp(device_names[i], str))
	  {
//                  printk("vdm.cpp: CVDM::GetDeviceFromName SUCCESS.");
		  device_objects[i]->data = device_minor[i];
		  return device_objects[i];
	  }
	  i++;
	}
	return NULL;
}

CDevice* CVDM::GetRegistrationDevice(char *str)
{
	int i=0;
	char *p;

	while (i < MAX_DEV)
	{ 
	  if (registered_devices[i] != NULL)
	  {
		  if (!strcmp(registered_devices[i]->GetRegistrationName(), str))
			  return registered_devices[i];
	  } 
          i++;
	}
	return NULL;
}


/************************************************************************/
/* Read from a Device  													*/
/************************************************************************/
/* This functions handles a Read call for a device.
/************************************************************************/
unsigned int CVDM::ReadFromDevice(struct ipc_message *m)
{
   unsigned char buffer[512];
  CDevice *dev;

  dev = GetDeviceFromName(m->MSGT_READ_BLOCK_DEVICENAME);

  if (dev == NULL)
  {
	  printk("vdm.cpp: Device %s not registered.", m->MSGT_READ_BLOCK_DEVICENAME);
	  m->type = MSG_READ_BLOCK_REPLY;
	  m->MSGT_READ_BLOCK_RESULT = ERROR_DEVICE_NOT_FOUND;
	  send_msg(m->sender, m);
  }

  else
  {
//    printk("vdm.cpp: reading from device %s",dev->name);
      m->MSGT_READ_BLOCK_MINOR = dev->data;

      // look in cache
      if (cache_active)
      {
//        msgbox(ID_OK,MODAL,"Dev is %d",dev->data);

        if (cache_hit(dev->data, m->MSGT_READ_BLOCK_BLOCKNUMMER, m->MSGT_READ_BLOCK_BUFFER))
        {
//            msgbox(ID_OK,MODAL,"Found in cache");
            m->type = MSG_READ_BLOCK_REPLY;
            m->MSGT_READ_BLOCK_RESULT = 0;
            send_msg(m->sender, m);
            return 0;
        }
//        else msgbox(ID_OK,MODAL,"Not found in cache");
      }
	   send_msg(dev->GetPID(), m);
  }
}

/************************************************************************/
/* Write to a Device  													*/
/************************************************************************/
/* This functions handles a Write call for a device.
/************************************************************************/
unsigned int CVDM::WriteToDevice(struct ipc_message *m)
{		 
  CDevice *dev;

  dev = GetDeviceFromName(m->MSGT_WRITE_BLOCK_DEVICENAME);

  if (dev == NULL)
  {
	  printk("vdm.cpp: Device %s not registered.", m->MSGT_WRITE_BLOCK_DEVICENAME);
	  m->type = MSG_WRITE_BLOCK_REPLY;
	  m->MSGT_WRITE_BLOCK_RESULT = ERROR_DEVICE_NOT_FOUND;
	  send_msg(m->sender, m);
  }

  else
  {
      m->MSGT_WRITE_BLOCK_MINOR = dev->data;
      send_msg(dev->GetPID(), m);
  }
}

/************************************************************************/
/* Virtual Device Manager Thread
/************************************************************************/
void CVDM::Thread(void)
{
  struct ipc_message *m;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

	 switch (m->type)
	 {
	 case MSG_READ_BLOCK:
		 ReadFromDevice(m);
		 break;
	 case MSG_WRITE_BLOCK:
		 WriteToDevice(m);
		 break;
    case MSG_READ_BLOCK_REPLY:
       if (cache_active)
       {
          cache_add(m->MSGT_READ_BLOCK_MINOR, m->MSGT_READ_BLOCK_BLOCKNUMMER,
                    m->MSGT_READ_BLOCK_BUFFER);
//          msgbox(ID_OK,MODAL,"added to cache");
       }
       send_msg(m->source->pid, m);
       break;
    case MSG_WRITE_BLOCK_REPLY:
       if (cache_active)
       {
          cache_add(m->MSGT_READ_BLOCK_MINOR, m->MSGT_READ_BLOCK_BLOCKNUMMER,
                    m->MSGT_READ_BLOCK_BUFFER);
//          msgbox(ID_OK,MODAL,"added to cache");
       }
       send_msg(m->source->pid, m);
       break;
	 }
  }
}

/************************************************************************/
/* VDM Thread Entry Point
/************************************************************************/
/* This function must be a standard C procedure.
/* Here we call the memberfunction of the VDM object.
/************************************************************************/
void VDM_Thread_Entry(void)
{
	vdm.Thread();

}

/************************************************************************/
/* Initialize CVDM object        
/************************************************************************/
/* Initializes the CVDM object and creates a thread
/************************************************************************/
unsigned int CVDM::Initialize(void)
{
    printk("vdm.cpp: Initializing virtual device manager...");
    
	for (int i=0; i < MAX_DEV; i++)
		registered_devices[i] = NULL;

    // create the vdm-thread now.

	TASK_VDM = ((struct task_struct*)CreateKernelTask(VDM_Thread_Entry, "vdm", RTP,0))->pid;
	printk("vdm.cpp: Creating virtual device mananger thread, pid = %d...",TASK_VDM);
}


/************************************************************************/
/* Initialize CVDM object        
/************************************************************************/
/* This functions is callled from C code to initialize the cvdm class.
/* What to do for registering devices:
/*   1. Construct a derived CDevice object
/*   2. Call the initialize function
/*			Create a Thread for the device
/*			Register the device 
/*			Set the name of the device
/************************************************************************/
unsigned int VDM_Initialize(void)
{
	char buffer[512]={0};

	vdm.Initialize();

	// Register CDiskFloppy object
	CDiskFloppy *fdd;

	fdd = new CDiskFloppy();
        fdd->Initialize();

	// Register CHardDisk object
	CHardDisk *hdd;

	hdd = new CHardDisk();
	hdd->Initialize();
}


