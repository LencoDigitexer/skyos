/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : fs\sub\ext2fs.c
/* Last Update: 3.10.99
/* Version    : alpha           (very few testing!)
/* Coded by   : Max Sch„fer using code written by Szeleney Robert and Resl
		Christian
/* Docus from : the papers on ext2fs by Remy Card and Theodore Ts'o
/************************************************************************/
/* Definition:
/*   This file implements the support for the ext2fs filesystem
/*   Includes functions for mount, unmount (errr, strictly speaking ... no :-},
 *   open, close, read, ....
/*   Supported devices: all registered block devices
/************************************************************************/

/* Main problems with this code:
 *   - it is nearly untested
 *   - the algorithm in offset2block is absolutely unelegant
 *   - my coding style differs greatly from the skyos-gurus' one ;-)
 *   - write access should be implemented
 *   - most of the information inside the superblock and other control
 *     structures is flatly ignored
 *   - there are plenty of things which are bad ideas in some or the other
 *     way and I will try to embellish them in the future, but I don't have
 *     so much time...
 */

#include "sched.h"
#include "vfs.h"
#include "error.h"
//#include "types.h"

// uncomment the next line to enable in-time debug messages
#define EXT2FS_DEBUG 1

int ext2fs_pid;                                  // Process-ID

struct ext2fs_superblock sb;
unsigned char tmp_block_buffer[1024];

/************************************************************************/
/* EXT2FS task
/************************************************************************/
void ext2fs_task()
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
               m->MSGT_MOUNT_RESULT = ext2fs_mount(m);
               m->type = MSG_MOUNT_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_OPEN:
             {
               m->MSGT_OPEN_RESULT = ext2fs_open(m);
               m->type = MSG_OPEN_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_LOOKUP:
             {
               m->MSGT_LOOKUP_RESULT = ext2fs_lookup(m);
               m->type = MSG_REPLY;
               send_msg(m->sender, m);
               break;
             }
        case MSG_READ:
             {
               m->MSGT_READ_RESULT = ext2fs_read(m);
               m->type = MSG_READ_REPLY;
               send_msg(m->sender, m);
               break;
             }
     }
  }
}

/************************************************************************/
/* Initialize the ext2fs.
/* Module Init Entry Point.
/************************************************************************/
int ext2fs_init(void)
{
  printk("vfs.c: Initializing ext2fs...");
  ext2fs_pid = ((struct task_struct*)CreateKernelTask((unsigned int)ext2fs_task,
"ext2fs", HP,0))->pid;

  register_protocol("ext2fs",ext2fs_pid);

  return SUCCESS;
}

/************************************************************************/
/* Mount a device as ext2fs volume.
/************************************************************************/
int ext2fs_mount(struct ipc_message *m)
{
  struct inode *inode;
  // read superblock
  if(ext2fs_read_superblock(m->MSGT_MOUNT_DEVICE,  m->MSGT_MOUNT_SUPERBLOCK))
  	return ERROR_READ_BLOCK;
  // check if ext2fs superblock
  if(!ext2fs_is_valid_superblock(m->MSGT_MOUNT_SUPERBLOCK))
  	return ERROR_WRONG_PROTOCOL;
  // make a local copy
  memcpy(&sb, m->MSGT_MOUNT_SUPERBLOCK, sizeof(struct ext2fs_superblock));
  // read rootinode
  inode = m->MSGT_MOUNT_ROOTINODE;
////  if(ext2fs_read_inode(m->MSGT_MOUNT_DEVICE, 2, inode->u.ext2fs_inode))
  if(ext2fs_read_inode(m->MSGT_MOUNT_DEVICE, m->MSGT_MOUNT_SUPERBLOCK, &(inode->u.ext2fs_inode)))
  	return ERROR_READ_BLOCK;
  return SUCCESS;
}

int ext2fs_read_superblock(unsigned char *device, void *buffer)
{
  return ext2fs_readblock(device, 1, buffer);
}

int ext2fs_readblock(unsigned char *device, unsigned int block_no, void *buffer)
{
  // reads a 1024-byte block
  int res;
  res = readblock(device, block_no*2, buffer);
  if(res)
         return res;
  return readblock(device, block_no*2+1, &((unsigned char *)buffer)[512]);
}

int ext2fs_is_valid_superblock(void *superblock)
{
  struct ext2fs_superblock *sbp;
  sbp = (struct ext2fs_superblock *)superblock;
  if(sbp->magicno != 0xEF53)
    return 0;
  return 1;
}

int ext2fs_read_inode(unsigned char *device, unsigned int which, void *buffer)
{
        int group, offset, ret;
        unsigned int ino_tab, block, address;
        struct ext2fs_groupdesc gdp;
        unsigned int ino_per_group = sb.inodes_per_group;
        static char tmp[1024];
        group = (which-1) / ino_per_group;
	offset = (which-1) % ino_per_group;
	// now, get the start of the ino-table of the group our inode is in
        ret = ext2fs_read_groupdesc(device, group, &gdp);
        if(ret)
               return ERROR_READ_BLOCK;
        ino_tab = gdp.address_of_inode_table;
        block = ino_tab + ((offset * 128) / 1024);
        address = (offset * 128) % 1024;
        ret = ext2fs_readblock(device, block, tmp);
        if(ret == 0) {
       	       memcpy(buffer, &tmp[address], 128);
               return SUCCESS;
	}
        return ERROR_READ_BLOCK;
}

int ext2fs_read_groupdesc(unsigned char *device, int groupno,struct ext2fs_groupdesc *buffer)
{
  int gds_per_block = 1024 / sizeof(struct ext2fs_groupdesc);
  int block, offset;
  int ret;
  block = 1 + sb.first_data_block + groupno / gds_per_block;
  offset = (groupno % gds_per_block) * sizeof(struct ext2fs_groupdesc);
  ret = ext2fs_readblock(device, block, &tmp_block_buffer);
  if(ret == 0) {
	memcpy(buffer, (unsigned char *)((int)tmp_block_buffer+offset), sizeof(struct ext2fs_groupdesc));
        return SUCCESS;
  }
  return ERROR_READ_BLOCK;
}

int ext2fs_offset2block(unsigned char *device, struct ext2fs_inode *inode, unsigned int offset, unsigned int *block)
{
        static unsigned char tmp_block_buffer[1024];
	offset /= 1024;
        if(offset < 12) {
        	*block = inode->direct_blocks[offset];
                if(!*block) return ERROR_WRONG_OFFSET;
                return SUCCESS;
        }
        offset -= 12;
        if(offset < 256) {
        	unsigned int aux_block_addr;
                unsigned int *aux_block;
		aux_block_addr = inode->indirect_block;
                if(!aux_block_addr) return ERROR_WRONG_OFFSET;
                ext2fs_readblock(device, aux_block_addr, tmp_block_buffer);
                aux_block = (unsigned int *)tmp_block_buffer;
                // aux_block now contains the indirect block
                *block = aux_block[offset];
                if(!*block) return ERROR_WRONG_OFFSET;
                return SUCCESS;
        }
        offset -= 256;
        if(offset < 256*256) {		// in double indirect block
        	unsigned int aux_block_addr;
                unsigned int *aux_block;
		aux_block_addr = inode->double_indirect_block;
                if(!aux_block_addr) return ERROR_WRONG_OFFSET;
                ext2fs_readblock(device, aux_block_addr, tmp_block_buffer);
                aux_block = (unsigned int *)tmp_block_buffer;
                // aux_block now contains the double indirect block
                aux_block_addr = aux_block[offset/256];
                if(!aux_block_addr) return ERROR_WRONG_OFFSET;
                ext2fs_readblock(device, aux_block_addr, tmp_block_buffer);
                aux_block = (unsigned int *)tmp_block_buffer;
                offset %= 256;
                // aux_block now contains the indirect block
                aux_block_addr = aux_block[offset];
                if(!*block) return ERROR_WRONG_OFFSET;
                return SUCCESS;
	}
        offset -= 256*256;
        if(offset < 256*256*256) {	// in triple indirect block
        	unsigned int aux_block_addr;
                unsigned int *aux_block;
		aux_block_addr = inode->triple_indirect_block;
                if(!aux_block_addr) return ERROR_WRONG_OFFSET;
                ext2fs_readblock(device, aux_block_addr, tmp_block_buffer);
                aux_block = (unsigned int *)tmp_block_buffer;
                // aux_block now contains the triple indirect block
                aux_block_addr = aux_block[offset/(256*256)];
                if(!aux_block_addr) return ERROR_WRONG_OFFSET;
                ext2fs_readblock(device, aux_block_addr, tmp_block_buffer);
                aux_block = (unsigned int *)tmp_block_buffer;
                offset %= 256*256;
                // aux_block now contains the double indirect block
                aux_block_addr = aux_block[offset/256];
                if(!aux_block_addr) return ERROR_WRONG_OFFSET;
                ext2fs_readblock(device, aux_block_addr, tmp_block_buffer);
                aux_block = (unsigned int *)tmp_block_buffer;
                offset %= 256;
                // aux_block now contains the indirect block
                aux_block_addr = aux_block[offset];
                if(!*block) return ERROR_WRONG_OFFSET;
                return SUCCESS;
	}
        return ERROR_WRONG_OFFSET;
}

/************************************************************************/
/* Search for an Inode on a mounted device                               */
/************************************************************************/
int ext2fs_lookup(struct ipc_message *m)
{
/*
  m->MSGT_LOOKUP_ACTINODE	: our current inode
  m->MSGT_LOOKUP_DEVICE         : our current device
  m->MSGT_LOOKUP_NAME		: the name of the file to be looked up
  m->MSGT_LOOKUP_INODE		: the inode of the looked-up inode will be
  				  copied here
*/
  struct ext2fs_inode *cur_ino = &(((struct inode *)(m->MSGT_LOOKUP_ACTINODE))->u.ext2fs_inode);
  unsigned char *cur_dev = (unsigned char *)m->MSGT_LOOKUP_DEVICE;
  unsigned char *wanted_name = (unsigned char *)m->MSGT_LOOKUP_NAME;
  struct ext2fs_inode *wanted_ino = &(((struct inode *)(m->MSGT_LOOKUP_INODE))->u.ext2fs_inode);
  struct ext2fs_dirent dirent;
  int i, ret;
  unsigned int pos = 0;			// position inside cur_ino's data area
  unsigned int oldpos;
  while(1)
  {
    oldpos = pos;
    // already beyond end of directory?
    i = ext2fs_getchar(cur_dev, cur_ino, pos);
    if(i == 0)	// think so
      return ERROR_FILE_NOT_FOUND;
    // copy the constant entries
    for(i=0;i<(int)(2*sizeof(int));i++)
      ((unsigned char *)&dirent)[i] = ext2fs_getchar(cur_dev, cur_ino, pos++);
    // now look up length and copy rest
    for(i=0;i<dirent.name_length;i++)
      dirent.name[i] = ext2fs_getchar(cur_dev, cur_ino, pos++);
    dirent.name[i] = '\0';
    // test if we alreay found entry
    if(strcmp(wanted_name, dirent.name) == 0)
    {
      ret = ext2fs_read_inode(cur_dev, dirent.ino_number, wanted_ino);
      if(ret)
      	return ERROR_READ_BLOCK;
      return SUCCESS;
    }
    // skip over padding
    pos = oldpos + dirent.entry_length;
  }
  return ERROR_FILE_NOT_FOUND; // to keep the stupid (i.e. the compiler) happy
}

int ext2fs_open(struct ipc_message *m)
{
  return ext2fs_lookup(m);
}

int ext2fs_getchar(char *device, struct ext2fs_inode *ino, unsigned int pos)
{
  // only reads one character directly from an inode
  unsigned int block, offset;
  int ret;
  offset = pos % 1024;
  ret = ext2fs_offset2block(device, ino, pos, &block);
  if(ret)  return 0;
  ret = ext2fs_readblock(device, block, tmp_block_buffer);
  if(ret)
    return ERROR_READ_BLOCK;
  return tmp_block_buffer[offset];
}

/************************************************************************/
/* Read from an opened file on a skyfs mounted device                   */
/************************************************************************/
int ext2fs_read(struct ipc_message *m)
{
  struct inode *vfs_i;
  struct ext2fs_inode *inode;
  unsigned int size = m->MSGT_READ_SIZE;
  unsigned int i,j;
  unsigned int read_bytes;
  unsigned int sum_read = 0;
  unsigned char *buffer_ptr = m->MSGT_READ_BUFFER;
  unsigned char str[255];
  unsigned int block;
  int lastblock = 0;
  memset(tmp_block_buffer, 0, 1024);
  vfs_i = m->MSGT_READ_INODE;
  inode = &(vfs_i->u.ext2fs_inode);

  do
  {
    i = *m->MSGT_READ_FP / 1024;
    j = *m->MSGT_READ_FP % 1024;
    read_bytes = 1024 - j;
    if (read_bytes > size) read_bytes = size;
    if ((*m->MSGT_READ_FP + read_bytes) > inode->size)
    {
      lastblock = 1;
      read_bytes = inode->size - *m->MSGT_READ_FP;
    }

    if (ext2fs_offset2block(m->MSGT_READ_DEVICE, inode, *m->MSGT_READ_FP, &block))
return 0;

    if (ext2fs_readblock(m->MSGT_READ_DEVICE, block, tmp_block_buffer))    return
ERROR_READ_BLOCK;

    memcpy(buffer_ptr, tmp_block_buffer+j, read_bytes);

    buffer_ptr += read_bytes;
    *m->MSGT_READ_FP += read_bytes;
    size -= read_bytes;
    sum_read += read_bytes;
  } while ((sum_read < m->MSGT_READ_SIZE) && (!lastblock));
  return sum_read;
}
