#include "ethernet.h"

struct s_net_buff_arp
{
   int need;                  // set if ARP needed
   unsigned int addr;         // protocol address for ARP
};

struct s_net_buff
{
  struct s_net_buff* next;

  struct s_net_device *dev;

  unsigned char* data;
  unsigned char* rowdata;
  unsigned int   data_length;

  unsigned int size;

  unsigned int pa_addr;

  struct s_net_buff_arp arp;

  struct s_ethhdr *eth;
  struct s_socket *socket;
  struct s_sockaddr_in *sa_daddr;

  unsigned int csum;

  unsigned int retransmittimer;
  unsigned int retransmits;
};

struct s_net_device
{
  unsigned char name[20];
  unsigned short ioaddr;
  unsigned short irq;

  unsigned char tx_lock;

  unsigned short if_port;
  unsigned int mode;            // 1 = loopback mode

  unsigned char ha_len;
  unsigned char pa_len;

  unsigned char dev_addr[7];


  unsigned int pa_addr;

  unsigned int pa_gateway;      // Standard GATEWAY IP Address

  struct enet_statistics stats;

  void *priv;

  int			  (*open)(struct s_net_device *dev);
  int			  (*stop)(struct s_net_device *dev);

  unsigned int major;           // major device pid for network device

};

