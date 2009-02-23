/*
 * Copyright (c) 2000 Greg Haerr <greg@censoft.com>
 *
 * 24bpp Linear Video Driver for Microwindows
 *
 *  - godori <ghcstop>, www.aesop-embedded.org
 *    => Modified. 2001. 02. 11
 */
/*#define NDEBUG*/
#include <assert.h>
#include <string.h>
#include "gfxdev.h"

/* Calc linelen and mmap size, return 0 on fail*/
static int
linear24_init(PSD psd)
{
   if (!psd->size) {
   	  printf("pre psd->linelen = %d\n", psd->linelen);
      psd->size = psd->yres * psd->linelen;
      /* convert linelen from byte to pixel len for bpp 16, 24, 32*/
      psd->linelen /= 3;
      
      printf("aft psd->linelen = %d\n", psd->linelen);
   }
   return 1;
}

/* Set pixel at x, y, to pixelval c*/
static void
linear24_drawpixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c)
{
   ADDR8   addr = psd->addr;
   MWUCHAR   r, g, b;

   assert (addr != 0);
   assert (x >= 0 && x < psd->xres);
   assert (y >= 0 && y < psd->yres);
   assert (c < psd->ncolors);

   x += psd->xoffset;
   y += psd->yoffset;


   r = PIXEL888RED(c);
   g = PIXEL888GREEN(c);
   b = PIXEL888BLUE(c);
   addr += (x + y * psd->linelen) * 3;
      *addr++ = b;
      *addr++ = g;
      *addr = r;
}

/* Read pixel at x, y*/
static MWPIXELVAL
linear24_readpixel(PSD psd, MWCOORD x, MWCOORD y)
{
   ADDR8   addr = psd->addr;

   assert (addr != 0);
   assert (x >= 0 && x < psd->xres);
   assert (y >= 0 && y < psd->yres);

   x += psd->xoffset;
   y += psd->yoffset;


   addr += (x + y * psd->linelen) * 3;
   return RGB2PIXEL888(addr[2], addr[1], addr[0]);
}

/* Draw horizontal line from x1,y to x2,y including final point*/
static void
linear24_drawhorzline(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
   ADDR8   addr = psd->addr;
   MWUCHAR   r, g, b;

   assert (addr != 0);
   assert (x1 >= 0 && x1 < psd->xres);
   assert (x2 >= 0 && x2 < psd->xres);
   assert (x2 >= x1);
   assert (y >= 0 && y < psd->yres);
   assert (c < psd->ncolors);


   x1 += psd->xoffset;
   x2 += psd->xoffset;
   y += psd->yoffset;

   r = PIXEL888RED(c);
   g = PIXEL888GREEN(c);
   b = PIXEL888BLUE(c);
   addr += (x1 + y * psd->linelen) * 3;
      while(x1++ <= x2) {
         *addr++ = b;
         *addr++ = g;
         *addr++ = r;
      }
}

/* Draw a vertical line from x,y1 to x,y2 including final point*/
static void
linear24_drawvertline(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
   ADDR8   addr = psd->addr;
   int   linelen = psd->linelen * 3;
   MWUCHAR   r, g, b;

   assert (addr != 0);
   assert (x >= 0 && x < psd->xres);
   assert (y1 >= 0 && y1 < psd->yres);
   assert (y2 >= 0 && y2 < psd->yres);
   assert (y2 >= y1);
   assert (c < psd->ncolors);


   x += psd->xoffset;
   y1 += psd->yoffset;
   y2 += psd->yoffset;


   r = PIXEL888RED(c);
   g = PIXEL888GREEN(c);
   b = PIXEL888BLUE(c);
   addr += (x + y1 * psd->linelen) * 3;
      while(y1++ <= y2) {
         addr[0] = b;
         addr[1] = g;
         addr[2] = r;
         addr += linelen;
      }
}

int set_sub_driver_24bpp(PSD psd)
{
	psd->Init         = linear24_init;
	psd->DrawPixel    = linear24_drawpixel;
	psd->ReadPixel    = linear24_readpixel;
	psd->DrawHorzLine = linear24_drawhorzline;
	psd->DrawVertLine = linear24_drawvertline;
	psd->FillRect     = gen_fillrect;
	
	return 0;
}
