/************************************************************************/
/* CArchPage
/************************************************************************/
/* This is a object for one page
/* You also can allocate them.
/************************************************************************/
class CArchPage
{
  private:
    unsigned int base;

  public:
    unsigned char present;
    unsigned char write;
    unsigned char user;
    unsigned char accessed;
    unsigned char dirty;
    unsigned char avail;

  public:
    CArchPage();

    void SetBase(unsigned int);
    unsigned int GetBase(void);

    unsigned int Alloc(void);
    unsigned int Format(void);
};

/************************************************************************/
/* CArchPage
/************************************************************************/
/* This is an object for one page table.
/************************************************************************/
class CArchPageTable : public CArchPage
{
  private:

  public:
    CArchPageTable();

    void Load(void);
    void Store(void);

    void Insert(unsigned int , CArchPage *);
};
