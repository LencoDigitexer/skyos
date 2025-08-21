/* Header file for the ext2fs subsystem */

#include "types.h"

#ifndef __EXT2FS_H__

#define __EXT2FS_H__

// function prototypes

void ext2fs_task();
int  ext2fs_init(void);
int  ext2fs_mount(struct ipc_message *m);
int  ext2fs_read_superblock(unsigned char *device, void *buffer);
int  ext2fs_readblock(unsigned char *device, unsigned int block_no, void *buffer);
int  ext2fs_is_valid_superblock(void *superblock);
int  ext2fs_read_inode(unsigned char *device, unsigned int which, void *buffer);
//int  ext2fs_read_groupdesc(unsigned char *device, int groupno,struct ext2fs_groupdesc *buffer);
//int  ext2fs_offset2block(unsigned char *device, struct ext2fs_inode *inode, unsigned int offset, unsigned int *block);
int  ext2fs_lookup(struct ipc_message *m);
int  ext2fs_open(struct ipc_message *m);
//int  ext2fs_getchar(char *device, struct ext2fs_inode *ino, unsigned int pos);
int  ext2fs_read(struct ipc_message *m);

// structures

struct ext2fs_inode {
	unsigned short mode;
       	unsigned short owner_uid;
        unsigned int size;
       	time_t atime;
        time_t ctime;
       	time_t mtime;
        time_t dtime;
       	unsigned short owner_gid;
        unsigned short link_count;
       	unsigned int num_of_used_blocks;
	unsigned int ino_flags;
        unsigned int reserved;
       	unsigned int direct_blocks[12];
        unsigned int indirect_block;
        unsigned int double_indirect_block;
        unsigned int triple_indirect_block;
        unsigned int version;
        unsigned int file_ACL;
        unsigned int dir_ACL;
	unsigned int frag_addr;
        unsigned char  frag_num;
       	unsigned char  frag_size;
        unsigned short padding;
       	unsigned int reserved2[2];
};

// the following is not used so far

#define EXT2_VALID_FS 0x01
#define EXT2_ERROR_FS 0x02

enum errbehav { EXT2_ERRORS_CONTINUE = 0x01, EXT2_ERRORS_RO,
		EXT2_ERRORS_PANIC };

enum osid { EXT2_OS_LINUX, EXT2_OS_HURD, EXT2_OS_MASIX, EXT2_OS_FREEBSD,
		EXT2_OS_LITES };

// but this is :-)

struct ext2fs_superblock {
	unsigned int num_of_inodes;
       	unsigned int num_of_blocks;
        unsigned int num_of_res_blocks;
       	unsigned int num_of_free_blocks;
        unsigned int num_of_free_inodes;
       	unsigned int first_data_block;
        unsigned int log_blocksize;
       	unsigned int log_fragsize;
        unsigned int blocks_per_group;
       	unsigned int frags_per_group;
        unsigned int inodes_per_group;
       	time_t last_mounted;
        time_t last_written;
       	unsigned short mnt_count;
        short max_mnt_count;
       	unsigned short magicno;
        unsigned short fs_state;
       	unsigned short errbehavior;
        unsigned short minor_rev_level;
       	time_t last_checked;
       	unsigned int check_interval;
        enum osid  creator_os;
        unsigned int major_rev_level;
       	unsigned short deflt_uid;
        unsigned short deflt_gid;
       	unsigned int first_avail_inode;
        unsigned short ino_size;
        unsigned short my_group;
       	unsigned int compat_feature_set;
        unsigned int incompat_feature_set;
       	unsigned int readonly_compat_feature_set;
       	unsigned char  uuid[16];
        char  vol_label[16];
        char  last_mountpoint[64];
       	unsigned int padding[206];
};


struct ext2fs_groupdesc {
       	unsigned int address_of_block_bitmap;
        unsigned int address_of_inode_bitmap;
        unsigned int address_of_inode_table;
        unsigned short num_of_free_blocks;
        unsigned short num_of_free_inodes;
        unsigned short num_of_dirs;
        unsigned short padding;
        unsigned int reserved;
};

struct ext2fs_dirent {
        unsigned int ino_number;
        unsigned short entry_length;
        unsigned short name_length;
        unsigned char  name[257];
};

#endif
