/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1998 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : fs\sub\msdos\msdos.c
/* Last Update: 07.12.1998
/* Version    : alpha
/* Coded by   : Szeleney Robert
/* Docus from : SKY V1.0, PC Intern 5
/************************************************************************/
/* Definition:
/*   The MSDOS - Sub Filesystem                                         */
/*   Supports: FAT12/16
/*
/* To Do:
/*   - Support of FAT32
/*   - Write/Create/Delete/Modify
/* Updates:
    03.02.00   FAT16 works now with subdirs
    03.02.00   binary/text read support
/************************************************************************/
#include "devices.h"
#include "rtc.h"
#include "error.h"
#include "ctype.h"
#include "sched.h"
#include "vfs.h"
#include "msgbox.h"
#include "fcntl.h"

#define get_unaligned(ptr) (*(ptr))
#define EOF 0

extern struct task_struct *current;
// enable this for real time debugging
static int debug = 1;

int msdos_open(struct ipc_message *m);


void msdos_task()
{
  int ret;
  int task;
  unsigned int from;
  unsigned char str[255];
  struct ipc_message *m;

  m = (struct ipc_message *)valloc(sizeof(struct ipc_message));

  while (1)
  {
     wait_msg(m, -1);

     switch (m->type)
     {
        case MSG_MOUNT:
             {
               m->MSGT_MOUNT_RESULT = msdos_mount(m);
               m->type = MSG_MOUNT_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_OPEN:
             {
               printk("msdos.c: Open ...");
               m->MSGT_OPEN_RESULT = msdos_open(m);
               m->type = MSG_OPEN_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_FIND:
             {
               printk("msdos.c: find ...");
               m->MSGT_FIND_RESULT = msdos_find_inode(m);
               m->type = MSG_FIND_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_GETINODE:
             {
               printk("msdos.c: getinode ...");
               m->MSGT_GETINODE_RESULT = msdos_get_inode(m);
               m->type = MSG_GETINODE_REPLY;
               send_msg(m->sender, m);
               break;
             }
/*        case MSG_LOOKUP:
             {
               m->MSGT_LOOKUP_RESULT = skyfs_lookup(m);
               m->type = MSG_REPLY;
               send_msg(m->sender, m);
               break;
             }*/
        case MSG_READ:
             {
               m->MSGT_READ_RESULT = msdos_read(m);
               m->type = MSG_READ_REPLY;
               send_msg(m->sender, m);
               break;
             }
     }
  }
}

int msdos_init()
{
   int pid;

   printk("msdos.c: Initializing FAT12/16 filesystem.");
   printk("msdos.c: FAT12/16 fsdd by Szeleney Robert (c) 1999");
   printk("msdos.c: Version 0.8a. Read only.");
   pid = ((struct task_struct*)CreateKernelTask((unsigned int)msdos_task, "msdos", HP,0))->pid;

   register_protocol("fat",pid);

   return SUCCESS;
}

static int get_name_segment(unsigned char *name, unsigned char *seg)
{
  int i=0,j=0;
  unsigned char *p;

  if (name[0] == '\0') return 0;

  while ((name[i] != '\0') && (name[i] != '/'))
  {
    seg[j] = name[i];
    j++;
    i++;
    if (j>=255) return 0;
  }

  seg[j] = '\0';
  if (name[i] == '\0')
     return 2;

  i++;
  j=0;
  while (name[i] != '\0')
  {
    name[j] = name[i];
    i++;
    j++;
  }

  name[j] = '\0';
  return 1;
}

static unsigned int clust2block(struct fat_superblock *sb, int clust)
{
  int block;

  block = clust - 2;
  block *= sb->cluster_size;
  block += sb->begindata;

  return block;
}



int msdos_read_fat(unsigned char *device, struct fat_superblock *sb)
{
  int i;

  if (sb->fat != NULL)
    vfree(sb->fat);

  sb->fat = (unsigned char*)valloc(sb->fat_length * 512);

  for (i=0; i < sb->fat_length; i++)
     readblock(device, sb->beginfat + i, sb->fat + i*512);

  printk("msdos.c: fat-cache updated.");

}


int msdos_mount(struct ipc_message *m)
{
  unsigned char buffer[512];
  unsigned char *device = m->MSGT_MOUNT_DEVICE;
  struct fat_dir_entry *entry;
  int sectors;

  struct fat_boot_sector *fat_bootsector;
  struct superblock *sb;
  struct fat_superblock *fat_sb;
  struct inode *i = (struct inode*)m->MSGT_MOUNT_ROOTINODE;

  if (debug)
    printk("msdos.c: mount: device = %s",device);

  if (readblock(device, 1, buffer))
  {
    show_msgf("msdos.c: Unable to read bootsector of device %s",device);
    return ERROR_READ_BLOCK;
  }

  fat_bootsector = (struct fat_boot_sector*)buffer;

  sb = &i->sb;
  entry = &(i->u.fat_dir_entry);
  fat_sb = &(sb->u.fat_sb);

//  if (fat_bootsector->fats        == 0) return ERROR_WRONG_PROTOCOL;
//  if (fat_bootsector->dir_entries == 0) return ERROR_WRONG_PROTOCOL;
  if (fat_bootsector->secs_track  == 0) return ERROR_WRONG_PROTOCOL;

  fat_sb->beginfat  = fat_bootsector->reserved + 1;

  // floppy or disk?
  sectors = fat_bootsector->sectors;
  if (sectors == 0)
  {
     printk("msdos.c: Device is a harddisk");
     sectors = fat_bootsector->total_sect;
  }

   // Check which FAT type it is
  	if (!fat_bootsector->fat_length && fat_bootsector->fat32_length)
  	{
     struct fat_boot_fsinfo *fsinfo;
     unsigned int fsinfo_offset;

     printk("msdos.c: Mounting FAT32 Filesystem...");
     fat_sb->fat32 = 1;
     fat_sb->fat16 = 0;

     printk("Fats: %d",fat_bootsector->fats);
     printk("Fat32-Length: %d",fat_bootsector->fat32_length);
     printk("Fat Rootcluster: %d",fat_bootsector->root_cluster);

     fat_sb->begindata  = fat_sb->beginfat + fat_bootsector->fats *
                            fat_bootsector->fat32_length;
     fat_sb->begindir   = fat_bootsector->root_cluster+1;

    //	MSDOS_SB(sb)->fat_length= CF_LE_W(b->fat32_length)*sector_mult;
	//	MSDOS_SB(sb)->root_cluster = CF_LE_L(b->root_cluster);
		fsinfo_offset = fat_bootsector->info_sector + 0x1e0;
		if (fsinfo_offset + sizeof(struct fat_boot_fsinfo) > 512)
      {
			printk("msdos.c: mount() Bad fsinfo_offset");
		}
      else
         printk("Infosector: %d + 0x1e0 is %d",fat_bootsector->info_sector, fsinfo_offset);

   }
   else
   {
     fat_sb->fat32 = 0;
     if ((sectors / fat_bootsector->secs_track) >= 4096)
     {
       printk("msdos.c: Mounting FAT16 Filesystem...");
       fat_sb->fat16 = 1;
     }
     else
     {
       printk("msdos.c: Mounting FAT12 Filesystem...");
       fat_sb->fat16 = 0;
     }

     fat_sb->begindir  = fat_sb->beginfat + fat_bootsector->fats *
                         fat_bootsector->fat_length;
     fat_sb->begindata = fat_sb->begindir +
                       (get_unaligned((unsigned short *) &fat_bootsector->dir_entries) * 32 / 512);
   }

   entry->start = fat_sb->begindir;


//   fat_sb->dir_blocks = 1 + (fat_bootsector->dir_entries / 16);

   fat_sb->cluster_size = fat_bootsector->cluster_size;

   fat_sb->fat_length   = fat_bootsector->fat_length;

  msdos_read_fat(device, fat_sb);

  if (debug)
  {
   printk("msdos_mount: begindata = %d",fat_sb->begindata);
   printk("msdos_mount: begindir  = %d",fat_sb->begindir);
   printk("msdos_mount: beginfat = %d",fat_sb->beginfat);
   printk("msdos_mount: cluster size = %d",fat_sb->cluster_size);
   printk("msdos_mount: fat length = %d",fat_sb->fat_length);
   printk("msdos_mount: FAT Type = %s",
     (fat_sb->fat32 == 1)?"FAT32":((fat_sb->fat16 == 1)?"FAT16":"FAT12"));
  }

  //
//  fat_sb->begindir = entry->start = fat_sb->begindata;

  return SUCCESS;
}

/************************************************************************
/* read or write an entry in the file allocation table.
/************************************************************************
/* pos = offset pos in fat
/* (op == 1) ==> read and return entry
/********************************************************************/
unsigned int msdos_fat_io(unsigned char *device,struct fat_superblock *superblock, int pos)
{
  int offset;
  unsigned short sector = 0;
  int newpos = 0;

  offset = pos;

  if (superblock->fat16)
  {
      newpos = offset * 2;
//      mem_dump(superblock->fat + newpos, 20);
      memcpy(&sector, superblock->fat + newpos, 2);
//       msgbox(ID_OK,1,"FATIO: pos = %d, newpos = %d",pos,sector);
  }
  else
  {
    newpos = offset * 3 / 2;

    memcpy(&sector, superblock->fat  + newpos, 2);

    if (offset & 1)
    {
      sector = sector >> 4;
    }
    else
      sector = sector & 0xFFF;

  }
  if (superblock->fat16)
  {
     if (sector == 0xFFFF)
        return EOF;
  }
  else if (!superblock->fat16)
  {
      if (sector == 0xFFF)
        return EOF;
  }

//  if (debug)
//    printk("msdos.c: newclustor: %d",sector);

  return sector;
}

/**************************************************************************/
/* Returns the nr'th directory entry from a buffer
/* Return Value: Always true
/**************************************************************************/
/*int msdos_get_entry(struct fat_dir_entry *inode, int nr, unsigned char *buffer)
{
  int i=0,j=0;

  i=nr*32;

  if (!buffer[i]) return ERROR_FILE_NOT_FOUND;

  while ((i<nr*32+8) && (buffer[i]!=0x20)) inode->name[j++]=buffer[i++];

  i=nr*32+8;
  if (buffer[i]!=0x20) inode->name[j++]='.';

  while ((i<nr*32+11) && (buffer[i]!=0x20)) inode->name[j++]=buffer[i++];

  inode->name[j]='\0';

  i=nr*32;
//  if (buffer[i+11] & 16) inode->type=0;
//  else inode->type=1;

  memcpy(&inode->size, nr * 32 + buffer + 28, 4);

//  inode->direct[0] = (unsigned short)buffer[i+0x1A];

  return SUCCESS;
}


int msdos_get_inode(unsigned char *device, struct fat_superblock *sb, struct fat_dir_entry *entry, unsigned int number)
{
  unsigned char buffer[512];

  int block = sb->begindata;

  while (number>15)
  {
    block = msdos_fat_io(device,sb,block);
    block++;
    number=number-16;
  }

  if (readblock(device, block, buffer))
    return ERROR_READ_BLOCK;

  if (msdos_get_entry(entry, number, buffer)) return ERROR_FILE_NOT_FOUND;

//  if (inode->name[0] == 0xE5) inode->type=2;
//  if (inode->name[0] == 0x05) inode->name[0]=0xE5;

  /*  if (inode->direct[0]) inode->direct[0] = inode->direct[0] * superblock->SPF + superblock->begindata;
  else inode->direct[0] = superblock->begindir;*/

/*  return SUCCESS;
} */

int msdos_compare_name(struct fat_dir_entry *e , char*name)
{
  char str[13] = {0};
  int i=0;
  int j=0;
  char ch;

  while ((i<8) && (e->name[i] != ' '))
  {
     str[i] = e->name[i];
     i++;
  }

  if (e->ext[0] != ' ')
  {
    str[i] = '.';
    i++;
  }

  j = 0;
  while ((j<3) && (e->ext[j] != ' '))
  {
    str[i] = e->ext[j];
    i++;
    j++;
  }

  str[i] = '\0';

  for (i=0;i<strlen(name);i++)
     if ((name[i] >= 97) && (name[i] <= 122))
       name[i] -= 32;

  if (!strcmp(str, name)) return SUCCESS;

  return ERROR_FILE_NOT_FOUND;
}

int msdos_get_entry(unsigned char *buffer, struct fat_dir_entry* entry, unsigned char *name, int nr)
{
  struct fat_dir_entry *e;
  int i = 0;
  int found = 0;

  while (1)
  {
    e = (buffer) + i*32;

    // get nr'th dir entry
    if (nr>=0)
    {
       e = (buffer) + nr*32;
       memcpy(entry,e,sizeof(struct fat_dir_entry));
       if (e->name[0] == 0)
         return ERROR_FILE_NOT_FOUND;

       return SUCCESS;
    }

    // last dir entry?
    if (e->name[0] == 0)
    {
       memcpy(entry, e, sizeof (struct fat_dir_entry));
       return ERROR_FILE_NOT_FOUND;
    }

    if (e->name[0] == 0x05) e->name[0]= 0xE5;

    if (msdos_compare_name(e->name, name) == SUCCESS)
    {
//      printk("msdos_get_entry: MATCH for %.8s.%.3s with %s",e->name, e->ext,name);
      memcpy(entry, e, sizeof (struct fat_dir_entry));
      return SUCCESS;
    }
    else
    {
//       printk("msdos_get_entry: NO MATCH for %.8s.%.3s with %s",e->name, e->ext, name);
    }

    i++;

    if (i==16)
    {
       memcpy(entry, e, sizeof (struct fat_dir_entry));
       return ERROR_FILE_NOT_FOUND;
    }
  }
}

int msdos_lookup_inode(unsigned char *device, struct fat_superblock *sb, struct fat_dir_entry *start,struct fat_dir_entry *entry, unsigned char* name)
{
  unsigned char buffer[512] = {0};

  int lastclust = start->start;
  int block = 0;
  int number = 0;
  int ret = ERROR_FILE_NOT_FOUND;
  int clust = 0;
  int block_in_clust = 0;


  if (start->start == sb->begindir)
    block = sb->begindir;
  else block = clust2block(sb, start->start);

  clust = start->start;

/*  printk("I start = %d",start->start);
  printk("begindir = %d",sb->begindir);
  printk("block = %d",block);*/

  block_in_clust = 0;

  while (ret == ERROR_FILE_NOT_FOUND)
  {
    readblock(device, block, buffer);

    ret = msdos_get_entry(buffer, entry, name,-1);

/*
    printk("start = %d",start->start);
    printk("begindir = %d",sb->begindir);
    printk("block = %d",block);*/

//    printk("ret = %d",ret);

//    current->state = TASK_SLEEPING;
//    while (1);
    if (ret == ERROR_FILE_NOT_FOUND)
    {
//      msgbox(ID_OK,MODAL,"Not found in %d. block",block);
      if (entry->name[0] == 0)
      {
//         printk("last in dir");
         return ERROR_FILE_NOT_FOUND;
      }
   }
/// updated 27.10.1999
   if (ret == SUCCESS) return SUCCESS;
///
   else
   {
        if (block <= sb->begindata)
        {
          block++;
//          msgbox(ID_OK,MODAL,"Block inc");
        }
        else
        {
           if (block_in_clust < (sb->cluster_size-1))  // if more block in cluster
           {
//              msgbox(ID_OK,MODAL,"Inc block in cluster");
              block++;
              block_in_clust++;
           }

           else
           {
//              msgbox(ID_OK,MODAL,"Get next cluster...");
              lastclust = clust;
              clust = msdos_fat_io(device, sb, lastclust);

              if (clust == EOF)
              {
//                msgbox(ID_OK,1,"clust == EOF");
                return ERROR_FILE_NOT_FOUND;
              }
              block = clust2block(sb, clust);
              block_in_clust = 0;
           }
        }
   }
  }

//  printk("!!! FILE FOUND !!!");
  return 2;
}

int msdos_fat_next(unsigned char *device, unsigned int clust, unsigned int block, struct fat_superblock *sb)
{
   unsigned int lastclust;
   unsigned int newclust;

     if (block <= sb->begindata)
        block++;
     else
     {
         lastclust = clust;
         newclust = msdos_fat_io(device, sb, lastclust);

         if (newclust == EOF)
         {
//            msgbox(ID_OK,1,"clust == EOF");
            return ERROR_FILE_NOT_FOUND;
         }
     }

  return newclust;
}

int msdos_get_inode(struct ipc_message *m)
{
  struct inode *lookinode = m->MSGT_GETINODE_INODE;
  struct inode *destinode = m->MSGT_GETINODE_ACTINODE;
  struct fat_dir_entry e;

  unsigned char *device = m->MSGT_GETINODE_DEVICE;
  struct inode *i = (struct inode*)m->MSGT_GETINODE_ROOTINODE;

  int ret = ERROR_FILE_NOT_FOUND;

  struct superblock *sb;
  struct fat_superblock *fat_sb;

  unsigned int first = m->MSGT_GETINODE_FIRST;
  unsigned int count = m->MSGT_GETINODE_COUNT;

  unsigned char buffer[512];

  unsigned int block;
  unsigned int clust;

  unsigned int entries;
  int newclust;

  sb = &i->sb;
  fat_sb = &sb->u.fat_sb;

  if ((lookinode->u.fat_dir_entry.start) == fat_sb->begindir)
    block = fat_sb->begindir;

  else block = clust2block(fat_sb, lookinode->u.fat_dir_entry.start);

  clust = lookinode->u.fat_dir_entry.start;

//  msgbox(ID_OK,MODAL,"Lookinode: %d",lookinode->u.fat_dir_entry.start);

  entries = 16 * fat_sb->cluster_size;

  while (first >= entries)
  {
     first-=entries;
     newclust = msdos_fat_next(device, clust,block, fat_sb);

     clust = newclust;
     block = clust2block(fat_sb, clust);
  }

  while (first >= 16)
  {
     block++;
     first-=16;
  }

  readblock(device, block, buffer);

  ret = msdos_get_entry(buffer, &e, NULL, first);

  memcpy(&(destinode->u.fat_dir_entry),&e,sizeof(struct fat_dir_entry));

  return ret;
}

// looks for a inode with the name m->MSGT_FIND_NAME and returns the
// vfs-inode
int msdos_find_inode(struct ipc_message *m)
{
  struct fat_boot_sector *fat_bootsector;
  struct inode *actinode;

  unsigned char name[1024];
  unsigned char seg[255];
  int ret = 0;
  unsigned last = 1;

  struct superblock *sb;
  struct fat_superblock *fat_sb;

  struct inode *lookinode = m->MSGT_FIND_INODE;
  unsigned char *device = m->MSGT_FIND_DEVICE;

  struct inode *i = (struct inode*)m->MSGT_FIND_ROOTINODE;
  struct inode *i2 = (struct inode*)m->MSGT_LOOKUP_INODE;

  actinode = valloc(sizeof(struct inode));
  memcpy(actinode, m->MSGT_FIND_ROOTINODE, sizeof(struct inode));

  sb = &i->sb;
  fat_sb = &sb->u.fat_sb;

  strcpy(name, m->MSGT_LOOKUP_NAME);

  while (last == 1)
  {
     last = get_name_segment(name, seg);

     if (last == 0) break;

//     printk("seg is");
//     printk("%s",seg);
//     msgbox(ID_OK, 1,"msdos.c: Searching for seg = %s",seg);
     ret = msdos_lookup_inode(device, fat_sb, &(actinode->u.fat_dir_entry),
                              &(lookinode->u.fat_dir_entry), seg);

     if (ret == ERROR_FILE_NOT_FOUND)
     {
        vfree(actinode);
        return ret;
     }
//     msgbox(ID_OK, 1,"msdos.c: seg found = %s, clust = %d",seg,lookinode->u.fat_dir_entry.start);

     if (last == 1)
     {
        memcpy(&(actinode->u.fat_dir_entry), &(lookinode->u.fat_dir_entry), sizeof(struct fat_dir_entry));

        if ((lookinode->u.fat_dir_entry.attr & ATTR_DIR) == 0)
        {
//           msgbox(ID_OK,1,"%8s is no dir.",lookinode->u.fat_dir_entry.name);
           vfree(actinode);
           return ERROR_FILE_NOT_FOUND;
        }
//        else  msgbox(ID_OK,1,"%8s is a dir.",lookinode->u.fat_dir_entry.name);
     }
  }
//  printk("file found");

  vfree(actinode);

  if (last == 2)        // last segment was found
     return SUCCESS;

  return ERROR_FILE_NOT_FOUND;
}

int msdos_open(struct ipc_message *m)
{
  struct fat_boot_sector *fat_bootsector;
  struct inode *actinode;

  unsigned char name[1024];
  unsigned char seg[255];
  int ret = 0;
  unsigned last = 1;

  struct inode *lookinode = m->MSGT_LOOKUP_INODE;
  struct superblock *sb;
  struct fat_superblock *fat_sb;
  unsigned char *device = m->MSGT_OPEN_DEVICE;

  struct inode *i = (struct inode*)m->MSGT_OPEN_ROOTINODE;
  struct inode *i2 = (struct inode*)m->MSGT_LOOKUP_INODE;

  actinode = valloc(sizeof(struct inode));
  memcpy(actinode, m->MSGT_OPEN_ROOTINODE, sizeof(struct inode));

  sb = &i->sb;
  fat_sb = &sb->u.fat_sb;

  strcpy(name, m->MSGT_LOOKUP_NAME);

//  printk("name is");
//  printk("%s",name);
  while (last == 1)
  {
     last = get_name_segment(name, seg);

     if (last == 0) break;

//     printk("seg is");
//     printk("%s",seg);
//     msgbox(ID_OK, 1,"msdos.c: Searching for seg = %s",seg);
     ret = msdos_lookup_inode(m->MSGT_OPEN_DEVICE, fat_sb, &(actinode->u.fat_dir_entry),
                              &(lookinode->u.fat_dir_entry), seg);
     if (ret == ERROR_FILE_NOT_FOUND)
        return ret;
//     printk("seg found");
//     msgbox(ID_OK, 1,"msdos.c: seg found = %s, clust = %d",seg,lookinode->u.fat_dir_entry.start);

     if (last == 1)
     {
        memcpy(&(actinode->u.fat_dir_entry), &(lookinode->u.fat_dir_entry), sizeof(struct fat_dir_entry));

        if ((lookinode->u.fat_dir_entry.attr & ATTR_DIR) == 0)
        {
//           msgbox(ID_OK,1,"%8s is no dir.",lookinode->u.fat_dir_entry.name);
            vfree(actinode);
            return ERROR_FILE_NOT_FOUND;
        }
//        else  msgbox(ID_OK,1,"%8s is a dir.",lookinode->u.fat_dir_entry.name);
     }


  }
//  printk("file found");

   vfree(actinode);

  if (last == 2)        // last segment was found
  {
     // check if file
     if (lookinode->u.fat_dir_entry.attr & ATTR_DIR)
     {
//       msgbox(ID_OK,1,"Is a dir");
       return ERROR_FILE_NOT_FOUND;
     }
     if (lookinode->u.fat_dir_entry.attr & ATTR_VOLUME)
     {
//       msgbox(ID_OK,1,"Is a dir");
       return ERROR_FILE_NOT_FOUND;
     }
  }


  if (last==2)  return lookinode->u.fat_dir_entry.size;

  return ERROR_FILE_NOT_FOUND;
}

/************************************************************************
  read a msdos file in binary or text mode
/************************************************************************/

int msdos_read(struct ipc_message *m)
{
  struct inode *vfs_i;        // This is the file we want to read from

  struct inode *vfs_i_root;   // Pointer to the root inode
  struct fat_dir_entry *e;

  unsigned int size = m->MSGT_READ_SIZE;           // how many bytes to read
  unsigned char *buffer_ptr = m->MSGT_READ_BUFFER; // buffer to store read bytes
  unsigned char *device = m->MSGT_READ_DEVICE;
  unsigned int mode = m->MSGT_OPEN_MODE;           // binary or text
  unsigned int binary = 0;

  unsigned int block;
  unsigned int offset;

  unsigned int lastclust = 0;
  unsigned int actclust = 0;
  unsigned int sum_read = 0;
  unsigned int read_bytes = 0;

  unsigned int i,j;

  unsigned char *buf;      // buffer to store a whole cluster


  unsigned int blocks_to_read;
  unsigned int count_max;
  int last = 0;

  vfs_i      = m->MSGT_READ_INODE;
  vfs_i_root = m->MSGT_READ_ROOTINODE;
  e = &(vfs_i->u.fat_dir_entry);

  if (m->MSGT_READ_MODE & O_BINARY)
    binary = 1;
  else binary = 0;


//  printk("reading file %.8s. First Cluster = %d, sectors = %d",
//         e->name, e->start, vfs_i_root->sb.u.fat_sb.cluster_size);

/*  buf = (unsigned char*)valloc(512 * vfs_i_root->sb.u.fat_sb.cluster_size);

  if (!buf)
  {
    msgbox(ID_OK,MODAL,"Critical Error: Unable to allocate buffer");
    return ERROR_FILE_NOT_FOUND;
  }*/

  // set to first cluster
  block = e->start;
  actclust = block;

  // seek to offset
  offset = *m->MSGT_READ_FP;
//  printk("cluster is %d",e->start);
//  printk("offset is %d",offset);

  // We always read all blocks, also in text mode
  count_max = (offset & (512-1)) + size;
  blocks_to_read = count_max / 512;
  if (count_max & (512-1)) blocks_to_read++;


//  msgbox(ID_OK,MODAL,"File is: %8s",vfs_i->u.fat_dir_entry.name);
//  msgbox(ID_OK,MODAL,"Reading %d blocks",blocks_to_read);
//  msgbox(ID_OK,MODAL,"Device is: %s ",device);

  // now seek to cluster were the current offset is in it

  while (offset >= vfs_i_root->sb.u.fat_sb.cluster_size*512)
  {
   //    printk("offset is %d",offset);
//    msgbox(ID_OK,MODAL,"msdos.c: read() get next cluster...");
    lastclust = block;

    block = msdos_fat_io(device,&(vfs_i_root->sb.u.fat_sb), lastclust);
    actclust = block;
    offset -= vfs_i_root->sb.u.fat_sb.cluster_size*512;
  }

  block -= 2;
  block *= vfs_i_root->sb.u.fat_sb.cluster_size;
  block += vfs_i_root->sb.u.fat_sb.begindata;

  do
  {
    // read cluster in buf
    if ((vfs_i->cache == NULL) || (vfs_i->cache_index != block))
    {
//      msgbox(ID_OK,MODAL,"Cluster not in cache");
      if (vfs_i->cache != NULL) vfree(vfs_i->cache);

      vfs_i->cache = (unsigned char*)valloc(vfs_i_root->sb.u.fat_sb.cluster_size * 512);

      for (i=0;i<vfs_i_root->sb.u.fat_sb.cluster_size;i++)
//        readblock(m->MSGT_READ_DEVICE, block+i, buf+i*512);
        readblock(m->MSGT_READ_DEVICE, block+i, (vfs_i->cache)+i*512);

      vfs_i->cache_index = block;
    }

    buf = vfs_i->cache;

    // how many bytes to read left in cluster?
    read_bytes = 512*vfs_i_root->sb.u.fat_sb.cluster_size - offset;

    // don't read more than read-size is
    if (read_bytes > size) read_bytes = size;

    if ((*m->MSGT_READ_FP + read_bytes) > (vfs_i->u.fat_dir_entry.size))
    {
//      msgbox(ID_OK,MODAL,"reading over file. adjusted.");
      last = 1;
      read_bytes = vfs_i->u.fat_dir_entry.size - *m->MSGT_READ_FP;
    }

//    msgbox(ID_OK,MODAL,"reading %d bytes",read_bytes);
    if (read_bytes == 0) return 0;

    if (binary)
    {
      memcpy(buffer_ptr, buf+offset, read_bytes);
      buffer_ptr += read_bytes;
      *m->MSGT_READ_FP += read_bytes;
      size -= read_bytes;
      sum_read += read_bytes;
      offset = 0;
    }

    else       // read in text mode
    {
//       msgbox(ID_OK,MODAL,"text mode, read_bytes = %d",read_bytes);

       for (i=0;i<read_bytes;i++)
       {
          if ((buf[offset +i] != 0x0D))
          {
//             msgbox(ID_OK,MODAL,"Char is %c",buf[offset+i]);
             *buffer_ptr = buf[offset+i];
             buffer_ptr++;
             (*m->MSGT_READ_FP)+=1;
             size --;
             sum_read ++;
          }
          else
          {
//             if ((buf[offset +i] == 0x0D) && (buf[offset+i+1] == 0x0A))
//             {
//                msgbox(ID_OK,MODAL,"ENTER found");
                *buffer_ptr = '\0';
                *m->MSGT_READ_FP+=2;
                if (size > 1) size-=2;
                if (size > 0) size--;
                last = 1;
                break;
//             }
          }
       }
       offset = 0;

//       msgbox(ID_OK,MODAL,"sum_read = %d",sum_read);
//       msgbox(ID_OK,MODAL,"size     = %d",size);
//       msgbox(ID_OK,MODAL,"fp       = %d",*m->MSGT_READ_FP);
    }

    if (*m->MSGT_READ_FP == vfs_i->u.fat_dir_entry.size) break;
    if (last) break;       // file completly read
    if (size == 0) break;

//    msgbox(ID_OK,MODAL,"read() continue...");

    lastclust = actclust;
    actclust = msdos_fat_io(device,&(vfs_i_root->sb.u.fat_sb), lastclust);

    if (actclust == EOF)
    {
       msgbox(ID_OK,MODAL,"Critical error: read(). FAT is damaged");
       last = 1;
    }
    else
    {
      block = actclust;
      block -= 2;
      block *= vfs_i_root->sb.u.fat_sb.cluster_size;
      block += vfs_i_root->sb.u.fat_sb.begindata;
    }

//    printk("msdos.c: %d bytes read.",read_bytes);
  } while ((sum_read < m->MSGT_READ_SIZE) && (!last));

  if (binary) return sum_read;
  else
  {
    if (sum_read==0) sum_read = 1;
    return sum_read;
  }
}

