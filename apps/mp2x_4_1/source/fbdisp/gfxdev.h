#ifndef _GFXDEV_H
#define _GFXDEV_H
/*
 * Copyright (c) 1999, 2000 Greg Haerr <greg@censoft.com>
 *
 * Engine-level Screen, Mouse and Keyboard device driver API's and types
 * 
 * Contents of this file are not for general export
 *
 *                                     fixed by ghc: 2001. 2. 11
 */

/*
 * Type definitions
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/fb.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>


#include "gfxtype.h"
//#include "fontdisp.h"

#ifdef X86FB // 20050204
	#include <linux/kd.h>
	#include <linux/vt.h>
#endif	



#ifndef MWPIXEL_FORMAT
	#define MWPIXEL_FORMAT   MWPF_TRUECOLOR565
#endif


#if MWPIXEL_FORMAT == MWPF_TRUECOLOR565
	typedef unsigned short MWPIXELVAL;
#else
  	#if MWPIXEL_FORMAT == MWPF_TRUECOLOR332
  		typedef unsigned char MWPIXELVAL;
  	#else
    	#if MWPIXEL_FORMAT == MWPF_PALETTE
    		typedef unsigned char MWPIXELVAL;
    	#else
      		typedef unsigned long MWPIXELVAL;
    	#endif
  	#endif
#endif


 



typedef struct _mwscreendevice *PSD;

/*
 * Interface to Screen Device Driver
 * This structure is also allocated for memory (offscreen) drawing and blitting.
 */
typedef struct _mwscreendevice {
   int       fbfd;      /* ghcstop add: 20040902 mmsp2 fb ioctl test */ 
   	
   MWCOORD   xres;      /* X screen res (real) */
   MWCOORD   yres;      /* Y screen res (real) */
   MWCOORD   xvirtres;  /* X drawing res (will be flipped in portrait mode) */
   MWCOORD   yvirtres;  /* Y drawing res (will be flipped in portrait mode) */
   int       planes;    /* # planes*/
   int       bpp;       /* # bpp*/
   int       linelen;   /* line length in bytes for bpp 1,2,4,8*/
                        /* line length in pixels for bpp 16, 24, 32*/
   int       size;      /* size of memory allocated*/
   long      ncolors;   /* # screen colors*/
   int       pixtype;   /* format of pixel value*/
   int       flags;     /* device flags*/
   void     *addr;      /* address of memory allocated (memdc or fb)*/

   PSD        (*Open)(PSD psd);  
   void       (*Close)(PSD psd); 
   
   int        (*Init)              (PSD psd);
   void       (*DrawPixel)         (PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c);
   MWPIXELVAL (*ReadPixel)         (PSD psd, MWCOORD x, MWCOORD y);
   void       (*DrawHorzLine)      (PSD psd, MWCOORD x, MWCOORD x2, MWCOORD y,  MWPIXELVAL c);
   void       (*DrawVertLine)      (PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c);
   void       (*FillRect)          (PSD psd, MWCOORD x, MWCOORD y,  MWCOORD w, MWCOORD h, MWPIXELVAL c);

   int xoffset;
   int yoffset;
   int planew;
   int planeh;
   
} SCREENDEVICE;


	#define gfx_draw_pixel(psd, x, y, c)	   ((*(psd)->DrawPixel)(psd, x, y, c))
	#define gfx_draw_h_line(psd, x, x2, y, c)  ((*(psd)->DrawHorzLine)(psd, x, x2, y, c))
	#define gfx_draw_v_line(psd, x, y, y2, c)  ((*(psd)->DrawVertLine)(psd, x, y, y2, c))
	#define gfx_draw_pointed_rect(psd, x, y, x1, y1, c)  ((*(psd)->FillRect)(psd, x, y, x1, y1, c)) // ghcstop: see below

/* Truecolor color conversion and extraction macros*/
/*
 * Conversion from RGB to MWPIXELVAL
 */
/* create 24 bit 8/8/8 format pixel (0x00RRGGBB) from RGB triplet*/
#define RGB2PIXEL888(r,g,b)  (((r) << 16) | ((g) << 8) | (b))

/* create 16 bit 5/6/5 format pixel from RGB triplet */
#define RGB2PIXEL565(r,g,b)  ((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3))

/* create 8 bit 3/3/2 format pixel from RGB triplet*/
#define RGB2PIXEL332(r,g,b)  (((r) & 0xe0) | (((g) & 0xe0) >> 3) | (((b) & 0xc0) >> 6))

/*
 * Conversion from MWCOLORVAL to MWPIXELVAL
 */
/* create 24 bit 8/8/8 format pixel from RGB colorval (0x00BBGGRR)*/
#define COLOR2PIXEL888(c)   ((((c) & 0xff) << 16) | ((c) & 0xff00) | (((c) & 0xff0000) >> 16))

/* create 16 bit 5/6/5 format pixel from RGB colorval (0x00BBGGRR)*/
#define COLOR2PIXEL565(c)   ((((c) & 0xf8) << 8) | (((c) & 0xfc00) >> 5) | (((c) & 0xf80000) >> 19))

/* create 8 bit 3/3/2 format pixel from RGB colorval (0x00BBGGRR)*/
#define COLOR2PIXEL332(c)   (((c) & 0xe0) | (((c) & 0xe000) >> 11) | (((c) & 0xc00000) >> 22))

/*
 * Conversion from MWPIXELVAL to red, green or blue components
 */
/* return 8/8/8 bit r, g or b component of 24 bit pixelval*/
#define PIXEL888RED(pixelval)      (((pixelval) >> 16) & 0xff)
#define PIXEL888GREEN(pixelval)      (((pixelval) >> 8) & 0xff)
#define PIXEL888BLUE(pixelval)      ((pixelval) & 0xff)

/* return 5/6/5 bit r, g or b component of 16 bit pixelval*/
#define PIXEL565RED(pixelval)      (((pixelval) >> 11) & 0x1f)
#define PIXEL565GREEN(pixelval)      (((pixelval) >> 5) & 0x3f)
#define PIXEL565BLUE(pixelval)      ((pixelval) & 0x1f)

/* return 3/3/2 bit r, g or b component of 8 bit pixelval*/
#define PIXEL332RED(pixelval)      (((pixelval) >> 5) & 0x07)
#define PIXEL332GREEN(pixelval)      (((pixelval) >> 2) & 0x07)
#define PIXEL332BLUE(pixelval)      ((pixelval) & 0x03)

/*
 * Conversion from MWPIXELVAL to MWCOLORVAL
 */
/* create RGB colorval (0x00BBGGRR) from 8/8/8 format pixel*/
#define PIXEL888TOCOLORVAL(p)   ((((p) & 0xff0000) >> 16) | ((p) & 0xff00) | (((p) & 0xff) << 16))

/* create RGB colorval (0x00BBGGRR) from 5/6/5 format pixel*/
#define PIXEL565TOCOLORVAL(p)   ((((p) & 0xf800) >> 11) | (((p) & 0x07e0) << 3) | (((p) & 0x1f) << 16))

/* create RGB colorval (0x00BBGGRR) from 3/3/2 format pixel*/
#define PIXEL332TOCOLORVAL(p)   ((((p) & 0xe0) >> 5) | (((p) & 0x18) << 5) | (((p) & 0x03) << 16))

#if (MWPIXEL_FORMAT == MWPF_TRUECOLOR888) || (MWPIXEL_FORMAT == MWPF_TRUECOLOR0888)
#define RGB2PIXEL(r,g,b)   RGB2PIXEL888(r,g,b)
#define COLORVALTOPIXELVAL(c)   COLOR2PIXEL888(c)
#define PIXELVALTOCOLORVAL(p)   PIXEL888TOCOLORVAL(p)
#define PIXEL2RED(p)      PIXEL888RED(p)
#define PIXEL2GREEN(p)      PIXEL888GREEN(p)
#define PIXEL2BLUE(p)      PIXEL888BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR565
#define RGB2PIXEL(r,g,b)   RGB2PIXEL565(r,g,b)
#define COLORVALTOPIXELVAL(c)   COLOR2PIXEL565(c)
#define PIXELVALTOCOLORVAL(p)   PIXEL565TOCOLORVAL(p)
#define PIXEL2RED(p)      PIXEL565RED(p)
#define PIXEL2GREEN(p)      PIXEL565GREEN(p)
#define PIXEL2BLUE(p)      PIXEL565BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR332
#define RGB2PIXEL(r,g,b)   RGB2PIXEL332(r,g,b)
#define COLORVALTOPIXELVAL(c)   COLOR2PIXEL332(c)
#define PIXELVALTOCOLORVAL(p)   PIXEL332TOCOLORVAL(p)
#define PIXEL2RED(p)      PIXEL332RED(p)
#define PIXEL2GREEN(p)      PIXEL332GREEN(p)
#define PIXEL2BLUE(p)      PIXEL332BLUE(p)
#endif

/* Color defines*/
#define MWRGB(r,g,b)   ((MWCOLORVAL)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned long)(unsigned char)(b))<<16)))


#define BLACK      RGB2PIXEL( 0  , 0  , 0   )
#define BLUE      RGB2PIXEL( 0  , 0  , 128 )
#define GREEN      RGB2PIXEL( 0  , 128, 0   )
#define CYAN      RGB2PIXEL( 0  , 128, 128 )
#define RED      RGB2PIXEL( 128, 0  , 0   )
#define MAGENTA      RGB2PIXEL( 128, 0  , 128 )
#define BROWN      RGB2PIXEL( 128, 64 , 0   )
#define LTGRAY      RGB2PIXEL( 192, 192, 192 )
#define GRAY      RGB2PIXEL( 128, 128, 128 )
#define LTBLUE      RGB2PIXEL( 0  , 0  , 255 )
#define LTGREEN      RGB2PIXEL( 0  , 255, 0   )
#define LTCYAN      RGB2PIXEL( 0  , 255, 255 )
#define LTRED      RGB2PIXEL( 255, 0  , 0   )
#define LTMAGENTA   RGB2PIXEL( 255, 0  , 255 )
#define YELLOW      RGB2PIXEL( 255, 255, 0   )
#define WHITE      RGB2PIXEL( 255, 255, 255 )

/* other common colors*/
#define DKGRAY      RGB2PIXEL( 32,  32,  32)


//--------------------------------------------------------------------------
/* fblin16, 24, 32.c */
int set_sub_driver_16bpp(PSD psd);
int set_sub_driver_24bpp(PSD psd);
int set_sub_driver_32bpp(PSD psd);


/* main.c */
extern PSD psd; // screen device pointer


/* scr_fb.c */
PSD   GdOpenScreen(void);
void  GdCloseScreen(PSD psd);
int   select_fb_subdriver(PSD psd);
void  gen_fillrect(PSD psd,MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2, MWPIXELVAL c);
void  gfx_draw_sized_rect(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD width, MWCOORD height, MWPIXELVAL c);

#ifndef STANDALONE
int osd_plane_open(void);
void osd_plane_close(void);
#endif

#endif
