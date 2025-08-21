typedef struct {
    unsigned short mode_number;
    const unsigned char *regs;
} ModeTable;

/* Enter new modes here. (number + string) */

typedef struct {
  unsigned int x;
  unsigned int y;
  unsigned int depth;
} ModeInfo;


#define TEXT 	     0		/* Compatible with VGAlib v1.2 */
#define G320x200x16  1
#define G640x200x16  2
#define G640x350x16  3
#define G640x480x16  4
#define G320x200x256 5
#define G320x240x256 6
#define G320x400x256 7
#define G360x480x256 8
#define G640x480x2   9

#define G640x480x256 10
#define G800x600x256 11
#define G1024x768x256 12

#define G1280x1024x256 13	/* Additional modes. */

#define G320x200x32K 14
#define G320x200x64K 15
#define G320x200x16M 16
#define G640x480x32K 17
#define G640x480x64K 18
#define G640x480x16M 19
#define G800x600x32K 20
#define G800x600x64K 21
#define G800x600x16M 22
#define G1024x768x32K 23
#define G1024x768x64K 24
#define G1024x768x16M 25
#define G1280x1024x32K 26
#define G1280x1024x64K 27
#define G1280x1024x16M 28

#define G800x600x16 29
#define G1024x768x16 30
#define G1280x1024x16 31

#define G720x348x2 32		/* Hercules emulation mode */

#define G320x200x16M32 33	/* 32-bit per pixel modes. */
#define G640x480x16M32 34
#define G800x600x16M32 35
#define G1024x768x16M32 36
#define G1280x1024x16M32 37

/* additional resolutions */
#define G1152x864x16 38
#define G1152x864x256 39
#define G1152x864x32K 40
#define G1152x864x64K 41
#define G1152x864x16M 42
#define G1152x864x16M32 43

#define G1600x1200x16 44
#define G1600x1200x256 45
#define G1600x1200x32K 46
#define G1600x1200x64K 47
#define G1600x1200x16M 48
#define G1600x1200x16M32 49

#define OneModeEntry(res) {G##res,g##res##_regs}
#define END_OF_MODE_TABLE { 0, NULL }

