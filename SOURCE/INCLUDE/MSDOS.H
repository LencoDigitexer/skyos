#define PACKED __attribute__((packed))

#define ATTR_RO      1  /* read-only */
#define ATTR_HIDDEN  2  /* hidden */
#define ATTR_SYS     4  /* system */
#define ATTR_VOLUME  8  /* volume label */
#define ATTR_DIR     16 /* directory */
#define ATTR_ARCH    32 /* archived */

struct fat_boot_sector
{
	unsigned char	ignored[3] PACKED;	/* Boot strap short or near jump */
	unsigned char	system_id[8] PACKED;	/* Name - can be used to special case
				   partition manager volumes */
	unsigned short	sector_size PACKED ;	/* bytes per logical sector */
	unsigned char	cluster_size PACKED ;	/* sectors/cluster */
	unsigned short	reserved PACKED ;	/* reserved sectors */
	unsigned char	fats PACKED ;		/* number of FATs */
	unsigned char  dir_entries[2] PACKED ;	/* root directory entries */
	unsigned short	sectors PACKED ;	/* number of sectors */
	unsigned char	media PACKED ;		/* media code (unused) */
	unsigned short	fat_length PACKED ;	/* sectors/FAT */
	unsigned short	secs_track PACKED ;	/* sectors per track */
	unsigned short	heads PACKED ;		/* number of heads */
	unsigned int	hidden PACKED ;		/* hidden sectors (unused) */
	unsigned int	total_sect PACKED ;	/* number of sectors (if sectors == 0) */

	/* The following fields are only used by FAT32 */
	unsigned int	fat32_length;	/* sectors/FAT */
	unsigned short	flags;		/* bit 8: fat mirroring, low 4: active fat */
	unsigned char	version[2];	/* major, minor filesystem version */
	unsigned int	root_cluster;	/* first cluster in root directory */
	unsigned short	info_sector;	/* filesystem info sector */
	unsigned short	backup_boot;	/* backup boot sector */
	unsigned short	reserved2[6];	/* Unused */
};

struct fat_superblock
{
  unsigned char fat32;
  unsigned char fat16;
  unsigned int begindata;
  unsigned int beginfat;
  unsigned int begindir;
  unsigned int dir_blocks;
  unsigned int cluster_size;
  unsigned int fat_length;

  unsigned char *fat;

};

struct fat_dir_entry
{
	unsigned char	name[8],ext[3] PACKED ;	/* name and extension */
	unsigned char	attr PACKED ;		/* attribute bits */
	unsigned char    lcase PACKED ;		/* Case for base and extension */
	unsigned char	ctime_ms PACKED ;	/* Creation time, milliseconds */
	unsigned short	ctime PACKED ;		/* Creation time */
	unsigned short	cdate PACKED ;		/* Creation date */
	unsigned short	adate PACKED ;		/* Last access date */
	unsigned short   starthi PACKED ;	/* High 16 bits of cluster in FAT32 */
	unsigned short	time,date,start PACKED ;/* time, date and first cluster */
	unsigned int	size PACKED ;		/* file size (in bytes) */
};

struct fat_boot_fsinfo {
	unsigned int  reserved1  PACKED;	/* Nothing as far as I can tell */
	unsigned int   signature  PACKED;	/* 0x61417272L */
	unsigned int   free_clusters  PACKED;	/* Free cluster count.  -1 if unknown */
	unsigned int   next_cluster  PACKED;	/* Most recently allocated cluster.
				 * Unused under Linux. */
	unsigned int   reserved2[4]   PACKED;
};


