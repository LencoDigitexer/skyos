#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphics.h>
#include <dos.h>
#include "v8060\vcsobj.h"
#include "mouse\mouse.h"
#include "v8060\v8060.h"

InputBox::InputBox()
{
  style.x1 = 50;
  style.x2 = 250;
  style.y1 = 260;
  style.y2 = 310;
  style.c1 = 77;
  style.c2 = 78;
  style.c3 = 79;
  style.type = 0;
  sprintf(style.titel,"\0");
  c4 = 15;
}

InputBox::InputBox(S_Box new_style)
{
  style.x1 = new_style.x1;
  style.y1 = new_style.y1;
  style.x2 = new_style.x2;
  style.y2 = new_style.y2;
  style.c1 = new_style.c1;
  style.c2 = new_style.c2;
  style.c3 = new_style.c3;
  style.x1 = new_style.x1;
  style.type = new_style.type;
  sprintf(style.titel, new_style.titel);
  c4 = 15;
}

InputBox::InputBox(int x1, int y1, int x2, int y2, int c1, int c2, int c3, char *titel,int type)
{
  style.x1 = x1;
  style.y1 = y1;
  style.x2 = x2;
  style.y2 = y2;
  style.c1 = c1;
  style.c2 = c2;
  style.c3 = c3;
  style.x1 = x1;
  style.type = type;
  sprintf(style.titel, titel);
  c4 = 15;
}

void InputBox::show(void)
{
  box(style.x1, style. y1, style.x2, style.y2,style.c1, style.c2, style.c3);
  box(style.x1+10, style. y1+25, style.x2-10, style.y1+38,0,0,15);
  outc(style.x1, style.x2, style.y1+4, c4,style.titel);
}

void InputBox::hide(void)
{
  box(style.x1, style. y1, style.x2, style.y2,0,0,0);
}

void InputBox::input(void)
{
  Get_String(string, c4, style.x1+ 12, style.y1 + 27, 255);
}

void InputBox::read(char *str)
{
  input();
  strcpy(str, string);
}

InputBox::~InputBox(void)
{
}

NormBox::NormBox()
{
  style.x1 = random(300);
  style.x2 = style.x1 + 200+random(100);
  style.y1 = random(400);
  style.y2 = style.y1+ 70+random(100);
  style.c1 = 77;
  style.c2 = 78;
  style.c3 = 79;
  style.type = 0;
  sprintf(style.titel,"\0");
  c4 = 15;
  count = 0;
  rel_x = 10;
}

NormBox::NormBox(S_Box new_style)
{
  style.x1 = new_style.x1;
  style.y1 = new_style.y1;
  style.x2 = new_style.x2;
  style.y2 = new_style.y2;
  style.c1 = new_style.c1;
  style.c2 = new_style.c2;
  style.c3 = new_style.c3;
  style.x1 = new_style.x1;
  style.type = new_style.type;
  sprintf(style.titel, new_style.titel);
  c4 = 15;
  count = 0;
  rel_x = 10;
}

NormBox::NormBox(int x1, int y1, int x2, int y2, int c1, int c2, int c3, char *titel,int type)
{
  style.x1 = x1;
  style.y1 = y1;
  style.x2 = x2;
  style.y2 = y2;
  style.c1 = c1;
  style.c2 = c2;
  style.c3 = c3;
  style.x1 = x1;
  style.type = type;
  sprintf(style.titel, titel);
  c4 = 15;
  count = 0;
  rel_x = 10;

}

void NormBox::show(void)
{
  box(style.x1, style. y1, style.x2, style.y2,style.c1, style.c2, style.c3);
  outc(style.x1, style.x2, style.y1+4, c4,style.titel);
  count = 0;

  if ((style.type & FRAMED) == FRAMED)
    box(style.x1+3, style. y1+20, style.x2-3, style.y2-3,0,0,15);
  if ((style.type & 2) == 2)
  {
    for (int i=5;i>0;i--)
    {
      lin(style.x1,style.y1+(5-i),style.x1+i,style.y1+(5-i),0);
      lin(style.x2,style.y1+(5-i),style.x2-i,style.y1+(5-i),0);
    }
  }
}

void NormBox::hide(void)
{
  box(style.x1, style. y1, style.x2, style.y2,0,0,0);
}

void NormBox::print(int c5, char *str, int st)
{
  if (st == 1)
    outsc(style.x1+ rel_x, style.x2 - rel_x,style.y1 + 25 + count*12, c5, str);
  if (st == 0)
    outs(style.x1+ rel_x, style.y1 + 25 + count*12, c5, str,0);
  count+=1;
}

void NormBox::print()
{
  count+=1;
}
void NormBox::Caption(char *str)
{
  strcpy(style.titel,str);
}

void NormBox::Change(int x1, int y1, int x2, int y2)
{
  style.x1=x1;
  style.y1=y1;
  style.x2=x2;
  style.y2=y2;
}

void NormBox::SetTextX(int wert)
{
  rel_x = wert;
}
void NormBox::Type(int type)
{
  style.type = type;
}
void NormBox::SetH(int wert)
{
  style.y2 = style.y1+wert;
}

void NormBox::SetW(int wert)
{
  style.x2 = style.x1+wert;
}

NormBox::~NormBox(void)
{
}

C_Objekt::C_Objekt()
{
}

void C_Objekt::draw()
{
}

void C_Objekt::hook()
{
}

int C_Objekt::handle()
{
}

C_Box::C_Box(int nx1, int ny1, int nx2, int ny2, int c1, int c2, int c3, int ch, char *titel, int code, int col)
  : C_Objekt()
{
  c = code;
  b.x1 = nx1;
  if (nx2 == 0) nx2 = nx1 + strlen(titel)*8+20;
  b.x2 = nx2;
  b.y1 = ny1;
  if (ny2 == 0) ny2 = ny1 + 20;
  b.y2 = ny2;
  b.c1 = c1;
  b.c2 = c2;
  b.c3 = c3;
  b.ch = ch;
  b.type = 0;
  sprintf(b.titel,titel);
  b.ct = col;
}

void C_Box::draw()
{
  box(b.x1, b. y1, b.x2, b.y2,b.c1, b.c2, b.c3);
  outsc(b.x1, b.x2,b.y1+6,b.ct,b.titel);
}

void C_Box::hook()
{
  Rect(b.x1,b.y1,b.x2,b.y2,b.ch);
}

int C_Box::handle()
{
  return 100 +c;
}


C_BoxTime::C_BoxTime(int nx1, int ny1, int nx2, int ny2, int c1, int c2, int c3, int ch, char *titel, int code, int col)
  : C_Objekt()
{
  c = code;
  b.x1 = nx1;
  if (nx2 == 0) nx2 = nx1 + strlen(titel)*8+20;
  b.x2 = nx2;
  b.y1 = ny1;
  if (ny2 == 0) ny2 = ny1 + 20;
  b.y2 = ny2;
  b.c1 = c1;
  b.c2 = c2;
  b.c3 = c3;
  b.ch = ch;
  b.type = 0;
  sprintf(b.titel,titel);
  b.ct = col;
}

void C_BoxTime::draw()
{
  box(b.x1, b. y1, b.x2, b.y2,b.c1, b.c2, b.c3);
  outs(b.x1 + 10, b.y1+6,b.ct,b.titel,0);
}

void C_BoxTime::hook()
{
  Rect(b.x1,b.y1,b.x2,b.y2,b.ch);
}

int C_BoxTime::handle()
{
  struct time t;
  static struct time t2;

  char str[255];

  gettime(&t);
  if (t2.ti_sec != t.ti_sec)
  {
    sprintf(str,"%02d:%02d:%02d",t.ti_hour,t.ti_min,t.ti_sec);
    strcpy(b.titel,str);
    draw();
    gettime(&t2);
  }
}

C_Input::C_Input(int nx1, int ny1, int nx2, int ny2, int c1, int c2, int c3, int ch, char *titel)
  : C_Objekt()
{
  b.x1 = nx1;
  if (nx2 == 0) nx2 = nx1 + strlen(titel)*8+20;
  b.x2 = nx2;
  b.y1 = ny1;
  if (ny2 == 0)
  {
    if (strlen(titel)==0) ny2 = ny1 + 15;
    else ny2 = ny1 + 35;
  }
  b.y2 = ny2;
  b.c1 = c1;
  b.c2 = c2;
  b.c3 = c3;
  b.ch = ch;
  b.type = 0;
  sprintf(b.titel,titel);
  sprintf(inputtext,"\0");
}

void C_Input::set(char *str)
{
  strcpy(inputtext,str);
}

void C_Input::set(int d)
{
  sprintf(inputtext,"%d",d);
}

void C_Input::set(float d)
{
  sprintf(inputtext,"%2.2f",d);
}

void C_Input::get(char *str)
{
  strcpy(str,inputtext);
}

void C_Input::draw()
{
  box(b.x1, b. y1, b.x2, b.y2,b.c1, b.c2, b.c3);
  box(b.x1+2,b.y1+2,b.x2-2,b.y2-2,0,0,0);
  outs(b.x1+12,b.y1+5,15,inputtext,0);
  outs(b.x1,b.y1-10,15,b.titel,0);
}

void C_Input::hook()
{
  Rect(b.x1,b.y1,b.x2,b.y2,b.ch);
}

int C_Input::handle()
{
  inputtext[0] = '\0';
  draw();
  Get_String(inputtext,15,b.x1+12,b.y1+5,70);

  return 0;
}

C_BoxText::C_BoxText(int nx1, int ny1, int nx2, int ny2, int c1, int c2, int c3, int ch, char *titel)
  : C_Objekt()
{
  b.x1 = nx1;
  if (nx2 == 0) nx2 = nx1 + strlen(titel)*8+20;
  b.x2 = nx2;
  b.y1 = ny1;
  if (ny2 == 0)
  {
    if (strlen(titel)==0) ny2 = ny1 + 15;
    else ny2 = ny1 + 35;
  }
  b.y2 = ny2;
  b.c1 = c1;
  b.c2 = c2;
  b.c3 = c3;
  b.ch = ch;
  b.type = 0;
  sprintf(b.titel,titel);
  count = 0;
  Akt = First = NULL;
}

void C_BoxText::draw()
{
  box(b.x1, b. y1, b.x2, b.y2,b.c1, b.c2, b.c3);
  if (Akt->str[0] != '\0')
    outsc(b.x1,b.x2,b.y1+5,15,Akt->str,0);

  outs(b.x1,b.y1-17,15,b.titel,0,2,5);
}

void C_BoxText::hook()
{
  Rect(b.x1,b.y1,b.x2,b.y2,b.ch);
}

int C_BoxText::handle()
{
  if (Akt->Next != NULL) Akt = Akt->Next;
  else Akt = First;

  draw();
  return 0;
}

void C_BoxText::add(char *str)
{
  PSL dummy;
  PSL newdum;

  dummy = First;

  while (dummy->Next)
    dummy = dummy->Next;

  if (!(newdum = new SSL))
    exit(9);

  if (First!=NULL)
    dummy->Next = newdum;

  newdum->Next = NULL;
  strcpy(newdum->str,str);
  dummy = dummy->Next;
  dummy = newdum;

  if (First==NULL)
    Akt = First = dummy;
}

int C_BoxText::get(char *str)
{
  PSL dummy;
  int count = 0;

  dummy = First;

  while (dummy != Akt)
  {
    count++;
    dummy = dummy->Next;
  }
  strcpy(str,dummy->str);
  return count;
}

void C_BoxText::showentry(int d)
{
  Akt = First;

  for (int i=0;i<d;i++)
    Akt = Akt->Next;
}

C_Manager::C_Manager()
{
  First = Last = NULL;
  hooked = 255;
  used = 0;
}

void C_Manager::New(POBJ nb)
{
  PSLOBJ newobj;

  if (!(newobj = new SL_Obj))
    exit(9);

  if (First == NULL)
  {
    First = Last = newobj;
    newobj->Last = newobj->Next = NULL;
  }
  else
  {
    Last->Next = newobj;
    newobj->Last = Last;
    newobj->Next = NULL;
    Last = newobj;
  }


  if (hooked == 255) hooked = 0;
  else hooked++;

  if (!(newobj->b = new C_Objekt))
    exit(9);

  newobj->b = nb;


  used++;
}

void C_Manager::Hook(int h)
{
  PSLOBJ dummy;
  C_Objekt *b;

  Draw(hooked);
  hooked = h;

  dummy = First;
  for (int i=1;i<=h;i++)
    dummy = dummy->Next;

  b = dummy->b;

  b->hook();
}

void C_Manager::Draw(void)
{
  for (int i=0;i<(used);i++)
    Draw(i);
}

void C_Manager::Draw(int i)
{
  PSLOBJ dummy;
  C_Objekt *b;

  dummy = First;
  for (int j=1;j<=i;j++)
    dummy = dummy->Next;

  b = dummy->b;

  b->draw();
}

int C_Manager::Handle(int trans)
{
  PSLOBJ dummy = First;
  PSLOBJ alw = First;
  C_Objekt *b;

  for (int i=1;i<trans;i++)
    alw = alw->Next;

  for ( i=0;i<hooked;i++)
   dummy = dummy->Next;

  char ch;
  int ende = 0;

  while (!ende)
  {
    dummy->b->hook();

    if (trans) alw->b->handle();

    if (kbhit()) ch = gc();
    else ch = '.';

//    while (!ipxRecvChk()) recvPacket();

    if (ch == 13)
    {
      int ret = dummy->b->handle();
      if (ret>=100) ende = ret - 100;
    }
    if (ch == 27) ende = 250;
    if (ch==9)
    {
      dummy->b->draw();
      if (dummy->Next) dummy = dummy->Next;
      else dummy = First;
      dummy->b->hook();
    }
    if (ch == 0)
    {
      ch = gc();
      if (ch == 15)
      {
	dummy->b->draw();
	if (dummy->Last) dummy =dummy->Last;
	else dummy = Last;
	dummy->b->hook();
      }
    }
  }
  if (ende == 250) return 255;
  else return ende;
}

C_Manager::~C_Manager()
{
  PSLOBJ dummy;
  while (First)
  {
    dummy = First;
    First = First->Next;
    delete dummy;
  }
}

C_Desktop::C_Desktop()
{
  First = Last = NULL;
  used = 0;
}

void C_Desktop::New(PLINK nb)
{
  PSLLINK newobj;

  if (!(newobj = new SL_Link))
    exit(9);

  if (First == NULL)
  {
    First = Last = newobj;
    newobj->Last = newobj->Next = NULL;
  }
  else
  {
    Last->Next = newobj;
    newobj->Last = Last;
    newobj->Next = NULL;
    Last = newobj;
  }

  newobj->b = new C_Link;
  newobj->b = nb;
  used++;
}

void C_Desktop::Draw(void)
{
  for (int i=0;i<(used);i++)
    Draw(i);
}

void C_Desktop::Draw(int i)
{
  PSLLINK dummy;
  C_Link *b;

  dummy = First;
  for (int j=1;j<=i;j++)
    dummy = dummy->Next;

  b = dummy->b;

  b->draw();
}

void C_Desktop::Check_Link(int x, int y)
{
  int mx=0, my=0, mb=1;
  PSLLINK dummy;
  C_Link *b;
  int ox, oy ,ol ,ow;
  int found = 255;

  dummy = First;
  for (int j=0;j<used;j++)
  {
    b = dummy->b;
    b->getinfo(&ox,&oy,&ol,&ow);
    if ((x>ox) && (x<(ox+ol)) && (y>oy) && (y<(oy+ow)))
    {
      found = j;
    }
    dummy = dummy->Next;
  }
  if (found!=255)
  {
    while (mb==1)
    {
      mouse(&mx,&my,&mb);
      if (mb==1)
      {
	b->move(mx,my);
	Draw(found);
      }
    }

  }
}


C_Desktop::~C_Desktop()
{
  PSLLINK dummy;
  while (First)
  {
    dummy = First;
    First = First->Next;
    delete dummy;
  }
}

C_Link::C_Link()
{
  x=y=l=w= 0;
  cox = x;
  coy = y;

  pic[0] = '\0';
  name[0] = '\0';
}

C_Link::C_Link(int nx, int ny, int nl, int nw, char *npic, char *nname)
{
  x=nx;
  y=ny;
  l=nl;
  w=nw;
  strcpy(pic,npic);
  strcpy(name,nname);
  for (int i=0;i<50;i++)
    for (int j=0;j<50;j++)
      back[i][j] = getpixel(x+i,y+j);
  cox = x;
  coy = y;
}

void C_Link::draw(void)
{

  for (int i=0;i<50;i++)
    for (int j=0;j<50;j++)
      putpixel(cox+i,coy+j,back[i][j]);
  outs(cox+(l/2)-(strlen(name)*8/2),coy+w+5,7,name,0);

  for (i=0;i<50;i++)
    for (int j=0;j<50;j++)
      back[i][j] = getpixel(x+i,y+j);

  Picg(pic,x,y);
  outs(x+(l/2)-(strlen(name)*8/2),y+w+5,1,name,0);
  cox = x;
  coy = y;
};

void C_Link::getinfo(int *ox, int *oy, int *ol, int *ow)
{
  *ox = x;
  *oy = y;
  *ol = l;
  *ow = w;
}

void C_Link::move(int nx, int ny)
{
  cox = x;
  coy = y;
  x = nx;
  y = ny;
}
