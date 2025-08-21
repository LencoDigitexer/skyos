/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : arch\i386\desc.cpp
/* Last Update: 10.03.1999
/* Version    :
/* Coded by   : Szeleney Robert
/************************************************************************/
/* Definition:
/*   Intel 386 Descriptor Object.
/************************************************************************/
extern "C" void desc_module(void);


class CArchDescriptor
{
  private:
    unsigned int data[2];

  public:
    CArchDescriptor();

    virtual void Load(unsigned int) {}; 
};

class CArchDescriptorCode : public CArchDescriptor
{
  private:
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;

  public:
    CArchDescriptorCode();
    CArchDescriptorCode(unsigned int, unsigned int, unsigned int);
    void Load(unsigned int);
};

CArchDescriptor::CArchDescriptor()
{
  memset((unsigned char*)data, 0, 8);
}

CArchDescriptorCode::CArchDescriptorCode()  : CArchDescriptor()
{
}

CArchDescriptorCode::CArchDescriptorCode(unsigned int b, unsigned int a, unsigned int g)  : CArchDescriptor()
{
   base_low = b;
   access = a;
   granularity = g;
}

void CArchDescriptorCode::Load(unsigned int addr)
{
  show_msgf("loading code descriptor at address...");
}

void desc_module(void)
{
  CArchDescriptor *desc;

  CArchDescriptorCode code(1,1,1);

  desc = (CArchDescriptor*)&code;

  desc->Load(10);
}






