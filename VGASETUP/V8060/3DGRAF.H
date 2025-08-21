struct THREED_POINT
{
  int x,y,z;
};

class THREED_OBJECT
{
  private:
    struct THREED_POINT POINTS[100];
    int folge[100];
    int maxpoints;
    int folgemax;
    float alpha, beta, size;
    int RELX;
    int RELY;

  public:
    THREED_OBJECT();
    THREED_OBJECT(char *filename);
    void Draw(int color);
    void Rot(int al, int be);
    void Aktuell(int num);
};

void New_Object(void);
