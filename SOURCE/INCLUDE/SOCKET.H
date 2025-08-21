#define SOCKET_CONNECTED  4
#define SOCKET_CONNECTING 8
#define SOCKET_W_SYN_ACK  16

struct s_sockaddr_in
{
  unsigned int sa_family;
  unsigned int sa_port;
  unsigned int sa_addr;
};

struct s_socket_packet
{
  struct s_sockaddr_in saddr;
  struct s_net_buff    *data;
};

struct s_socket
{
  unsigned int family;          // socketfamily (e.g. AF_INET, AF_IPX
  unsigned int type;            // type of socket (e.g. TCP, UDP)
  unsigned int port;            // port of socket (e.g. 23)

  unsigned int pa_addr;         // protocol address (e.g. 130.0.0.2)

  int receive_pid;              // pid of task who waits to receive
  unsigned int owner;           // pid of task who connects to socket

  unsigned int fd;              // file descriptor

  unsigned int state;

  struct list *data;            // socket_packets list of received data

  unsigned char *buffer;        // pointer to buffer

  struct s_sockaddr_in saddr;

  unsigned int seq;


};


