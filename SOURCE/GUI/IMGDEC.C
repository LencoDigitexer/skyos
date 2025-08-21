/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : gui\imgdec.c
/* Last Update: 19.03.1999
/* Version    : alpha
/* Coded by   : Hayrapetian Gregory
/* Docus      :
/************************************************************************/
/* Definition:
/*   The image decoder
/************************************************************************/

#include "imgdec.h"
#include "newgui.h"

void load_coltab(TGAColorTable *coltab)
{
  int i;

  outportb(0x3c8,0);

  for (i=0;i<256;i++)
  {
//    if ((coltab->color[i].red == 0) || (coltab->color[i].green == 0) || (coltab->color[i].blue == 0))
//      show_msgf("red: %d  green: %d  blue: %d",coltab->color[i].red,coltab->color[i].green,coltab->color[i].blue);

    outportb(0x3c9, coltab->color[i].red >> 2);
    outportb(0x3c9, coltab->color[i].green >> 2);
    outportb(0x3c9, coltab->color[i].blue >> 2);
  }

}


int load_TGA(window_t *win, int fd, TGAFile *tga_file)
{
  int i,j;
  unsigned char coltab[768];

  sys_read(fd,&tga_file->header,18);
///  sys_read(fd,coltab,2000);
  sys_read(fd,coltab,768);
//  tga_file->color_table

  if (tga_file->source)
    vfree(tga_file->source);

  tga_file->source = (unsigned char*) valloc(tga_file->header.Height*tga_file->header.Width+50000);

  show_msgf("BytesInIdentField: %d",tga_file->header.BytesInIdentField);
  show_msgf("ColorMapType: %d",tga_file->header.ColorMapType);
  show_msgf("ImageTypeCode: %d",tga_file->header.ImageTypeCode);
  show_msgf("ColorMapOrigin: %d",tga_file->header.ColorMapOrigin);
  show_msgf("ColorMapLength: %d",tga_file->header.ColorMapLength);
  show_msgf("ColorMapEntrySize: %d",tga_file->header.ColorMapEntrySize);
  show_msgf("XOrigin: %d",tga_file->header.XOrigin);
  show_msgf("YOrigin: %d",tga_file->header.YOrigin);
  show_msgf("Width: %d",tga_file->header.Width);
  show_msgf("Height: %d",tga_file->header.Height);
  show_msgf("ImagePixelSize: %d",tga_file->header.ImagePixelSize);
  show_msgf("ImageDescByte: %d",tga_file->header.ImageDescByte);


  for (i=0;i<256;i++)
  {
    tga_file->color_table.color[i].blue = coltab[i*3];
    tga_file->color_table.color[i].green = coltab[i*3+1];
    tga_file->color_table.color[i].red = coltab[i*3+2];
  }

  if (tga_file->header.ImageTypeCode == 1)
  {
    if (tga_file->header.ImagePixelSize == 8)
    {
///      sys_seek(fd, 786, 0);
      sys_read(fd,tga_file->source,tga_file->header.Width*tga_file->header.Height /* +50000 */);
      return 0;
    }
  }

  return 1;
}


