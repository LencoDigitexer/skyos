#include "v8060\v8060.h"
#include "v8060\3dgraf.h"
#include <graphics.h>
#include <conio.h>
#include <math.h>

void THREED_OBJECT::Aktuell(int num)
{
  if (num == 1)
  {
    for (int i=0;i<=10;i++)
    {
    POINTS[(4*i)].x = -10;
    POINTS[(4*i)+0].y =  10;
    POINTS[(4*i)+0].z =  i*10;
    POINTS[(4*i)+1].x =  10;
    POINTS[(4*i)+1].y =  10;
    POINTS[(4*i)+1].z =  i*10;
    POINTS[(4*i)+2].x =  10;
    POINTS[(4*i)+2].y = -10;
    POINTS[(4*i)+2].z =  i*10;
    POINTS[(4*i)+3].x = -10;
    POINTS[(4*i)+3].y = -10;
    POINTS[(4*i)+3].z = i*10;
    }

    i = 0;

    folge[0] = 1;
    folge[1] = 33;
    folge[2] = 36;
    folge[3] = 4;
    folge[4] = 1;

    folge[5] = 2;
    folge[6] = 34;
    folge[7] = 33;
    folge[8] = 1;

    folge[9] = 4;
    folge[10] = 36;
    folge[11] = 35;
    folge[12] = 3;
    folge[13] = 4;

    folge[14] = 3;
    folge[15] = 35;
    folge[16] = 34;
    folge[17] = 2;
    folge[18] = 3;

    folgemax = 18;

    maxpoints = 10;
  }
}





THREED_OBJECT::THREED_OBJECT(void)
{
  int i;

  for (i=0;i<100;i++)
    POINTS[i].x = POINTS[i].y = POINTS[i].z = 0;

  for (i=0;i<50;i++)
    folge[i] = 0;

  maxpoints = 0;
  RELX = 400;
  RELY = 300;

  size = 1;
  alpha = 45;
  beta  = -105;
}

THREED_OBJECT::THREED_OBJECT(char *filename)
{
  int i;

  for (i=0;i<100;i++)
    POINTS[i].x = POINTS[i].y = POINTS[i].z = 0;

  for (i=0;i<50;i++)
    folge[i] = 0;

  maxpoints = 0;
  RELX = 400;
  RELY = 300;
  size = 1;
  alpha = 45;
  beta  = -105;

  FILE *f;

  f = fopen(filename,"rb");

  int dummy[170];

  fread(&POINTS, sizeof POINTS,1,f);
  fread(&dummy, 200-sizeof POINTS,1,f);
  fread(&folge , sizeof(folge) ,1,f);
  fread(&maxpoints, sizeof(maxpoints), 1,f);

  fclose(f);
}

void New_Object(void)
{
  int maxpoints;
  int beta, alpha, size;
  struct THREED_POINT POINT[10];
  int folge[50];

  clrscr();
  printf("Maxpoints: ");
  scanf("%d",&maxpoints);
  printf("\n");

  for (int i=0;i<maxpoints;i++)
  {
    printf("Point %d X: ",i+1);
    scanf("%d",&POINT[i].x);
    printf("\n");
    printf("Point %d Y: ",i+1);
    scanf("%d",&POINT[i].y);
    printf("\n");
    printf("Point %d Z: ",i+1);
    scanf("%d",&POINT[i].z);
    printf("\n");
  }

  int ende = 0;
  char str[255];
  int key = 0;

  while (!ende)
  {
    printf("Folge Key %02d : ",key+1);
    int wert;
    scanf("%d",&wert);
    printf("\n");
    if (wert!=0)
    {
      folge[key] = wert;
      key++;
    }
    else ende =1;
  }

  FILE *f;

  printf("\n\nFilename:");
  scanf("%s",str);

  f = fopen(str,"wb");

  fwrite(&POINT, 200,1,f);
  fwrite(&folge , sizeof(folge) ,1,f);
  fwrite(&maxpoints, sizeof(maxpoints), 1,f);

  fclose(f);
}

void THREED_OBJECT::Draw(int color)
{
  struct THREED_POINT dummy[100];

  double SinA, SinB, CosA, CosB;
  int   lauf;

  SinA = sin(double(alpha/180*3.1415));
  SinB = sin(double(beta/180*3.1415));
  CosA = cos(double(alpha/180*3.1415));
  CosB = cos(double(beta/180*3.1415));

  for (lauf = 0; lauf <= maxpoints; lauf ++)
  {
    dummy[lauf].x = (int)((POINTS[lauf].x * CosA - POINTS[lauf].y * SinA) * size + RELX);
    dummy[lauf].y = (int)(((POINTS[lauf].x * SinA - POINTS[lauf].y * CosA) * CosB + POINTS[lauf].z * SinB) * size + RELY);
    lauf = lauf;
  }

  setcolor(color);
  moveto(dummy[folge[0]].x, dummy[folge[0]].y);
  for (lauf = 1; lauf <= folgemax;lauf++)
  {
    lineto(dummy[folge[lauf]].x, dummy[folge[lauf]].y);
  }

}

void THREED_OBJECT::Rot(int al, int be)
{
  alpha = al;
  beta = be;
}