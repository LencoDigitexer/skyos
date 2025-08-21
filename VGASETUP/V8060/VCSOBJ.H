/* TYPE:
   00000000
	  inbox
	 cutted
*/

#define FRAMED 1

struct S_Box
{
    int x1;
    int y1;
    int x2;
    int y2;
    int c1;
    int c2;
    int c3;
    int ch;
    char titel[50];
    int type;
    int ct;
    int hookable;
};

typedef struct S_SL
{
  struct S_SL *Next;
  char str[255];
}SSL;

typedef SSL *PSL;

class NormBox
{
  private:
    struct S_Box style;
    char string[255];
    int c4;
    int count;
    int rel_x;

  public:
    NormBox(void);
    NormBox(S_Box new_style);
    NormBox(int x1, int y1, int x2, int y2, int c1, int c2, int c3, char *titel,int type);

    void Caption(char *str);
    void show(void);
    void hide(void);
    void print(int c5, char*str, int style=0);
    void print();
    void Change(int x1, int y1, int x2, int y2);
    void Type(int type);
    void SetTextX(int wert);
    void SetH(int wert);
    void SetW(int wert);
    ~NormBox();
};

class InputBox
{
  private:
    struct S_Box style;
    char string[255];
    int c4;

  public:
    InputBox(void);
    InputBox(S_Box new_style);
    InputBox(int x1, int y1, int x2, int y2, int c1, int c2, int c3, char *titel,int type);

    void show(void);
    void hide(void);
    void input(void);
    void read(char *str);
    void set(char *str);
    ~InputBox();
};

class C_Objekt
{
  private:

  public:
    C_Objekt();
    virtual void draw();
    virtual void hook();
    virtual int handle();
};

typedef C_Objekt* POBJ;

class C_Box  : public C_Objekt
{
  private:
    struct S_Box b;
    int c;

  public:
    C_Box(int nx1, int ny1, int nx2, int ny2, int c1, int c2, int c3, int ch, char *titel, int code, int col=15);
    void draw();
    void hook();
    int  handle();
};

class C_BoxTime  : public C_Objekt
{
  private:
    struct S_Box b;
    int c;

  public:
    C_BoxTime(int nx1, int ny1, int nx2, int ny2, int c1, int c2, int c3, int ch, char *titel, int code, int col=15);
    void draw();
    void hook();
    int  handle();
};

class C_BoxText  : public C_Objekt
{
  private:
    PSL First;
    PSL Akt;
    struct S_Box b;
    int count;

  public:
    C_BoxText(int nx1, int ny1, int nx2, int ny2, int c1, int c2, int c3, int ch, char*titel);
    void add(char *str);
    int get(char *str);
    void draw();
    void hook();
    int  handle();
    void showentry(int d);
};

class C_Input  : public C_Objekt
{
  private:
    struct S_Box b;
    char inputtext[255];

  public:
    C_Input(int nx1, int ny1, int nx2, int ny2, int c1, int c2, int c3, int ch, char *titel);
    void draw();
    void hook();
    int  handle();
    void set(char *str);
    void set(int d);
    void set(float d);
    void get(char *str);
};

typedef struct S_SL_Obj
{
  struct S_SL_Obj *Last;
  struct S_SL_Obj *Next;
  C_Objekt *b;
} SL_Obj;

typedef SL_Obj *PSLOBJ;

class C_Manager
{
  private:
    PSLOBJ First;
    PSLOBJ Last;
    int hooked;
    int used;

  public:
    C_Manager();

    void New(POBJ nb);
    int  Handle(int trans);
    void Hook(int h);
    void Draw(void);
    void Draw(int i);

    ~C_Manager();
};

class C_Link
{
  private:
    int x;
    int y;
    int l;
    int w;
    int cox;
    int coy;
    char pic[100];
    char back[50][50];
    char name[20];

  public:
    C_Link();
    C_Link(int nx, int ny, int nl, int nw, char* npic, char *nname);
    void draw();
    void move(int nx, int ny);
    void getinfo(int *ox, int *oy, int *ol, int *ow);
};

typedef C_Link * PLINK;

typedef struct S_SL_Link
{
  struct S_SL_Link *Last;
  struct S_SL_Link *Next;
  C_Link *b;
} SL_Link;

typedef SL_Link *PSLLINK;

class C_Desktop
{
  private:
    PSLLINK First;
    PSLLINK Last;
    int used;

  public:
    C_Desktop();

    void New(PLINK nb);
    void Draw(void);
    void Draw(int i);
    void Check_Link(int x, int y);

    ~C_Desktop();
};

