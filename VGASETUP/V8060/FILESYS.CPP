#include <dos.h>
#include <io.h>
#include <conio.h>
#include <math.h>
#include <string.h>
#include <complex.h>
#include <graphics.h>
#include "v8060\v8060.h"

struct Files
{
   char name[50];
   char phys[50];
   int prot;
} LF = {"\0"};


struct Vector
{
  int x, y, z;
};

float alpha = 45;
float beta = -105;


Vector Tower_h[5] = {-80,-80,0,80,-80,0,80,80,0,-80,80,0,-80,-80,0};
Vector Tower_v[5] = {80,80,0,-80,80,0,-80,80,-394,80,80,-394,80,80,0};
Vector Tower_d[5] = {80,80,0,80,-80,0,80,-80,-394,80,80,-394,80,80,0};
Vector Selector = {0,0,0};


void DrawTower(int color, int mx, int my, float size)
{
  Vector td[5];
  Vector td2[5];

  float sa, sb, ca, cb;
  int l;

  setcolor(color);

  sa = sin(alpha/180*M_PI);
  sb = sin(beta/180*M_PI);
  ca = cos(alpha/180*M_PI);
  cb = cos(beta/180*M_PI);

  setcolor(4);
  setfillstyle(1,4);
  pieslice( (int)(((Selector.x)*ca - (Selector.y) *sa)*size) + mx,
	   (int)((((Selector.x)*sa +(Selector.y) *ca)*cb + Selector.z*sb)*size) + my,
	   0,360,5);
  setcolor(color);

  for (l=0;l<=4;l++)
  {

    td[l].x = (int)((Tower_h[l].x*ca -Tower_h[l].y *sa)*size) + mx;
    td[l].y = (int)(((Tower_h[l].x*sa + Tower_h[l].y *ca)*cb + Tower_h[l].z*sb)*size) + my;
  }

  for (int i=0;i<20;i++)
  {
    moveto(td[0].x,td[0].y+20*size*i);
    for (l=1;l<=4;l++)
      lineto(td[l].x,td[l].y+20*size*i);
  }
  setcolor(4);
  setfillstyle(1,4);
  pieslice( (int)(((Selector.x)*ca - (Selector.y) *sa)*size) + mx,
	   (int)((((Selector.x)*sa +(Selector.y) *ca)*cb + Selector.z*sb)*size) + my,
	   0,360,5);
  setcolor(color);

  for (i=0;i<9;i++)
  {
  for (l=0;l<=4;l++)
  {
    td2[l].x = (int)((Tower_v[l].x*ca -(Tower_v[l].y-20*i) *sa)*size) + mx;
    td2[l].y = (int)(((Tower_v[l].x*sa + (Tower_v[l].y-20*i) *ca)*cb + Tower_v[l].z*sb)*size) + my;
  }
  moveto(td2[0].x,td2[0].y);
  for (int j=1;j<=4;j++)
      lineto(td2[j].x,td2[j].y);
  }

  setcolor(4);
  setfillstyle(1,4);
  pieslice( (int)(((Selector.x)*ca - (Selector.y) *sa)*size) + mx,
	   (int)((((Selector.x)*sa +(Selector.y) *ca)*cb + Selector.z*sb)*size) + my,
	   0,360,5);
  setcolor(color);

  for (i=0;i<9;i++)
  {
  for (l=0;l<=4;l++)
  {
    td2[l].x = (int)(((Tower_d[l].x-20*i)*ca - Tower_d[l].y *sa)*size) + mx;
    td2[l].y = (int)((((Tower_d[l].x-20*i)*sa +Tower_d[l].y *ca)*cb + Tower_d[l].z*sb)*size) + my;
  }
  moveto(td2[0].x,td2[0].y);
  for (int j=1;j<=4;j++)
  {
      lineto(td2[j].x,td2[j].y);
  }
  }

  setcolor(4);
  setfillstyle(1,4);

  setcolor(4);
  setfillstyle(1,4);
  pieslice( (int)(((Selector.x)*ca - (Selector.y) *sa)*size) + mx,
	   (int)((((Selector.x)*sa +(Selector.y) *ca)*cb + Selector.z*sb)*size) + my,
	   0,360,5);
  setcolor(color);
}


void FileSys(FILE *f)
{
  int color = 2;
  int open = 1;
  int sc = 0;
  char ch = 0;
  char str[255];
  float size = 0.10;
  int level = 1;
  int sector = 1;
  int x = 1;
  int y = 1;
  int sm = 0;

  char Info[255];
  sprintf(Info,"Please select File in Save Tower %d",level);
  sprintf(str,"c:\\vms\\source\\kernel\\system\\filesys\\level%003d",level);


  if ( ((f = fopen(str,"rb")) == NULL) || (filelength(fileno(f)) == 0))
  {
     fclose(f);
     f = fopen(str,"wb");

     strcpy(LF.name,"Datei wird verwendet!");

     fwrite(&LF,sizeof(LF),1,f);
     strcpy(LF.name,"\0");
     for (int i=0;i<68;i++)
       fwrite(&LF,sizeof(LF) ,1,f);
     strcpy(LF.name,"Datei wird verwendet!");
     fwrite(&LF,sizeof(LF),1,f);
     strcpy(LF.name,"\0");
     for (i=0;i<64*20-70;i++)
       fwrite(&LF,sizeof(LF) ,1,f);


     fclose(f);
     f = fopen(str,"rb");
  }

  while (ch!=27)
  {
    if (sm)
    {
    }
    DrawTower(color,200,130,size);

//  DrawTower(0,200,130,size);
    box(0,50,370,560,0,0,0);

    alpha+=1;

    if (open==1)
    {
      size += 0.01;
      if (size>0.95) open = 2;
    }
    if (open==2)
    {
      if (sc>strlen(Info)) open = 3;
      else
      {
	sprintf(str,"%c",Info[sc]);
	outs(380+sc*12,70,4,str,0,2,6);
	sc++;
      }
    }

    if (kbhit())
    {
      ch = getch();
      if (ch=='+') size+=0.25;
      if (ch=='-') size-=0.25;
      if (ch=='s') open = 2;
	if (ch=='c') color++;
	if (ch=='d') color--;
     if (ch==0)
      {
	ch = getch();
	if (ch==77)
	  Selector.x += 20;
	if (ch==75)
	  Selector.x -= 20;
	if (ch==72)
	  Selector.y += 20;
	if (ch==80)
	  Selector.y -= 20;
	if (ch==73)
	  Selector.z += 21;
	if (ch==81)
	  Selector.z -= 21;

	box(380,120,500,240,0,0,0);
	sprintf(str,"Level : #15%003d",level);
	outs(380,120,14,str,0,2,4);
	sprintf(str,"Sector: #15%003d",Selector.z / 20);
	outs(380,140,14,str,0,2,4);
	sprintf(str,"X     : #15%003d",Selector.x / 20);
	outs(380,160,14,str,0,2,4);
	sprintf(str,"Y     : #15%003d",Selector.y / 20);
	outs(380,180,14,str,0,2,4);

	fseek(f,0,SEEK_SET);
	int count;
	count = (Selector.z/-20 * 64) + (4 + Selector.y/20) * 8 + Selector.x/20 + 4;

	sprintf(str,"Offset: #15%003d",count);
	outs(380,200,14,str,0,2,4);

	for (int i=0;i<=count;i++)
	  fread(&LF,sizeof(LF),1,f);

	sprintf(str,"File  : #15%s",LF.name);
	outs(380,220,14,str,0,2,4);
      }
      if (ch==13)
      {
	box(380,250,800,300,0,0,0);
	outs(380,250,15,"Filename: ",0,2,4);
	Get_String(str,15,400,260,70);
	strcpy(LF.name,str);

	fseek(f,0,SEEK_SET);
	int count;
	count = (Selector.z/-20 * 64) + (4 + Selector.y/20) * 8 + Selector.x/20 + 4;
    }
    }
  }
  fclose(f);

}

