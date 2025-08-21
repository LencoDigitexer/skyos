#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

struct entry
{
  long addr;
  unsigned char name[20];
};

unsigned long htoa(unsigned char *str)
{
  int length;
  int i=0;
  int j;
  long double m;
  int k;
    long double value=0;

  length = strlen(str);
  length--;

  while (length >=0)
  {
    if (str[length] == '0') j = 0;
    if (str[length] == '1') j = 1;
    if (str[length] == '2') j = 2;
    if (str[length] == '3') j = 3;
    if (str[length] == '4') j = 4;
    if (str[length] == '5') j = 5;
    if (str[length] == '6') j = 6;
    if (str[length] == '7') j = 7;
    if (str[length] == '8') j = 8;
    if (str[length] == '9') j = 9;
    if (str[length] == 'A') j = 10;
    if (str[length] == 'B') j = 11;
    if (str[length] == 'C') j = 12;
    if (str[length] == 'D') j = 13;
    if (str[length] == 'E') j = 14;
    if (str[length] == 'F') j = 15;

    if (i==0)
    {
      value = j;
    }
    else
    {
      m = 16;
      for (k=1;k<i;k++)
      {
	m = m * 16;
      }
      if (j==0)
	value = value + m * 1;
      else
	value = value + m * j;
    }
    i++;
    length--;
  }
  printf("\nvalue: %ld",value);


}

int main(void)
{
  FILE *f, *f2;
  struct entry e;
  unsigned char str[255];
  unsigned char str2[255];
  int j;
  int i;
  int count;
  int old;

  unsigned char *strd = "111111\0";


  printf("EXCEPTION INFO Viewer (c) 1998 by Szeleney Robert\n");
  printf("Source File   : object.dat\n");
  printf("File to append: kernel.sys\n\n");

  f = fopen("c:\\sky\\object\\object.dat","rt");
  f2 = fopen("c:\\sky\\object\\kernel.sys","a+");

  fread(str,1,50,f);
  i = 0;

  while (str[i] != '\n') i++;
  i++;
  i++;
  fseek(f,i,SEEK_SET);
  old = i;

  while (!feof(f))
  {
     count = fread(str,1,50,f);
     if (str[9] == 'T')
     {
        memset(&e,0,sizeof(e));

        for (j=0;j<8;j++)
          str2[j] = str[j];

        str2[8] = '\0';
        e.addr = atoi(str2);


        j = 12;
        while (str[j] != '\n')
        {
          e.name[j-12] = str[j];
          j++;
        }
        e.name[j] = '\0';


     }

     i = 0;
     while (str[i] != '\n') i++;
     i++;
     i++;
     fseek(f,old + i, SEEK_SET);
     old += i;
  }
}
