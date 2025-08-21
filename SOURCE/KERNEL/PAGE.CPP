/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : arch\i386\page.cpp
/* Last Update: 16.03.1999
/* Version    :
/* Coded by   : Szeleney Robert
/************************************************************************/
/* Definition:
/*   Intel 386 Page Table Module.
/************************************************************************/
#include "page.h"

#define NULL (void*)0

extern "C" void page_module(void);
extern unsigned int kernel_page_dir;

CArchPage::CArchPage(void)
{
   base = 0;
}

void CArchPage::SetBase(unsigned int newbase)
{
   base = newbase;
}

unsigned int CArchPage::GetBase(void)
{
   return base;
}

unsigned int CArchPage::Alloc(void)
{
   base = (unsigned int)palloc(1);
   memset(base, 0, 4096);

   present = 1;
   dirty = accessed = avail = 0;
   write = user = 1;

   return base;
}

unsigned int CArchPage::Format(void)
{
  unsigned int form;

  form = base;

  form |= present;
  form |= (write << 1);
  form |= (user  << 2);
  form |= (accessed << 5);
  form |= (dirty    << 6);

  return form;
}

CArchPageTable::CArchPageTable(void) : CArchPage()
{
}

void CArchPageTable::Insert(unsigned int nr, CArchPage *page)
{
   unsigned int *p;

//   show_msgf("inserting page starting at %x in dir %x",page->GetBase(), GetBase());
//   show_msgf("formated page attributes: %x",page->Format());

   p = GetBase();

   p += nr;

   *p = page->Format();
}

void page_module(void)
{
  CArchPageTable p;

  p.Alloc();
  show_msgf("Page Table allocated at %0x",p.GetBase());
  p.present = 1;
  p.dirty = 0;
  p.write = 1;
  p.user =  1;
  p.avail = 0;

  CArchPage page;

  page.SetBase(0x1000);
  page.present = 1;
  page.dirty = 0;
  page.write = 1;
  page.user = 1;
  page.avail = 0;

  p.Insert(0, &page);

  CArchPageTable *arr[10];
}






