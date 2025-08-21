class CHardDisk : public CBlockDevice
{
public:
	CHardDisk();

	unsigned int Initialize(void);
	
	unsigned int ReadBlock(unsigned int blocknr, char *buffer);
	unsigned int WriteBlock(unsigned int, char *);

	void ide_wait_ns(int ns);
	int ata_Select(unsigned short port, unsigned char Sel);
	void ata_cmd(char CmdByte, unsigned short drive, unsigned int block, unsigned int count);

	int ata_read(unsigned short drive, unsigned int block, unsigned int count, unsigned short *buffer);
	void ata_identify(unsigned short drivenr);
	void ide_detect(char nrdrives);
	void ata_partProbe(void);
	void gafb(unsigned short *buffer, unsigned char *str, unsigned int start, unsigned int size);
	
	void Thread(void);
};
