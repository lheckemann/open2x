/*
 * Copyright (c) 1999, 2000 Greg Haerr <greg@censoft.com>
 *
 * Engine-level Screen, Mouse and Keyboard device driver API's and types
 * 
 * Contents of this file are not for general export
 *
 *                                     fixed by ghc: 2001. 2. 11
 */
#include "gfxdev.h"

#ifdef STANDALONE
//	#define GDEBUG
	#ifdef GDEBUG
   		FILE *dbg;
		#  define gprintf(x...) fprintf(dbg, x)
	#else
		#  define gprintf(x...)
	#endif

	extern int sondisp(void);
#endif

PSD psd;


#ifndef STANDALONE
int osd_plane_open(void)
{
   if((psd = GdOpenScreen()) == NULL)
   {
      printf("Cannot initialise screen\n");
      return -1;
   }
   printf("graphic device initialize done\n");
   printf("screen device open success: pixelvalue = %d\n", sizeof(MWPIXELVAL));
   printf("psd->xres = %d, psd->yres = %d, psd->linelen = %d\n", psd->xres, psd->yres, psd->linelen);

return 0;

DONE:
   GdCloseScreen(psd);
   return -1;
}

void osd_plane_close(void)
{
	if( psd )
		GdCloseScreen(psd);
}

#endif

