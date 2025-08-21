struct registry_entry
{
  unsigned char key[10];
  unsigned char value[20];
};

struct local_system_info
{
   int magic;
   struct registry_entry entry[15];
};

