#include "msgbox.h"
#include "fcntl.h"

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned long	u32;

typedef struct
{
	u16 Wd, Ht;          /* width of each char in _bytes_; height in rows */
	u16 First, Last;     /* codes (0-255) for first and last chars in font */
   u8 *Spacings, *Bitmaps; /* proportional spacing       bytes; glyph bitmaps (Wd x Ht bytes each) */
} font;

font *CurrFont;

unsigned char *lb;

unsigned ScnWd, ScnWdBytes, ScnHt;

/*****************************************************************************
text output to 256-color screen
****************************************************************************
void wrch8(u8 Char)
{	unsigned short CharWidth, Row, Dst;
	u8 *Src, BlitBuffer[512];

// valid Char?
	if((Char < CurrFont->First) || (Char > CurrFont->Last))
		return;
	Char -= CurrFont->First;
// proportional or monospaced font?
	if(CurrFont->Spacings == NULL)
		CharWidth=CurrFont->Wd * 8;
	else CharWidth=CurrFont->Spacings[Char];
// make sure char is not too wide for blit buffer
	if(CharWidth > 512)
		return;
// check cursor position
	if(CsrX + CharWidth >= ScnWd)
	{	CsrX=0;	// new line if too far right
		CsrY += CurrFont->Ht;
		if(CsrY + CurrFont->Ht > ScnHt)
// wrap to top left corner if too far down
			CsrY=0; }
// point to source bitmap, blit buffer, and screen
	Src=CurrFont->Bitmaps + CurrFont->Ht *
		CurrFont->Wd * Char;
	Dst=CsrX + CsrY * ScnWdBytes;

	for(Row=0; Row < CurrFont->Ht; Row++)
	{	u8 *Src2, Mask;
		unsigned Col;

// expand one row of pixels from monochrome font to 256-color blit buffer
		Src2=Src;
		Mask=0x80;
		for(Col=0; Col < CharWidth; Col++)
		{	if((*Src2 & Mask) != 0)
				BlitBuffer[Col]=Color;
			else BlitBuffer[Col]=0;
			Mask >>= 1;
			if(Mask == 0)
			{	Mask=0x80;
				Src2++; }}
// put it on screen
		vidmemwr(Dst, BlitBuffer, CharWidth);
// update pointers
		Src += CurrFont->Wd;
		Dst += ScnWdBytes; }
// update CsrX
	if(CurrFont->Spacings == NULL)
		CsrX += CharWidth;
	else CsrX=CsrX + CharWidth + 1; }

*/
#define	MAX_LINE_LEN	128
#define MAX_WD		4
#define	MAX_HT		32

static unsigned int pos = 0;
unsigned int filesize = 0;

int expect( int Infile, unsigned char *Line, unsigned char *Target)
{
	unsigned Len;

	Len=strlen(Target);
	do
	{
      if (IsEOF(Infile)) return-1;
	   sys_read(Infile, Line, MAX_LINE_LEN);
   }
	while(memcmp(Line, Target, Len) != 0);
	return(0);
}

int bdfLoadFont(font* Font, const char* FileName)
{
	unsigned char Line[MAX_LINE_LEN];
   int Infile;
   unsigned int readed;
	int FontYOff;

   msgbox(ID_OK, 1, "Load Font: %s",FileName);

   Infile = sys_open(FileName, O_RDONLY | O_TEXT);
   if (Infile < 0)
   {
     msgbox(ID_OK,1,"Error while opening file.");
     return 0;
   }

// look for STARTFONT
	if (expect(Infile, Line, "STARTFONT") != 0)
	{
      sys_close(Infile);
      msgbox(ID_OK,1,"String not found");
		return 0;
   }

// OK, it's a BDF font file: look for FONTBOUNDINGBOX
	if(expect(Infile, Line, "FONTBOUNDINGBOX") != 0)
ERR:{
		return(-1);
    }

	{	int FontWd, FontHt, FontXOff;

		sscanf(Line + 16, "%d %d %d %d",
			&FontWd, &FontHt, &FontXOff, &FontYOff);
// use FONTBOUNDINGBOX to set Font->Wd. Must be in range 1-MAX_WD (bytes)
		Font->Wd=(FontWd + 7) >> 3;
		if(Font->Wd < 1 || Font->Wd > MAX_WD)
		{	DEBUG(printf("bad Font->Wd (%u)\n", Font->Wd);)
			goto ERR; }
		DEBUG(printf("Font->Wd=%u, ", Font->Wd);) }

// look for PIXEL_SIZE
	if(expect(Infile, Line, "PIXEL_SIZE") != 0)
		goto ERR;
// use PIXEL_SIZE to set Font->Ht. Must be in range 1-MAX_HT
	Font->Ht=atoi(Line + 11);
	if(Font->Ht < 1 || Font->Ht > MAX_HT)
	{	DEBUG(printf("bad PIXEL_SIZE (%u)\n", Font->Ht);)
		goto ERR; }
	DEBUG(printf("Font->Ht=%u, ", Font->Ht);)
// look for "CHARS ". You need the space,
or it will trip on CHARSET_REGISTRY, etc.
	if(expect(Infile, Line, "CHARS ") != 0)
		goto ERR;

// CHARS sets size of Font->Spacings and Font->Bitmaps arrays
	{	unsigned NumChars;

		NumChars=256;//atoi(Line + 6);	xxx
		Font->Spacings=malloc(NumChars);
		if(Font->Spacings == NULL)
ERR2:		{	DEBUG(printf("out of memory\n");)
			fclose(Infile);
			return(ERR_MEM); }
		Font->Bitmaps=malloc(NumChars * Font->Ht * Font->Wd);
		if(Font->Bitmaps == NULL)
		{	free(Font->Spacings);
			goto ERR2; }
		DEBUG(printf("%u characters, ", NumChars);) }

// got all the info we need, move on to bitmap data
	Font->First=255;
	Font->Last=0;
// look for ENCODING
	while(expect(Infile, Line, "ENCODING") == 0)
	{	int CharXOff, CharYOff, CharWd, CharHt;
		int Encoding;

		Encoding=atoi(Line + 9);
		if(Encoding > 255)
ERR3:		{	DEBUG(printf("bad ENCODING (%u)\n", Encoding);)
			free(Font->Spacings);
			free(Font->Bitmaps);
			fclose(Infile);
			return(ERR_SUBFMT); }
		if(Encoding < Font->First)
			Font->First=Encoding;
		if(Encoding > Font->Last)
			Font->Last=Encoding;
// look for BBX, which gives the proportional spacing value (char width)
		if(expect(Infile, Line, "BBX") != 0)
			goto ERR3;
		sscanf(Line + 4, "%d %d %d %d",
			&CharWd, &CharHt, &CharXOff, &CharYOff);
		Font->Spacings[Encoding]=CharWd;
// look for BITMAP
		if(expect(Infile, Line, "BITMAP") != 0)
			goto ERR3;

// start reading the actual raster data
		{	u32 CharData[MAX_HT], RowData, OrOfAll, Mask;
			unsigned ShiftCount, Row, Col;
			u8 *Ptr;

			OrOfAll=0;
			memset(CharData, 0, sizeof(CharData));

			for(Row=0; Row < MAX_HT; Row++)
			{	unsigned YPos;

// xxx - unexpected EOF (ERR_READ); not ERR_SUBFMT
				if(fgets(Line, MAX_LINE_LEN, Infile) == NULL)
					goto ERR3;
// exit this loop when ENDCHAR seen
				if(memcmp(Line, "ENDCHAR", 7) == 0)
					break;
				sscanf(Line, "%lx", &RowData);
				YPos=Font->Ht - CharHt +
					FontYOff - CharYOff + Row;
				if(YPos >= 0 && YPos < MAX_HT)
					CharData[YPos]=RowData;
//else printf("warning: top or bottom edge of char might be clipped "
//"(YPos == %d)\n", YPos);
				OrOfAll |= RowData; }
// saw ENDCHAR: do some post-processing on the raster data
find out how far to shift left
			ShiftCount=0;
			for(Mask=0x80000000L; Mask != 0; Mask >>= 1)
			{	if(Mask & OrOfAll)
					break;
				ShiftCount++; }
			ShiftCount=8 * Font->Wd - (32 - ShiftCount);
// ShiftCount turns out to be always a multiple of 8
printf("ShiftCount=%u\n", ShiftCount);

// shift left so first non-white pixel is in leftmost column
			for(Row=0; Row < Font->Ht; Row++)
				CharData[Row] <<= ShiftCount;
// break into bytes; store at Font->Bitmaps
			Ptr=Font->Bitmaps + Font->Wd * Font->Ht * Encoding;
			for(Row=0; Row < Font->Ht; Row++)
			{	RowData=CharData[Row];
				for(Col=Font->Wd; Col != 0; Col--)
				{	Ptr[Col - 1]=RowData;
					RowData >>= 8; }
				Ptr += Font->Wd; }}}
Font->First=0; // xxx
	fclose(Infile);
	DEBUG(printf("OK\n");)
	return(0);
*/
}

void bdfload(void)
{
  font f;

  bdfLoadFont(&f, "//hd0a/tim08.bdf");
}

