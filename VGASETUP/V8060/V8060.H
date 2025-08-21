#include <stdio.h>

char gc(void);
void abox2(int x1, int y1, int x2, int y2, int c1, int c2, int c3);
void Picg(char *str, int x, int y);
void Save_Screen(void);
void Restore_Screen(void);
void box(int x1, int y1, int x2, int y2, int f1, int f2, int f3);
void boxopen(int x1, int y1, int x2, int y2, int f1, int f2, int f3, int del);
void Get_String(char string[],int color, int x, int y, int length);
void Get_Stringp(char string[],int color, int x, int y, int length);
void Retrace(void);
void GetPal(unsigned char palette[]);
void SetPal(unsigned char palette[]);
void Down_Pal(unsigned char palette[], int value);
void lin(int x1, int y1, int x2, int y2, int col);
void linp(int x1, int y1, int x2, int y2, int col1, int col2,int col3);
void outs(int x, int y, int col, char *string, int speed, int style = 255, int charsize = 255);
void outsl(int x, int y, int col, char *string, int speed);
void out(int x, int y, int col, char*string);
void outc(int x,int x2, int y, int col, char*string);
void outsc(int x,int x2, int y, int col, char*string, int font = 255, int charsize = 255);
void Pic(char *str, int x, int y);
void Get_Pass(int nr, char* pass);
void First_Init(void);
void First_Init_ex1(void);
void First_Init_ex2(void);
void t_out(char *string);
void Rect(int x1, int y1, int x2, int y2, int f1);
void C_Pic(char *str, int x, int y, int rot);
int  Get_Stringad(char string[],int color, int x, int y, int length, int back);
void box2(int x1, int y1, int x2, int y2, int c1, int c2, int c3);
void box2(int x1, int y1, char *titel, int c1, int c2, int c3, int c4);
void abox1(int x1, int y1, int x2, int y2, int c1, int c2, int c3, int del);
void boxw(int x1, int y1, int x2, int y2, char *titel, int clear = 0);
void Set_Palette(unsigned int start,unsigned int end,unsigned char *palette) ;
void Get_Palette(unsigned int start,unsigned int end,unsigned char *palette);
void openboxlr(int x1, int y1, int x2, int y2, int c1, int c2, int c3, int del);
void A_box(int x ,int y, int x2, int y2, int f, int f2);
void ab1(int x, int y, int x2, int y2, int c1, int c2, int c3, char *str, int font, int charsize);
void AV_box(int x ,int y, int x2, int y2, int f, int f2);
void ab1_a(int x, int y, int x2, int y2, int c1, int c2, int c3, char *str, int font, int charsize);
void ab1v_a(int x, int y, int x2, int y2, int c1, int c2 );
void Save_Screen(int k);
void Restore_Screen(int k);

class boxm
{
  private:
    int bx1;
    int bx2;
    int by1;
    int by2;
    int bc;
    char btitel[255];
    FILE *file;
    int handle;

  public:
    boxm(int x1, int y1, int x2, int y2, char *titel, int color);
    void hide();
    void show();
};


class RightMenu
{
  private:
    int x1;
    int x2;
    int y1;
    int y2;
    int count;
    char opt[10][255];

  public:
    RightMenu(int x, int y);
    void Add(char *o);
    void Show(void);
    int Handle();
};


