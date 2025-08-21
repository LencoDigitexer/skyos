/****************************************************************************/
/* Sky Operating System Version 2.0j (c) 10.11.1998 by Szeleney Robert      */
/* VESA Driver                                                              */
/*                                                                          */
/* Source File: c:\sky\source\kernel\vesa.c                                 */
/* Object File: c:\sky\object\vesa.o                                        */
/*                                                                          */
/* Last Update: 2. Dezember 1998                                            */
/* Coded by Gregory Hayrapetian                                             */
/*                                                                          */
/****************************************************************************/
/* Main functions:                                                          */
/*           - vesa_init: Init VESA                                         */
/*           - put_pixel: Puts a pixel                                      */
/*                                                                          */
/* All other functions are used by the memory manager, and should not       */
/* be called by an other task.                                              */
/****************************************************************************/

#define PACKED __attribute__((packed))

struct VbeInfoBlock
{
 char    VESASignature[4] PACKED;    // 0x81000
 short   VESAVersion PACKED;         // 0x81004
 char    *OEMStringPtr;       // 0x81006
 long    Capabilities PACKED;        // 0x8100a
 unsigned *VideoMdePtr;       // 0x8100e
 short   TotalMemory PACKED;         // 0x81012
/*
 //VBE 2.0
 short OemSoftwareRev;        // 0x81014
 char *OemVendorNamePtr;      // 0x81016
 char *OemProductNamePtr;     // 0x8101a
 char *OemProductRevPtr;      // 0x8101e
 char reserved[222];
 char OemData[256];
*/
};

struct PM_Int
{
 unsigned short SetWindow PACKED;
 unsigned short SetDisplayStart PACKED;
 unsigned short SetPalette PACKED;
 unsigned short IOPrivInfo PACKED;
};


void (*pm_bank)(char);
void (*pm_setstart)(char);
void (*pm_setpalette)(char);

struct VbeInfoBlock *vbe_info;
struct PM_Int *pm_info;

unsigned int active_bank = 0;

void SetBank(int bank)
{
 bank <<= 64;
 asm( "call *%0"
    :
    :"r" (pm_bank),"b" (0),"d"(bank)
    : "%eax","%ebx","%ecx","%edx","%esi","%edi");
}

void SetDisplayStart(unsigned int start)
{
 asm( "call *%0"
    :
    :"r" (pm_setstart),"b" (0),"c"((unsigned short)(start&0xfffff)), "d"((unsigned short)(start>>16))
    : "%eax","%ebx","%ecx","%edx","%esi","%edi");
}


void put_pixel(int x,int y,unsigned char color)
{
  int BYTESPERLINE = 1024;
  int addr = (int)y * BYTESPERLINE + x;
  char *ptr = (char*) (0xa0000 + (addr & 0xFFFF));

  if (active_bank != (int)(addr >> 16))
  {
    active_bank = (int)(addr >> 16);
    SetBank(active_bank);
  }
  *ptr = (char)color;

}


void vesa_init()
{
    unsigned short *ptr = (unsigned short*) 0x81004;
    unsigned short *mem = (unsigned short*) 0x81012;
    unsigned short *Rev = (unsigned short*) 0x81014;
    unsigned short *ModePtr = (unsigned short*) 0x8100e; // 0xc3dae
    unsigned int *str_ptr = (unsigned int*) 0x81006;
    char *str = (char*) (((unsigned int)((*str_ptr)&0xffff0000)>>12)|((unsigned int)(*str_ptr)&0xffff));
    char *VendorNamePtr,*ProductNamePtr,*ProductRevPtr;
    int i=0;
    unsigned int *ptr_pm = (unsigned int*) 0x80500;

    str_ptr = (unsigned int*) 0x81016;
    VendorNamePtr = (char*) (((unsigned int)((*str_ptr)&0xffff0000)>>12)|((unsigned int)(*str_ptr)&0xffff));

    str_ptr = (unsigned int*) 0x8101a;
    ProductNamePtr = (char*) (((unsigned int)((*str_ptr)&0xffff0000)>>12)|((unsigned int)(*str_ptr)&0xffff));

    str_ptr = (unsigned int*) 0x8100e;
    ModePtr = (char*) (((unsigned int)((*str_ptr)&0xffff0000)>>12)|((unsigned int)(*str_ptr)&0xffff));

    str_ptr = (unsigned int*) 0x80500;
    ptr_pm = (char*) (((unsigned int)((*str_ptr)&0xffff0000)>>12)|((unsigned int)(*str_ptr)&0xffff));

    str_ptr = (unsigned int*)
    printk("main.c: Vesa - VBE Version: %d.%d",(*ptr)>>8,(*ptr) & 0xff);
    printk("main.c: Vesa - Memory: %d KB",(*mem)*64);
    printk(" ");
    printk("main.c: Vesa - Rev.: %d",*Rev);
    printk("main.c: Vesa - Manufacturer: %s",str);
    printk("main.c: Vesa - Vendor Name: %s",VendorNamePtr);
    printk("main.c: Vesa - Product Name: %s",ProductNamePtr);
    printk("main.c: Vesa - Product Rev: %s",ProductRevPtr);
    printk(" ");

    while (*ModePtr != 0xffff)
    {
//      if ((++i % 10)==0) getch();
      printk("main.c: Vesa - Mode: 0x%3x",*ModePtr);
      ModePtr++;
    }

    // Protected Mode Interface
    pm_info = (struct PM_Int*) ptr_pm; //0xc7da0; // 0xc46b4

    pm_bank     = (void*)((char*)pm_info+pm_info->SetWindow);
    pm_setstart = (void*)((char*)pm_info+pm_info->SetDisplayStart);
    pm_setpalette=(void*)((char*)pm_info+pm_info->SetPalette);
/*
    pm_bank     = (void*)((char*)pm_info+pm_info->SetWindow);
    pm_setstart = (void*)((char*)pm_info+pm_info->SetDisplayStart);
*/
}
