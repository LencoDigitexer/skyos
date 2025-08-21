struct tfs_boot_block
{
  unsigned char FileSystem[8];
  unsigned char reserved[512-8];
};

struct tfs_super_block
{
  int Magic;
  int IndexBlock;
  int FirstDataBlock;
  unsigned char reserved[512-4-4];
};

struct tfs_index
{
  unsigned char name[12];
  int pointerblock;
};

struct tfs_index_block
{
  struct tfs_index index[32];
};

struct tfs_inode
{
   int offset;
   struct tfs_index index;
};

