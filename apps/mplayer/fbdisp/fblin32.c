/*
 * Copyright (c) 1999 Greg Haerr <greg@censoft.com>
 *
 * 32bpp Linear Video Driver for Microwindows
 *
 * Inspired from Ben Pfaff's BOGL <pfaffben@debian.org>
 *
 *  - godori <ghcstop>, www.aesop-embedded.org
 *    => Modified. 2001. 02. 11
 */

#include <assert.h>
#include <string.h>
#include "gfxdev.h"

/* Calc linelen and mmap size, return 0 on fail*/
static int
linear32_init(PSD psd)
{
   if (!psd->size) {
      psd->size = psd->yres * psd->linelen;
      /* convert linelen from byte to pixel len for bpp 16, 24, 32*/
      psd->linelen /= 4;
   }
   return 1;
}

/* Set pixel at x, y, to pixelval c*/
static void
linear32_drawpixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c)
{
   ADDR32   addr = psd->addr;

   assert (addr != 0);
   assert (x >= 0 && x < psd->xres);
   assert (y >= 0 && y < psd->yres);
   assert (c < psd->ncolors);
   
   x += psd->xoffset;
   y += psd->yoffset;
   

   addr[x + y * psd->linelen] = c;
}

/* Read pixel at x, y*/
static MWPIXELVAL
linear32_readpixel(PSD psd, MWCOORD x, MWCOORD y)
{
   ADDR32   addr = psd->addr;

   assert (addr != 0);
   assert (x >= 0 && x < psd->xres);
   assert (y >= 0 && y < psd->yres);
   
   x += psd->xoffset;
   y += psd->yoffset;
   

   return addr[x + y * psd->linelen];
}

/* Draw horizontal line from x1,y to x2,y including final point*/
static void
linear32_drawhorzline(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
   ADDR32   addr = psd->addr;

   assert (addr != 0);
   assert (x1 >= 0 && x1 < psd->xres);
   assert (x2 >= 0 && x2 < psd->xres);
   assert (x2 >= x1);
   assert (y >= 0 && y < psd->yres);
   assert (c < psd->ncolors);

   x1 += psd->xoffset;
   x2 += psd->xoffset;
   y += psd->yoffset;


   addr += x1 + y * psd->linelen;
   //FIXME: memsetl(dst, c, x2-x1+1)?
   while(x1++ <= x2)
       *addr++ = c;
}

/* Draw a vertical line from x,y1 to x,y2 including final point*/
static void
linear32_drawvertline(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
   ADDR32   addr = psd->addr;
   int   linelen = psd->linelen;

   assert (addr != 0);
   assert (x >= 0 && x < psd->xres);
   assert (y1 >= 0 && y1 < psd->yres);
   assert (y2 >= 0 && y2 < psd->yres);
   assert (y2 >= y1);
   assert (c < psd->ncolors);


   x += psd->xoffset;
   y1 += psd->yoffset;
   y2 += psd->yoffset;


      while(y1++ <= y2) {
         *addr = c;
         addr += linelen;
      }
}


int set_sub_driver_32bpp(PSD psd)
{
	psd->Init         = linear32_init;
	psd->DrawPixel    = linear32_drawpixel;
	psd->ReadPixel    = linear32_readpixel;
	psd->DrawHorzLine = linear32_drawhorzline;
	psd->DrawVertLine = linear32_drawvertline;
	psd->FillRect     = gen_fillrect;
	
	return 0;
}
