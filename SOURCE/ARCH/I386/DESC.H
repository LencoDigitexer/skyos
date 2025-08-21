class CArchDescriptor
{
  private:
    unsigned int data[2];

  public:
    CArchDescriptor();

    virtual void Load(unsigned int) {};
};

class CArchDescriptorCode : public CArchDescriptor
{
  private:
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;

  public:
    CArchDescriptorCode();
    CArchDescriptorCode(unsigned int, unsigned int, unsigned int);
    void Load(unsigned int);
};
