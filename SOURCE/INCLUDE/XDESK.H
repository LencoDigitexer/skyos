#define X_TYP_DIALOG    			1
	#define X_TYP_DIALOG_NORMAL    			1
	#define X_TYP_DIALOG_STARTMENU 			2
	#define X_TYP_DIALOG_STARTMENU_SUB		3

#define X_TYP_BUTTON    			2
#define X_TYP_EDIT      			3
#define X_TYP_AD_EDIT      			4

struct XS_Dialog
{
  int typ;
  char c1,c2,c3;
  char cg,cg2;
  int x1, x2, y1, y2;
  char titel[255];
  int attribut;
  void *objects[60];
  int objects_used;
  int objects_selected;
  int visible;
  int *old_screen;
};

struct XS_Button
{
  int typ;
  char c1,c2,c3;
  char cg,cg2;
  int x1, x2, y1, y2;
  char titel[255];
  int attribut;
  int code;
};

struct XS_Edit
{
  int typ;
  char c1,c2,c3;
  char cg,cg2;
  int x1, x2, y1, y2;
  char text[255];
  int attribut;
  int length;
};




