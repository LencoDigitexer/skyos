#define MAX_REPLIES 7

class CDiskFloppy : public CBlockDevice
{
public:
	CDiskFloppy();

	unsigned int Initialize(void);
	
	unsigned int ReadBlock(unsigned int blocknr, char *buffer);
   unsigned int WriteBlock(unsigned int blocknr, char *buffer);
	unsigned int fdc_rw(unsigned int, char *, int);

	void block2hts(int block,int *head,int *track,int *sector);
	void resetcontroller(void);
	void recalibrate(void);
	int seek(int track);
	void prepare_motoroff(void);
	void motoroff(void);
    void motoron(void);

	int waitfdc(int sensei);
	int getbyte();
	int result(void);
	void output_byte( char byte);
    void dma_setup(int write);
    
	void Thread(void);
};
