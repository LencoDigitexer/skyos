///////////////////////////////////////////////////////////
//                                                       //
//  PC MAGAZIN - Demo Projekt                            //
//                                                       //
//  Hier sind die Routinen untergebracht, die es         //
//  ermöglichen, ein Windows BMP File zu laden           //
//                                                       //
///////////////////////////////////////////////////////////

#include "bmp.h"
#include "newgui.h"
#include "controls.h"
#include "sched.h"
#include "fcntl.h"

// This structs are copied from Windows. (Source: Visual C++ 5.0 Help/MSDN)

#define PACKED __attribute__((packed))

typedef struct tagBITMAPFILEHEADER
{
     short    bfType             PACKED;
     int   bfSize                PACKED;
     short    bfReserved1        PACKED;
     short    bfReserved2        PACKED;
     int   bfOffBits             PACKED;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
     int  biSize                 PACKED;
     int   biWidth               PACKED;
     int   biHeight              PACKED;
     short   biPlanes            PACKED;
     short   biBitCount          PACKED;
     int  biCompression          PACKED;
     int  biSizeImage            PACKED;
     int   biXPelsPerMeter       PACKED;
     int   biYPelsPerMeter       PACKED;
     int  biClrUsed              PACKED;
     int  biClrImportant         PACKED;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD
{
    unsigned char rgbBlue     PACKED;
    unsigned char rgbGreen    PACKED;
    unsigned char rgbRed      PACKED;
    unsigned char rgbReserved PACKED;
} RGBQUAD;



int bmp_load( char *name, bitmaptype *bitmap)
{
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    int fd;
    int i;
    int b, g ,r;


    memset(&bitmap, 0, sizeof(bitmaptype));

    show_msgf("bmp.c: load file %s",name);

    fd = sys_open(name,O_RDONLY|O_BINARY);
    show_msgf("bmp.c: fd = %d",fd);

    if (fd < 0)
          return BMP_ERROR_FILEOPEN;

    // BMP Headers laden

    sys_read(fd,&bfh,sizeof(bfh));
    sys_read(fd,&bih,sizeof(bih));

    // prüfe Kompression und Format
    if ( ( bih.biPlanes != 1 )          ||
//         ( bih.biCompression != BI_RGB) ||
         ( bih.biBitCount < 8)          ||
         ( bfh.bfType != 0x4d42 ) )
    {
        sys_close( fd );
        return BMP_ERROR_FORMAT;
    }

    // bereite Header vor
    bitmap->width        = bih.biWidth;
    bitmap->height       = bih.biHeight;
    bitmap->bytesperline =
        ( (bih.biWidth * bih.biBitCount + 31) >> 5 ) << 2;
    bitmap->pixelbits    = bih.biBitCount;

    // Farb-Tabelle laden und konvertieren
    if ( ( bih.biClrUsed == 0 ) && 
         ( bih.biBitCount == 8 ) )
        bih.biClrUsed = 256;
    
    if ( bih.biClrUsed )
    {
        bitmap->c.lColors = (long *)valloc( 4 * 256 );
        
        sys_read(fd,bitmap->c.lColors,
           bih.biClrUsed * sizeof( RGBQUAD ) );

        // Umwandeln von BGR nach RGB
        for ( i = 0; i < 256; i++ )
        {
            b = bitmap->c.cColors[ i * 4 ];
            g = bitmap->c.cColors[ i * 4 + 1 ];
            r = bitmap->c.cColors[ i * 4 + 2 ];
            bitmap->c.lColors[ i ] = r | (g<<8) | (b<<16);
        }
    }

    // das Vorzeichen der height gibt an, ob das Bitmap
    // topdown oder bottomup gespeichert ist.. Im Speicher
    // wird es immer topdown sein.
    bitmap->b.cBitmap = (unsigned char *)
        valloc( bitmap->height * bitmap->bytesperline );
    
    if ( bih.biHeight < 0 )
    {
        for ( i = 0; i < -bih.biHeight; i++ )
          sys_read(fd,
          bitmap->b.cBitmap + i*bitmap->bytesperline,
                    bitmap->bytesperline );
    }
    else 
    {
        for ( i = bih.biHeight - 1; i >= 0; i--)

          sys_read( fd,bitmap->b.cBitmap + i*bitmap->bytesperline,
            bitmap->bytesperline );
    }

    sys_close( fd );
    
    return BMP_NOERROR;
}


// Freigeben des Speichers
void bmp_free( bitmaptype *bitmap )
{
//    if ( bitmap.cBitmap ) free( bitmap.cBitmap );
//    if ( bitmap.cColors ) free( bitmap.cColors );
//    ZeroMemory( &bitmap, sizeof( bitmaptype ) );
}


// Umwandeln in 16 Bit Farben
/*void bmp_make16bitpalette( bitmaptype &bitmap )
{
    if ( bitmap.cColors )
    {
        short *palette = (short *)malloc( 256 * 2 );
        for ( int i = 0; i < 256; i++ )
        {
            int r = bitmap.cColors[ i * 4 ];
            int g = bitmap.cColors[ i * 4 + 1 ];
            int b = bitmap.cColors[ i * 4 + 2 ];
            palette[i] = Rtab[ r ] |
                         Gtab[ g ] |
                         Btab[ b ];
        }
        free( bitmap.cColors );
        bitmap.sColors = palette;
    }
} */
void bmp_load_coltab(bitmaptype *bitmap)
{
  int i;
  static int ver = 0;

  outportb(0x3c8,0);

  for (i=0 ; i<256*3 ; i+=3)
  {
//    if ((coltab->color[i].red == 0) || (coltab->color[i].green == 0) || (coltab->color[i].blue == 0))
//      show_msgf("red: %d  green: %d  blue: %d",coltab->color[i].red,coltab->color[i].green,coltab->color[i].blue);

    if (ver == 0)
    {
     outportb(0x3c9, bitmap->c.cColors[i] >> 2);
     outportb(0x3c9, bitmap->c.cColors[i+1] >> 2);
     outportb(0x3c9, bitmap->c.cColors[i+2] >> 2);
    }
    else{
     outportb(0x3c9, bitmap->c.cColors[i] );
     outportb(0x3c9, bitmap->c.cColors[i+1] );
     outportb(0x3c9, bitmap->c.cColors[i+2] );
   }
  }
  ver++;
}

void bmpviewer_redraw(window_t *win, bitmaptype *bitmap, int loaded)
{
  int i,j;

  if (loaded)
  {
    for (j=0;j<bitmap->height;j++)
      for (i=0;i<bitmap->width;i++)
        win_draw_pixel(win,i,70+j,bitmap->b.cBitmap[j*bitmap->width+i]);
  }
}

void bmpviewer_task()
{
  bitmaptype b;
  struct ipc_message m;
  ctl_button_t *but1, *but2, *but3;
  ctl_input_t *input;
  char buf[256];
  window_t *win_bmpviewer;
  int fd, loaded=0;
  int ret;

  win_bmpviewer = create_window("BMP Viewer",300,100,400,300,WF_STANDARD);

  but1 = create_button(win_bmpviewer,20,20,60,25,"Load",1);
  but2 = create_button(win_bmpviewer,100,20,50,25,"Pal",2);
  but3 = create_button(win_bmpviewer,170,20,50,25,"Std",3);
  input = create_input(win_bmpviewer,250,20,100,20,4);

  fd = -1;

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_GUI_CTL_BUTTON_PRESSED:
            if (m.MSGT_GUI_CTL_ID == 1)
            {
              int i,j;

              if (fd < 0)
                sys_close(fd);

              GetItemText_input(win_bmpviewer, input, buf);
              show_msgf("Loading file: %s",buf);

              ret = bmp_load(buf,&b);
              if (ret == 0)
                show_msgf("bmp.c: BMP File OK!");
              else
                show_msgf("bmp.c: BMP File not ok, ret = %d",ret);

//              load_TGA(win_bmpviewer, fd, &tgafile);
              loaded = 1;
              bmpviewer_redraw(win_bmpviewer,&b,loaded);
            }
            if (m.MSGT_GUI_CTL_ID == 2)
              bmp_load_coltab(&b);
            if (m.MSGT_GUI_CTL_ID == 3)
              std_pal();
            break;
       case MSG_GUI_WINDOW_CLOSE:
            sys_close(fd);
            destroy_app(win_bmpviewer);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_bmpviewer, WF_STOP_OUTPUT, 0);
            set_wflag(win_bmpviewer, WF_MSG_GUI_REDRAW, 0);
            bmpviewer_redraw(win_bmpviewer,&b, loaded);
            break;
     }

  }

}

void bmpviewer_init_app()
{
//  if (!valid_app(win_imgview))
    CreateKernelTask(bmpviewer_task,"bmpviewer",NP,0);
}

