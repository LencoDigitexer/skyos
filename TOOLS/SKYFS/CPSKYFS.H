#define MAX_TRY         5

#define SUCCESS         0
#define ERROR           1

#define DIRECTORY_TYPE  0
#define FILE_TYPE       1

struct skyfs_inode
{
  unsigned char name[50];
  unsigned char type;                   // DIRECTORY, FILE, LINK
  unsigned short owner;
  unsigned short rights;
  unsigned char attributes;
  unsigned char createtime[7];
  unsigned char accesstime[7];
  unsigned char updatetime[7];
  unsigned int size;
  unsigned int direct[8];
  unsigned int indirect[3];
  unsigned char reserved[3];
};
