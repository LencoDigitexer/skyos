/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : arch\i386\idt.cpp
/* Last Update: 10.03.1999
/* Version    : beta
/* Coded by   : Szeleney Robert
/************************************************************************/
/* Definition:
/*   Intel 386 IDT Object.
/*   This Object administrates Descriptors for an IDT Table.
/************************************************************************/
extern "C" void idt_module(void);

class CArchIDT
{
  private:
    unsigned int base;
    unsigned int limit;

  public:
    CArchIDT(void);

    void Load(void);
    void Store(void);
    void SetBase(unsigned int);
    void SetLimit(unsigned int);
    unsigned int GetBase(void);
    unsigned int GetLimit(void);
};

/************************************************************************/
/* Initialize a NULL-IDT
/************************************************************************/
CArchIDT::CArchIDT(void)
{
   base = limit = 0;
}

/************************************************************************/
/* Load the class-member variables with current idt-register.
/************************************************************************/
void CArchIDT::Load(void)
{
  unsigned int data[2];

  data[0] = limit << 16;
  data[1] = base;
  __asm__ __volatile__("sidt (%0)" \
                         : \
                         : "p" (((char*)data)+2));
}

/************************************************************************/
/* Set the system idt-register with values stored in member-variables.
/************************************************************************/
void CArchIDT::Store(void)
{
  unsigned int data[2];

  __asm__ __volatile__("sidt (%0)" \
                       : \
                       : "p" (((char*)data)+2));

  limit = data[0] >> 16;
  base = data[1];

}

/************************************************************************/
/* Set Base of the IDT
/************************************************************************/
void CArchIDT::SetBase(unsigned int newbase)
{
   base = newbase;
}

/************************************************************************/
/* Set Limit of the IDT
/************************************************************************/
void CArchIDT::SetLimit(unsigned int newlimit)
{
   limit = newlimit;
}

/************************************************************************/
/* Get Base of the IDT
/************************************************************************/
unsigned int CArchIDT::GetBase(void)
{
  return base;
}

/************************************************************************/
/* Get Limit of the IDT
/************************************************************************/
unsigned int CArchIDT::GetLimit(void)
{
  return limit;
}

void idt_module(void)
{
  CArchIDT CGlobal_Idt;

  CGlobal_Idt.Store();
  show_msgf("Base: %x   Limit: %x",CGlobal_Idt.GetBase(),CGlobal_Idt.GetLimit());
  CGlobal_Idt.Load();

  CGlobal_Idt.SetLimit(0x7ff);
  CGlobal_Idt.SetBase(0xC0010DB0);
  CGlobal_Idt.Load();
}


