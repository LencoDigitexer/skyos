struct skyfs_superblock
{
 unsigned char jump[3];
 unsigned char filesystem[8];
 unsigned char mediatype;
 unsigned char sectorsize;
 unsigned char maxtrack;
 unsigned char maxhead;
 unsigned char maxsector;
 unsigned char begintrack;
 unsigned char beginhead;
 unsigned char beginsector;
 unsigned char endtrack;
 unsigned char endhead;
 unsigned char endsector;
 unsigned char blocksize;
 unsigned char bootcode[487];
 unsigned short id_code;
};
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