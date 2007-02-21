//[*]----------------------------------------------------------------------------------------------------[*]
/*
 * Copyright (c) 1999 Greg Haerr <greg@censoft.com>
 *
 * 16bpp Linear Video Driver for Microwindows
 *
 * Inspired from Ben Pfaff's BOGL <pfaffben@debian.org>
 *
 *  - godori <ghcstop>, www.aesop-embedded.org
 *    => Modified. 2001. 02. 11
 */
//[*]----------------------------------------------------------------------------------------------------[*]
/*#define NDEBUG*/
#include <assert.h>
#include <string.h>
#include "gfxdev.h"
//[*]----------------------------------------------------------------------------------------------------[*]
/* Calc linelen and mmap size, return 0 on fail*/
static int linear16_init(PSD psd)
{
   if (!psd->size) {
      psd->size = psd->yres * psd->linelen;
      /* convert linelen from byte to pixel len for bpp 16, 24, 32*/
      psd->linelen /= 2;
   }
   return 1;
}
//[*]----------------------------------------------------------------------------------------------------[*]
/* Set pixel at x, y, to pixelval c*/
static void linear16_drawpixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c)
{
   ADDR16   addr = psd->addr;

//   assert (addr != 0);
//   assert (x >= 0 && x < psd->xres);
//   assert (y >= 0 && y < psd->yres);
//   assert (c < psd->ncolors);

   if(!(addr != 0))					return;
   if(!(x >= 0 && x < psd->xres))	return;
   if(!(y >= 0 && y < psd->yres))	return;
   if(!(c < psd->ncolors))			return;

   x += psd->xoffset;
   y += psd->yoffset;

   addr[x + y * psd->linelen] = c;
}
//[*]----------------------------------------------------------------------------------------------------[*]
/* Read pixel at x, y*/
static MWPIXELVAL linear16_readpixel(PSD psd, MWCOORD x, MWCOORD y)
{
   ADDR16   addr = psd->addr;

//   assert (addr != 0);
//   assert (x >= 0 && x < psd->xres);
//   assert (y >= 0 && y < psd->yres);

   if(!(addr != 0))					return;
   if(!(x >= 0 && x < psd->xres))	return;
   if(!(y >= 0 && y < psd->yres))	return;

   x += psd->xoffset;
   y += psd->yoffset;


   return addr[x + y * psd->linelen];
}
//[*]----------------------------------------------------------------------------------------------------[*]
/* Draw horizontal line from x1,y to x2,y including final point*/
static void linear16_drawhorzline(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
   ADDR16   addr = psd->addr;

//   assert (addr != 0);
//   assert (x1 >= 0 && x1 < psd->xres);
//   assert (x2 >= 0 && x2 < psd->xres);
//   assert (x2 >= x1);
//   assert (y >= 0 && y < psd->yres);
//   assert (c < psd->ncolors);

   if(!(addr != 0))						return;
   if(!(x1 >= 0 && x1 < psd->xres))		return;
   if(!(x2 >= 0 && x2 < psd->xres))		return;
   if(!(x2 >= x1))						return;
   if(!(y >= 0 && y < psd->yres))		return;
   if(!(c < psd->ncolors))				return;

   x1 += psd->xoffset;
   x2 += psd->xoffset;
   y += psd->yoffset;



   addr += x1 + y * psd->linelen;
      //FIXME: memsetw(dst, c, x2-x1+1)?
      while(x1++ <= x2)
         *addr++ = c;
}
//[*]----------------------------------------------------------------------------------------------------[*]
/* Draw a vertical line from x,y1 to x,y2 including final point*/
static void linear16_drawvertline(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
   ADDR16   addr = psd->addr;
   int   linelen = psd->linelen;

//   assert (addr != 0);
//   assert (x >= 0 && x < psd->xres);
//   assert (y1 >= 0 && y1 < psd->yres);
//   assert (y2 >= 0 && y2 < psd->yres);
//   assert (y2 >= y1);
//   assert (c < psd->ncolors);

   if(!(addr != 0))					return;
   if(!(x >= 0 && x < psd->xres))	return;
   if(!(y1 >= 0 && y1 < psd->yres))	return;
   if(!(y2 >= 0 && y2 < psd->yres))	return;
   if(!(y2 >= y1))					return;
   if(!(c < psd->ncolors))			return;

   x += psd->xoffset;
   y1 += psd->yoffset;
   y2 += psd->yoffset;


   addr += x + y1 * linelen;
      while(y1++ <= y2) {
         *addr = c;
         addr += linelen;
      }
}
//[*]----------------------------------------------------------------------------------------------------[*]
int set_sub_driver_16bpp(PSD psd)
{
	psd->Init         = linear16_init;
	psd->DrawPixel    = linear16_drawpixel;
	psd->ReadPixel    = linear16_readpixel;
	psd->DrawHorzLine = linear16_drawhorzline;
	psd->DrawVertLine = linear16_drawvertline;
	psd->FillRect     = gen_fillrect;

	return 0;
}
//[*]----------------------------------------------------------------------------------------------------[*]
