#define MAX_DEV 30

class CDevice
{
public:
	char name[30];
	unsigned int thread_pid;
	int data;

	
	CDevice();
	void SetRegistrationName(char *str);
	char *GetRegistrationName(void);

	void SetPID(unsigned int pid);

	unsigned int GetPID(void);
	int GetData(void);

	virtual void Thread(void);

    virtual unsigned int Initialize();

	virtual unsigned int ReadBlock(unsigned int blocknr, char *buffer) {};
	virtual unsigned int WriteBlock(unsigned int blocknr, char *buffer) {};
};

class CBlockDevice : public CDevice
{
public:
	CBlockDevice();

};



class CVDM
{
private:
	char device_names[MAX_DEV][255];
	int device_minor[MAX_DEV];
	CDevice *device_objects[MAX_DEV];


public:
	CVDM();

	CDevice *registered_devices[MAX_DEV];


	unsigned int Initialize(void);
	unsigned int RegisterDevice(CDevice*);
	
	unsigned int ReadFromDevice(struct ipc_message *m);
	unsigned int WriteToDevice(struct ipc_message *m);
	

    unsigned int AddName(CDevice *dev, char *name, int data);
    
	CDevice*     GetDeviceFromName(char *str);
	CDevice*     GetRegistrationDevice(char *str);


	void ShowRegisteredDevices(void);

	void Thread(void);


};
