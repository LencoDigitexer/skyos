#include "v8060\v8060.h"
#include "mouse\mouse.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <graphics.h>
#include <dos.h>
#include <dir.h>
#include <io.h>

int screen_y = 10;
int FILLSTYLE = 1;
extern int mask;

struct Display
{
  int BACK_BOX_STYLE;
  int BACK_BOX_COLOR;
  int TITEL_COLOR;
  int TITEL_DECOLOR;
  int TITEL_STYLE;
  int TITELDRAW_COLOR;
  int TITELDRAW_DECOLOR;
} Displayset = {1,7,1,7,1,15,0};

char gc(void)
{
  while (!kbhit())
  {
//    while (!ipxRecvChk()) recvPacket();
  }

  return(getch());
}

void lin(int x1, int y1, int x2, int y2, int col)
{
  setcolor(col);
  line(x1,y1,x2,y2);
}

void linp(int x1, int y1, int x2, int y2, int col1, int col2, int col3=32000)
{
  if (col3==32000)
    col3 = col2;

  setcolor(col1);
  line(x1,y1,x2,y2);
  putpixel(x1,y1,col2);
  putpixel(x2,y2,col3);
}

void box2(int x1, int y1, int x2, int y2, int c1, int c2, int c3)
{
  setfillstyle(1,c2);
  bar(x1+1,y1+1,x2-1,y2-1);
  setcolor(c1);
  line(x1,y1,x1,y2);
  line(x1-1,y1-1,x1-1,y2+1);
  line(x1-2,y1-1,x1-2,y2+2);
  line(x1,y1,x2,y1);
  line(x1-1,y1-1,x2+1,y1-1);
  line(x1-2,y1-1,x2+2,y1-1);
  setcolor(c3);
  line(x2,y2,x2,y1);
  line(x2+1,y2+1,x2+1,y1-1);
  line(x2+2,y2+2,x2+2,y1-1);
  line(x2,y2,x1,y2);
  line(x2+1,y2+1,x1-1,y2+1);
  line(x2+2,y2+2,x1-2,y2+2);
}


void box2(int x1, int y1, char *titel, int c1, int c2, int c3, int c4)
{
  int x2 = x1 + strlen(titel) * 8 + 20;
  int y2 = y1 + 20;

  setfillstyle(1,c2);
  bar(x1+1,y1+1,x2-1,y2-1);
  setcolor(c1);
  line(x1,y1,x1,y2);
  line(x1-1,y1-1,x1-1,y2+1);
  line(x1-2,y1-1,x1-2,y2+2);
  line(x1,y1,x2,y1);
  line(x1-1,y1-1,x2+1,y1-1);
  line(x1-2,y1-1,x2+2,y1-1);
  setcolor(c3);
  line(x2,y2,x2,y1);
  line(x2+1,y2+1,x2+1,y1-1);
  line(x2+2,y2+2,x2+2,y1-1);
  line(x2,y2,x1,y2);
  line(x2+1,y2+1,x1-1,y2+1);
  line(x2+2,y2+2,x1-2,y2+2);
  setcolor(c4);
  outtextxy(x1+10,y1+5,titel);
}

void box(int x1, int y1, int x2, int y2, int f1, int f2, int f3)
{
  setfillstyle(FILLSTYLE,f2);
  bar(x1+1,y1+1,x2-1,y2-1);
  lin(x1,y1,x1,y2,f1);
  lin(x1,y1,x2,y1,f1);
  lin(x2,y2,x2,y1,f3);
  lin(x2,y2,x1,y2,f3);
}

void boxa(int x1, int y1, int x2, int y2, int f1, int f2, int f3, int st)
{
  setfillstyle(st,f2);
  bar(x1+1,y1+1,x2-1,y2-1);
  lin(x1,y1,x1,y2,f1);
  lin(x1,y1,x2,y1,f1);
  lin(x2,y2,x2,y1,f3);
  lin(x2,y2,x1,y2,f3);
}

void abox1(int x1, int y1, int x2, int y2, int c1, int c2, int c3, int del)
{
  int mid_x = (x1 + x2) / 2;
  int mid_y = (y1 + y2) / 2;

  int i;
  for (i=x1;i<=x2;i++)
  {
    linp(mid_x, mid_y, i, y1, c2, c1);
    delay(del);
  }
  for (i=y1;i<=y2;i++)
  {
    linp(mid_x, mid_y, x2, i, c2, c3);
    delay(del);
  }
  for (i=x2;i>=x1;i--)
  {
    linp(mid_x, mid_y, i, y2, c2, c3);
    delay(del);
  }
  for (i=y2;i>=y1;i--)
  {
    linp(mid_x, mid_y, x1, i, c2, c1);
    delay(del);
  }
  putpixel(mid_x,mid_y,c2);
}

void boxopen(int x1, int y1, int x2, int y2, int f1, int f2, int f3, int del=0)
{
  int mittelx = (x2-x1)/2;
  int mittely = (y2-y1)/2;

  int drawy = 0;

  if ((y2-y1) > (x2-x1))
  {
    drawy = mittely / mittelx;
  }
  else
  {
    drawy = mittelx / mittely;
  }

  int j=0;
  int k=0;

  for (int i=0;i<mittelx;i++)
  {
    j++;
    if (j==drawy)
    {
      j=0;
      k++;
    }
    box(x1+mittelx - i, y1+mittely - k, x1+mittelx + i, y1+mittely +k,f1,f2,f3);
    if (del) delay(del);
  }
  box(x1,y1,x2,y2,f1,f2,f3);


}
void Rect(int x1, int y1, int x2, int y2, int f1)
{
  setcolor(f1);
  line(x1,y1,x1,y2);
  line(x1,y1,x2,y1);
  line(x2,y2,x2,y1);
  line(x2,y2,x1,y2);
}

void Get_String(char string[],int color, int x, int y, int length)
{
 int count=0;
 int ende=0;
 char str[200]={'\0'};
 int i=0;
 char ch;
 int col;

 col=getpixel(x+(i*8),y);

 while (!ende)
 {
   while (!kbhit())
   {
     count++;
     if (count==1000)
     {
       setcolor(color);
       col=getpixel(x+(i*8),y);
       line(x+(i*8+1),y,x+i*8+1,y+7);
     }
     if (count==2000)
     {

       setcolor(col);
       line(x+(i*8+1),y,x+i*8+1,y+7);
       count=0;
     }
//    while (!ipxRecvChk()) recvPacket();

   }
   count=0;
   ch=gc();
   setcolor(col);
   line(x+(i*8+1),y,x+i*8+1,y+7);
   if (ch==0)
   {
     ch =gc();
     if (ch==68)
     {
       str[0]='\0';
       box(x,y,x+(i*8),y+7,78,78,78);
       i=0;
     }
     if (ch==75)
     {
       if (i>0)
       {
	 i--;
       }
     }
     if (ch==77)
     {
       if (i<strlen(str))
       {
	 i++;
       }
     }
   }
   else if (ch==13)
   {
     ende=1;
   }
   else if (ch==8)
   {
     if (i>0)
     {
       str[i-1]=' ';
       box(x+(i*8-8),y,x+(i*8),y+7,col,col,col);
       i--;
     }
   }
   else if ((i<length) && (ch>31))
   {
     char st[2];
     st[1]='\0';
     st[0]=ch;
     box(x+(i*8),y,x+(i*8+8),y+7,col,col,col);
     setcolor(color);
     outtextxy(x+(i*8),y,st);
     str[i] = ch;
     i++;
   }
}
   strcpy(string,str);
}

int Get_Stringad(char string[],int color, int x, int y, int length, int back)
{
 int count=0;
 int ende=0;
 char str[200]={'\0'};
 int i=strlen(string);
 char ch;
 int col;
 int returnwert = 0;

 strcpy(str,string);
 setcolor(color);
 outtextxy(x,y,str);

 col=back;

 while (!ende)
 {
   while (!kbhit())
   {
     count++;
     if (count==1000)
     {
       setcolor(color);
       col=back;
       line(x+(i*8+1),y,x+i*8+1,y+7);
     }
     if (count==2000)
     {

       setcolor(col);
       line(x+(i*8+1),y,x+i*8+1,y+7);
       count=0;
     }
//    while (!ipxRecvChk()) recvPacket();
   }
   count=0;
   ch=gc();
   setcolor(col);
   line(x+(i*8+1),y,x+i*8+1,y+7);
   if (ch==0)
   {
     ch =gc();
     if (ch==68)
     {
       str[0]='\0';
       box(x,y,x+(i*8),y+7,78,78,78);
       i=0;
     }
     if (ch==75)
     {
       if (i>0)
       {
	 i--;
       }
     }
     if (ch==77)
     {
       if (i<strlen(str))
       {
	 i++;
       }
     }
   }
   else if (ch==13)
   {
     ende=1;
   }
   else if (ch==8)
   {
     if (i>0)
     {
       str[i-1]='\0';
       box(x+(i*8-8),y,x+(i*8),y+7,col,col,col);
       i--;
     }
     else
     {
       ende = 1;
       returnwert = 255;
     }
   }
   else if ((i<length) && (ch>31))
   {
     char st[2];
     st[1]='\0';
     st[0]=ch;
     box(x+(i*8),y,x+(i*8+8),y+7,col,col,col);
     setcolor(color);
     outtextxy(x+(i*8),y,st);
     str[i] = ch;
     i++;
   }
   else if ((i==length) && (ch>31))
   {
     returnwert = ch;
     ende = 1;
   }

}
   strcpy(string,str);
   return returnwert;
}

void Get_Stringp(char string[],int color, int x, int y, int length)
{
 int count=0;
 int ende=0;
 char str[50]={'\0'};
 int i=0;
 char ch;
 int col;

 col=getpixel(x+(i*8),y);

 while (!ende)
 {
   while (!kbhit())
   {
     count++;
     if (count==1000)
     {
       setcolor(color);
       col=getpixel(x+(i*8),y);
       line(x+(i*8+1),y,x+i*8+1,y+7);
     }
     if (count==2000)
     {
       setcolor(col);
       line(x+(i*8+1),y,x+i*8+1,y+7);
       count=0;
     }
//    while (!ipxRecvChk()) recvPacket();
   }
   count=0;
   ch=gc();
   setcolor(col);
   line(x+(i*8+1),y,x+i*8+1,y+7);
   if (ch==13)
   {
     ende=1;
     str[i] = '\0';
   }
   else if (ch==8)
   {
     if (i>0)
     {
       str[i-1]='\0';
       box(x+(i*8-8),y,x+(i*8),y+7,78,78,78);
       i--;
     }
   }
   else if ((i<length) && (ch>31))
   {
     char st[2];
     st[1]='\0';
     st[0]='*';
     setcolor(color);
     outtextxy(x+(i*8),y,st);
     str[i] = ch;
     i++;
   }
}
   strcpy(string,str);
}

void Retrace(void)
{
  asm mov dx,0xda

wait1:
  asm in al,dx
  asm test al,0x08
  asm jnz wait1

wait2:
  asm in al,dx
  asm test al,0x08
  asm jz wait2
}


void SetPal(unsigned char *palette)
{
  asm {
  push si
  mov si,offset palette
  mov cx,256*3
  xor al,al
  mov dx,0x03c8
  out dx,al
  inc dx

  rep outsb
  pop si
}
}

void outs(int x, int y, int col, char *string, int speed, int style, int charsize)
{
  char str[2];
  int c = 0;

  struct textsettingstype textinfo;
  gettextsettings(&textinfo);

  int st = textinfo.font;
  int size = textinfo.charsize;
  if (style!=255) st = style;
  if (charsize!=255) size = charsize;

  settextstyle(st,textinfo.direction,size);

  str[1] = '\0';
  setcolor(col);

  for (int i=0;i<=strlen(string);i++)
  {
    str[0] = string[i];
    if (str[0]=='#')
    {
     char str2[3];
     str2[2] = '\0';
     str2[0] = string[i+1];
     str2[1] = string[i+2];
      i++;
      i++;
      setcolor(atoi(str2));

    }
    else
    {
      if (str[0] >31)
      {
       outtextxy(x+(c*8),y,str);
       c++;
       delay(speed);
      }
    }
  }
  settextstyle(textinfo.font,textinfo.direction,textinfo.charsize);
}

void outsl(int x, int y, int col, char *string, int speed)
{
  char str[2];

  str[1] = '\0';
  setcolor(col);

  x = x - (strlen(string)*8);
  for (int i=0;i<=strlen(string);i++)
  {
    str[0] = string[i];
    outtextxy(x+(i*8),y,str);
    delay(speed);
  }
}

void GetPal(unsigned char *palette)
{
  asm {
  push di
  push ds
  pop es
  mov di,offset palette
  mov cx,768
  xor al,al
  mov dx,0x03c7
  out dx,al
  mov dx,0x3c9

  rep insb
  pop di
}
}

void Down_Pal(unsigned char palette[],int value)
{
  for (int i=0;i<768;i++)
  {
   if ((palette[i]-value)>=0)
     palette[i]-=value;
   else palette[i]=0;
  }
  SetPal(palette);
}

void out(int x, int y, int col, char*string)
{
  setcolor(col);
  outtextxy(x,y,string);
}

void t_out(char*string)
{
  setcolor(15);
  outtextxy(10,screen_y,string);
  screen_y+=12;
  if (screen_y>460)
   screen_y= 10;
}

void outc(int x,int x2, int y, int col, char*string)
{
  setcolor(col);
  int xrel;

  xrel = (x + x2) /2;  // Mittelposition
  xrel = xrel - ( (strlen(string)*8) / 2);  // Minus Halbe Stringl„nge

  outtextxy(xrel,y,string);
}

void outsc(int x,int x2, int y, int col, char*string, int font, int charsize)
{
  setcolor(col);
  int xrel;
  char str[2];


  xrel = (x + x2) /2;  // Mittelposition
  xrel = xrel - ( (textwidth(string)) / 2);  // Minus Halbe Stringl„nge

  outs(xrel,y,col,string,0,font,charsize);
}

void Pic(char *str, int x, int y)
{
  FILE *f;
  int i=0;
  int j=0;
  int k=0;

  unsigned char pic[250] = {"\0"};

  f=fopen(str,"rb");

  fread(&pic[i],225,1,f);
  fclose(f);

  for (i=0;i<=14;i++)
    for (j=0;j<=14;j++)
    {
      if (pic[k]!=0) putpixel(x+i,y+j,pic[k]);
      k++;
    }
 }


void Picg(char *str, int x, int y)
{
  FILE *f;
  int i=0;
  int j=0;
  int k=0;

  unsigned char pic[250] = {"\0"};

  f=fopen(str,"rb");

  int pix,piy;

  pix = fgetc(f);
  piy = fgetc(f);


  for (i=0;i<pix;i++)
    for (j=0;j<piy;j++)
    {
      char ch = fgetc(f);
      if (ch!=0) putpixel(x+i,y+j,ch);
    }


  fclose(f);
 }

void Get_Pass_1(char *pass)
{
  char str[255];
  char strpass[255];

  int right = 0;

  while (!right)
  {

  cleardevice();

  box(50,360,590,400,77,78,79);
  box(60,380,580,398,0,16,15);
  outsc(50,590,365,15,"Please enter password");
  Get_Stringp(strpass,15,70,384,11);

  for (int i=0;i<strlen(strpass);i++)
    str[i] = '*';
  str[i+1] = '\0';

  outsc(1,800,100,15,str);

  outs(30,50,4,"Matching password...",5);
  outs(30,60,4,"Using CNF 5TR Method",5);
  outs(30,75,4,"Key: 7F660ABC0012041",5);
  for (i=0;i<=strlen(strpass)*8+11;i++)
  {
    lin(320,5,310-(strlen(strpass)*8/2)+i,90,4);

    for (int j=0;j<=20;j++)
      if (getpixel(310-(strlen(strpass)*8/2)+i,90+j) == 15)
	putpixel(310-(strlen(strpass)*8/2)+i,90+j,4);

    lin(310-(strlen(strpass)*8/2)+i,90,310-(strlen(strpass)*8/2)+i,100,4);

    delay(10);

    lin(320,5,310-(strlen(strpass)*8/2)+i,90,0);
    for (j=0;j<=20;j++)
      if (getpixel(310-(strlen(strpass)*8/2)+i,90+j) == 4)
	putpixel(310-(strlen(strpass)*8/2)+i,90+j,15);

    lin(310-(strlen(strpass)*8/2)+i,90,310-(strlen(strpass)*8/2)+i,100,0);

    //Zweiter Strahl von unten

    lin(320,350,330+(strlen(strpass)*8/2)-i,110,4);

    for ( j=0;j<=20;j++)
      if (getpixel(330+(strlen(strpass)*8/2)-i,110-j) == 15)
	putpixel(330+(strlen(strpass)*8/2)-i,110-j,4);

    lin(330+(strlen(strpass)*8/2)-i,110,330+(strlen(strpass)*8/2)-i,120,4);

    delay(10);

    lin(320,350,330+(strlen(strpass)*8/2)-i,110,0);
    for (j=0;j<=20;j++)
      if (getpixel(330+(strlen(strpass)*8/2)-i,110-j) == 4)
	putpixel(330+(strlen(strpass)*8/2)-i,110-j,15);

    lin(330+(strlen(strpass)*8/2)-i,110,330+(strlen(strpass)*8/2)-i,120,0);
  }

  if (strcmp(strpass,pass))
  {
    right = 0;
    settextstyle(0,0,4);
    out(40,410,4,"ACCESS DENIED");
    gc();
    out(40,410,0,"ACCESS DENIED");
    settextstyle(0,0,0);
  }
  else
  {
    right = 1;
    settextstyle(0,0,4);
    out(40,410,4,"ACCESS GRANTED");
    gc();
    out(40,410,0,"ACCESS GRANTED");
    settextstyle(0,0,0);
  }
 }
}

void Get_Pass(int nr, char *pass)
{
  switch (nr)
  {
    case 1: Get_Pass_1(pass); break;
  }
}

int huge DetectVGA256()
{
  return 2;
}

int huge DetectVGA256_800600()
{
  return 3;
}

int huge DetectVGA256_800600ex()
{
  return 4;
}

void First_Init(void)
{
  int Gd = DETECT, Gm,ret;

  installuserdriver("svga256",DetectVGA256);
  initgraph(&Gd,&Gm,"");
  ret = graphresult();
  if (ret == -2)
  {
    textmode(3);
    clrscr();
    printf("#2781 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Keine Grafikkarte gefunden ) entdeckt.\n");
    printf("Vergewissern Sie sich, das in ihrem System ein SVGA Karte mit einer");
    printf("Mindestaufl”sung von 800x600 mit 256 installiert ist.");
    printf("Shutdown...");
    exit(5);
  }
  if (ret == -3)
  {
    textmode(3);
    clrscr();
    printf("#2782 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Kein Grafiktreiber gefunden ) entdeckt.\n");
    printf("Vergewissern Sie sich, das in ihrem Verzeichniss ein SVGA 800x600");
    printf("mit mindestens 256 Farben installiert ist.");
    printf("Shutdown...");
    exit(5);
  }
  if (ret == -4)
  {
    textmode(3);
    clrscr();
    printf("#2782 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Falscher Grafiktreiber gefunden ) entdeckt.\n");
    printf("Vergewissern Sie sich, das in ihrem Verzeichniss ein SVGA 800x600");
    printf("mit mindestens 256 Farben installiert ist.");
    printf("Shutdown...");
    exit(5);
  }
  if (ret == -5)
  {
    textmode(3);
    clrscr();
    printf("#2782 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Zu wenig Speicher ) entdeckt.\n");
    printf("Prfen Sie ihren konventionellen Speicher auf mindestens 390KB und ihre");
    printf("Grafikkarte auf mind. 1 MB Ram fr 800x480 und 2 MB fr 800x600.");
    printf("Shutdown...");
    exit(5);
  }
}

void First_Init_ex1(void)
{
  int Gd = DETECT, Gm,ret;

  setdisk(2);
  installuserdriver("svga256",DetectVGA256_800600);
  initgraph(&Gd,&Gm,"");
  ret = graphresult();
  if (ret == -2)
  {
    textmode(3);
    clrscr();
    printf("#2781 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Keine Grafikkarte gefunden ) entdeckt.\n");
    printf("Vergewissern Sie sich, das in ihrem System ein SVGA Karte mit einer");
    printf("Mindestaufl”sung von 800x600 mit 256 installiert ist.");
    printf("Shutdown...");
    exit(5);
  }
  if (ret == -3)
  {
    textmode(3);
    clrscr();
    printf("#2782 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Kein Grafiktreiber gefunden ) entdeckt.\n");
    printf("Vergewissern Sie sich, das in ihrem Verzeichniss ein SVGA 800x600");
    printf("mit mindestens 256 Farben installiert ist.");
    printf("Shutdown...");
    exit(5);
  }
  if (ret == -4)
  {
    textmode(3);
    clrscr();
    printf("#2782 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Falscher Grafiktreiber gefunden ) entdeckt.\n");
    printf("Vergewissern Sie sich, das in ihrem Verzeichniss ein SVGA 800x600");
    printf("mit mindestens 256 Farben installiert ist.");
    printf("Shutdown...");
    exit(5);
  }
  if (ret == -5)
  {
    textmode(3);
    clrscr();
    printf("#2782 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Zu wenig Speicher ) entdeckt.\n");
    printf("Prfen Sie ihren konventionellen Speicher auf mindestens 390KB und ihre");
    printf("Grafikkarte auf mind. 1 MB Ram fr 800x480 und 2 MB fr 800x600.");
    printf("Shutdown...");
    exit(5);
  }
}
void First_Init_ex2(void)
{
  int Gd = DETECT, Gm,ret;

  installuserdriver("svga256",DetectVGA256_800600ex);
  initgraph(&Gd,&Gm,"");
  ret = graphresult();
  if (ret == -2)
  {
    textmode(3);
    clrscr();
    printf("#2781 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Keine Grafikkarte gefunden ) entdeckt.\n");
    printf("Vergewissern Sie sich, das in ihrem System ein SVGA Karte mit einer");
    printf("Mindestaufl”sung von 800x600 mit 256 installiert ist.");
    printf("Shutdown...");
    exit(5);
  }
  if (ret == -3)
  {
    textmode(3);
    clrscr();
    printf("#2782 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Kein Grafiktreiber gefunden ) entdeckt.\n");
    printf("Vergewissern Sie sich, das in ihrem Verzeichniss ein SVGA 800x600");
    printf("mit mindestens 256 Farben installiert ist.");
    printf("Shutdown...");
    exit(5);
  }
  if (ret == -4)
  {
    textmode(3);
    clrscr();
    printf("#2782 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Falscher Grafiktreiber gefunden ) entdeckt.\n");
    printf("Vergewissern Sie sich, das in ihrem Verzeichniss ein SVGA 800x600");
    printf("mit mindestens 256 Farben installiert ist.");
    printf("Shutdown...");
    exit(5);
  }
  if (ret == -5)
  {
    textmode(3);
    clrscr();
    printf("#2782 Kritischer Fehler: Beim Versuch, die Grafikkarte zu initialisieren\n");
    printf("wurde ein Fehler ( 2781: Zu wenig Speicher ) entdeckt.\n");
    printf("Prfen Sie ihren konventionellen Speicher auf mindestens 390KB und ihre");
    printf("Grafikkarte auf mind. 1 MB Ram fr 800x480 und 2 MB fr 800x600.");
    printf("Shutdown...");
    exit(5);
  }
}

void C_Pic(char *str, int x, int y, int rot)
{
  FILE *f;
  int i=0;
  int j=0;
  int k=0;

  unsigned char pic[250] = {"\0"};

  f=fopen(str,"rb");

  fread(&pic[i],225,1,f);
  fclose(f);

  if (rot==0)
  for (i=0;i<=14;i++)
    for (j=0;j<=14;j++)
    {
      if (pic[k]!=0)
      {
	putpixel(x+i*2,y+j*2,pic[k]);
	putpixel(x+i*2+1,y+j*2,pic[k]);
	putpixel(x+i*2,y+j*2+1,pic[k]);
	putpixel(x+i*2+1,y+j*2+1,pic[k]);
      }
      k++;
    }
  if (rot==1)
  for (i=14;i>=0;i--)
    for (j=0;j<=14;j++)
    {
      if (pic[k]!=0)
      {
	putpixel(x+i*2,y+j*2,pic[k]);
	putpixel(x+i*2+1,y+j*2,pic[k]);
	putpixel(x+i*2,y+j*2+1,pic[k]);
	putpixel(x+i*2+1,y+j*2+1,pic[k]);
      }
      k++;
    }
    }

void boxw(int x1, int y1, int x2, int y2, char *titel, int clear)
{
  box(x1,y1,x2,y2,15,7,22);
  box(x1+5,y1+3,x2-5,y1+18,15,1,22);
  if (clear)  box(x1+2,y1+18,x2-2,y2-2,22,0,15);
  outsc(x1,x2,y1+5,15,titel);
  C_Pic("c:\\vms\\source\\kernel\\gfx\\pic\\xgk.pic",x2-25,y1-6,0);

}

boxm::boxm(int x1, int y1, int x2, int y2, char *titel, int color)
{
  boxa(x1,y1,x2,y2,15,Displayset.BACK_BOX_COLOR,22,Displayset.BACK_BOX_STYLE);
  boxa(x1+5,y1+3,x2-5,y1+18,15,Displayset.TITEL_COLOR,22,Displayset.TITEL_STYLE);
  outsc(x1,x2,y1+7,Displayset.TITELDRAW_COLOR,titel);
  C_Pic("c:\\vms\\source\\kernel\\gfx\\pic\\xgk.pic",x2-28,y1-5,0);
  bx1 = x1;
  by1 = y1;
  bx2 = x2;
  by2 = y2;
  bc = color;
  strcpy(btitel,titel);


}

void boxm::show()
{
  boxa(bx1,by1,bx2,by2,15,Displayset.BACK_BOX_COLOR,22,Displayset.BACK_BOX_STYLE);
  boxa(bx1+5,by1+3,bx2-5,by1+18,15,Displayset.TITEL_COLOR,22,Displayset.TITEL_STYLE);
  outsc(bx1,bx2,by1+7,Displayset.TITELDRAW_COLOR,btitel);
  C_Pic("c:\\vms\\source\\kernel\\gfx\\pic\\xgk.pic",bx2-28,by1-5,0);
}

void boxm::hide()
{
  boxa(bx1+5,by1+3,bx2-5,by1+18,15,Displayset.TITEL_DECOLOR,22,Displayset.TITEL_STYLE);
  outsc(bx1,bx2,by1+7,Displayset.TITELDRAW_DECOLOR,btitel);
  C_Pic("c:\\vms\\source\\kernel\\gfx\\pic\\xgk.pic",bx2-28,by1-5,0);
}

RightMenu::RightMenu(int x, int y)
{
  x1 = x;
  x2 = x1 + 10;
  y1 = y;
  y2 = y1 + 10;
  count = 0;
}

void RightMenu::Add(char *o)
{
  int length;
  strcpy(opt[count],o);

  length = strlen(o) * 8;
  length+= x1 + 20;

  if (length > x2) x2 = length;
  count++;
  y2 += 10;
}

void RightMenu::Show(void)
{
  box(x1,y1,x2,y2,15,7,22);
  for (int i=0;i<count;i++)
  {
    if (strcmp(opt[i],"LINE"))
      outs(x1+5,y1+3+(i*10),0,opt[i],0);
    else lin(x1+5,y1+7+(i*10),x2-5,y1+7+(i*10),22);
  }
}

int RightMenu::Handle(void)
{
  mask = 1;
  FILE *f;

  f=fopen("c:\\vms\\source\\temp.box","wt");

  for (int i=x1;i<=x2;i++)
    for (int j=y1;j<=y2;j++)
    {
      fputc(getpixel(i,j),f);
    }

  fclose(f);
  Show();
  int x,y,b,ok;
  x=y=b=ok=0;
  int sel = 0;
  while (!ok)
  {
    do_mouse(&x,&y,&b,1);
    if ((x<x2) && (x>x1) && (y>y1) && (y<y2))
    {
      sel = (y-y1+5) / 10;
      ok =1 ;
    }
    else
    {
      sel = 255;
      ok = 1;
    }
  }
  f=fopen("c:\\vms\\source\\temp.box","rt");

  for (i=x1;i<=x2;i++)
    for (int j=y1;j<=y2;j++)
    {
      putpixel(i,j,fgetc(f));
    }

  fclose(f);
  mask = 0;
  return sel;


}


void Save_Screen(int k)
{
  FILE *f;

  char str[255];

  sprintf(str,"c:\\vms\\source\\kernel\\temp\\screen%d.dat",k);

  f=fopen(str,"wt");

  for (int i=0;i<=800;i++)
    for (int j=0;j<=600;j++)
    {
      fputc(getpixel(i,j),f);
    }
  fclose(f);
}

void Restore_Screen(int k)
{
  FILE *f;
  char str[255];

  sprintf(str,"c:\\vms\\source\\kernel\\temp\\screen%d.dat",k);

  f=fopen(str,"rt");

  for (int i=0;i<=800;i++)
    for (int j=0;j<=600;j++)
      putpixel(i,j,fgetc(f));
  fclose(f);
}
void Save_Screen(void)
{
  FILE *f;

  f=fopen("c:\\vms\\source\\kernel\\temp\\screen.dat","wt");

  for (int i=0;i<=800;i++)
    for (int j=0;j<=600;j++)
    {
      fputc(getpixel(i,j),f);
    }
  fclose(f);
}

void Restore_Screen(void)
{
  FILE *f;

  f=fopen("c:\\vms\\source\\kernel\\temp\\screen.dat","rt");

  for (int i=0;i<=800;i++)
    for (int j=0;j<=600;j++)
      putpixel(i,j,fgetc(f));
  fclose(f);
}




void Set_Palette(unsigned int start,unsigned int end,unsigned char *palette)
{
  unsigned int i;
  outportb(0x03c8,start);
  start*=3;
  end++;
  end*=3;
  for (i=start;i<end;i++) outportb(0x03c9,palette[i]);
}

void Get_Palette(unsigned int start,unsigned int end,unsigned char *palette)
{
  unsigned int i;
  outportb(0x03c7,start);
  start*=3;
  end++; end*=3;
  for (i=start;i<end;i++) palette[i] = inportb(0x03c9);
}

void abox2(int x1, int y1, int x2, int y2, int c1, int c2, int c3)
{
  setwritemode(XOR_PUT);

  for (int i=x1;i<=x2;i++)
  {
    setcolor(c1);
    line(400,600,i,y1);
    line(400,600,i,y1);
    putpixel(i,y1,c1);
    delay(10);
  }
  setwritemode(COPY_PUT);
}

void openboxlr(int x1, int y1, int x2, int y2, int c1, int c2, int c3, int del)
{

  for (int i=x1;i<=x2;i++)
  {
    for (int j=0;j<=del;j++)
      linp(i,y1,i,y2,c2,c1,c3);
  }
}

void A_box(int x ,int y, int x2, int y2, int f, int f2)
{
  box(x,y,x2,y2,f,f,f);
  Rect(x,y,x2,y2,f2);

  setcolor(f2);
  setfillstyle(1,f);
  pieslice(x,y+(abs(y-y2)/2),90,270,abs(y2-y)/2);
  pieslice(x2,y+(abs(y-y2)/2),270,360,abs(y2-y)/2);
  pieslice(x2,y+(abs(y-y2)/2),360,90,abs(y2-y)/2);
}

void AV_box(int x ,int y, int x2, int y2, int f, int f2)
{
  box(x,y,x2,y2,f,f,f);
  Rect(x,y,x2,y2,f2);

  setcolor(f2);
  setfillstyle(1,f);
  pieslice(x+(abs(x-x2))/2,y,0,180,abs(x2-x)/2);
  pieslice(x+(abs(x-x2))/2,y2,180,360,abs(x2-x)/2);
}

void ab1(int x, int y, int x2, int y2, int c1, int c2, int c3, char *str, int font, int charsize)
{
  A_box(x,y,x2,y2,c1,c2);
  outsc(x,x2,y+1,c3,str,font,charsize);
}

void ab1_a(int x, int y, int x2, int y2, int c1, int c2, int c3, char *str, int font, int charsize)
{
  int mid = x+abs(x2-x)/2;

  for (int i=0;i<=abs(x2-x)/2;i++)
    A_box(mid-i,y,mid+i,y2,c1,c2);
  outsc(x,x2,y+1,c3,str,font,charsize);
}

void ab1v_a(int x, int y, int x2, int y2, int c1, int c2)
{
  int mid = y+abs(y2-y)/2;

  for (int i=0;i<=abs(y2-y)/2;i++)
    AV_box(x,mid-i,x2,mid+i,c1,c2);
}