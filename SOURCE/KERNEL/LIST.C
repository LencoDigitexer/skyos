/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : kernel\list.c
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Resl Christian
/* Docus from :
/************************************************************************/
/* Definition:
/*   Implements a signle linked list with functions to handle them.
/************************************************************************/
#include "list.h"
#include "system.h"


// returns the pointer to the first listelement
void *add_item(struct list *listptr, void *item, int size)
{
  struct list *ptr=listptr;
  unsigned int flags;

  save_flags(flags);
  cli();

  if (ptr==NULL)
  {
    ptr = (struct list*) valloc(sizeof(struct list));
    ptr->item = (unsigned char*)valloc(size);
    memcpy(ptr->item,item,size);
    ptr->next=NULL;
    restore_flags(flags);

    return ptr;
  }
  while (ptr->next!=NULL) ptr=ptr->next;

  ptr->next = (struct list*) valloc(sizeof(struct list));
  ptr=ptr->next;
  ptr->item = (unsigned char*)valloc(size);
  memcpy(ptr->item,item,size);
  ptr->next=NULL;

  restore_flags(flags);
  return listptr;
}

// return the pointer to the selected item
void *get_item(struct list *listptr, int number)
{
  struct list *ptr=listptr;
  unsigned int flags;

  save_flags(flags);
  cli();

  while (number)
  {
    if (ptr==NULL)
    {
       restore_flags(flags);
       return NULL;
    }

    ptr=ptr->next;
    number--;
  }

   restore_flags(flags);

   if (ptr == NULL) return NULL;

   return ptr->item;
}

void *del_item(struct list *listptr, int number)
{
  struct list *ptr=listptr;
  struct list *helpptr=listptr;
  unsigned int flags;

  save_flags(flags);
  cli();

  while (number--)
  {
    if (ptr==NULL)
    {
      restore_flags(flags);
      return listptr;
    }
    helpptr = ptr;
    ptr = ptr->next;
  }

  if (ptr == listptr)
  {
    listptr = listptr->next;
    vfree(ptr->item);
    vfree(ptr);
    restore_flags(flags);
    return listptr;
  }
  helpptr->next=ptr->next;

  vfree(ptr->item);
  vfree(ptr);

  restore_flags(flags);
  return listptr;
}


