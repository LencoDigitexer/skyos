struct s_net_device
{
  unsigned short ioaddr;
  unsigned short irq;

  unsigned short if_port;

  unsigned char dev_addr[7];

  unsigned char tbusy;
};

