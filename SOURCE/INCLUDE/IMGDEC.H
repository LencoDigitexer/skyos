/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\imgview.h
/* Last Update: 19.03.1999
/* Version    : alpha
/* Coded by   : Gregory Hayrapetian
/* Docus      :
/************************************************************************/


#define PACKED __attribute__((packed))


typedef struct sTGAHeader
{
  unsigned char  BytesInIdentField  PACKED;
  unsigned char  ColorMapType       PACKED;
  unsigned char  ImageTypeCode      PACKED;
  unsigned short ColorMapOrigin     PACKED;
  unsigned short ColorMapLength     PACKED;
  unsigned char  ColorMapEntrySize  PACKED;
  unsigned short XOrigin            PACKED;
  unsigned short YOrigin            PACKED;
  unsigned short Width              PACKED;
  unsigned short Height             PACKED;
  unsigned char  ImagePixelSize     PACKED;
  unsigned char  ImageDescByte      PACKED;
} TGAHeader;

typedef struct sRGBColor
{
  unsigned char blue, green, red;
} RGBColor;

typedef struct sTGAColorTable
{
  RGBColor color[256];
} TGAColorTable;

typedef struct sTGAFile
{
  TGAHeader header;
  TGAColorTable color_table;
  unsigned char *source;
} TGAFile;
