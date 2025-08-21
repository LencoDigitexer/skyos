///////////////////////////////////////////////////////////
//                                                       //
//  PC MAGAZIN - Demo Projekt                            //
//                                                       //
//  Headerfile für die BMP Routinen                      //
//                                                       //
///////////////////////////////////////////////////////////
// Fehler die beim Laden auftreten können
#define BMP_NOERROR         0
#define BMP_ERROR_FILEOPEN -1
#define BMP_ERROR_FORMAT   -2

typedef struct s_bitmaptype
{
    int width;         // Breite in Pixel
    int height;        // Höhe in Pixel
    int pixelbits;     // Bits pro Pixel
    int bytesperline;  // Bytes pro Scanline

    union
    {
        unsigned char   *cBitmap;  
        short           *sBitmap;  
        long            *lBitmap;  
    } b;
    // pointer zu den paletten (falls vorhanden)
    union
    {
        unsigned char   *cColors;
        short           *sColors;  
        long            *lColors;
    } c;
} bitmaptype;
