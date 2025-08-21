#define MAX_CACHE_BLOCKS 100
struct cache_block
{
  int device;
  int sector;
  int count;
  unsigned char cache[512];
};

struct s_cache
{
   struct cache_block block[MAX_CACHE_BLOCKS];
};
