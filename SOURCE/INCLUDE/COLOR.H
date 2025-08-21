typedef struct RGB
{
   unsigned char r, g, b;
   unsigned char filler;
} RGB;

#define PAL_SIZE     256

typedef RGB PALETTE[PAL_SIZE];

extern RGB black_rgb;
extern PALETTE black_palette, _current_palette;

typedef struct {
   unsigned char data[32][32][32];
} RGB_MAP;

typedef struct {
   unsigned char data[PAL_SIZE][PAL_SIZE];
} COLOR_MAP;

