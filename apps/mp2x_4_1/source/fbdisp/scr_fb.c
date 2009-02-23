/*
 * Copyright (c) 1999, 2000 Greg Haerr <greg@censoft.com>
 *
 * Microwindows Screen Driver for Linux kernel framebuffers
 *
 * Portions used from Ben Pfaff's BOGL <pfaffben@debian.org>
 * 
 * Note: modify select_fb_driver() to add new framebuffer subdrivers
 *
 *                                       fixed by ghc: 2001.02.11
 */
#include "gfxdev.h"

#include "fbs.h" // framebuffer control

#include "../cx25874.h"
extern unsigned int disp_mode;

static PSD  fb_open(PSD psd);
static void fb_close(PSD psd);

// fixed by ghc: 2001.02.11
SCREENDEVICE   scrdev = {
-1,                    
   0,                  
   0,                  
   0,                  
   0,                  
   0,                  
   0,                  
   0,                  
   0,                  
   0,                  
   0,                  
   0,                  
   NULL,               
   fb_open,            
   fb_close,           
   NULL,               
   NULL,               
   NULL,               
   NULL,               
   NULL,               
   NULL,               

   0, 
   0, 
   0, 
   0  
};

/* static variables*/
static int fb;         /* Framebuffer file handle. */
static int status;      /* 0=never inited, 1=once inited, 2=inited. */


/* init framebuffer*/
static PSD
fb_open(PSD psd)
{
   char *   env;
   int   type, visual;
   struct fb_fix_screeninfo fb_fix;
   struct fb_var_screeninfo fb_var;
   int   tty;
   int   rbpp;

   assert(status < 2);

   /* locate and open framebuffer, get info*/
   if(!(env = getenv("FRAMEBUFFER")))
   {
      #ifndef X86FB
      	#ifdef USE_1ST_FB
      	env = "/dev/fb/0";
      	#else
      	env = "/dev/fb/1";
      	#endif
      #else
      	env = "/dev/fb";
      #endif
   }
   fb = open(env, O_RDWR);
   
   if(fb < 0) 
   {
      printf("Error opening %s: %m. Check kernel config\n", env);
      return NULL;
   }
   if(ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix) == -1 ||
      ioctl(fb, FBIOGET_VSCREENINFO, &fb_var) == -1) {
         printf("Error reading screen info: %m\n");
         goto fail;
   }
   /* setup screen device from framebuffer info*/
   type = fb_fix.type;
   visual = fb_fix.visual;

   psd->planew = psd->xres = psd->xvirtres = fb_var.xres;
   psd->planeh = psd->yres = psd->yvirtres = fb_var.yres;
   
   //printf(" psd->xres = %d, psd->yres = %d\n", psd->xres, psd->yres);

   psd->bpp = fb_var.bits_per_pixel;
   psd->ncolors = (psd->bpp >= 24)? (1 << 24): (1 << psd->bpp);

   /* set linelen to byte length, possibly converted later*/
   psd->linelen = fb_fix.line_length;
   
   printf("bpp = %d, linelen = %d\n", psd->bpp, psd->linelen);
   
   rbpp = psd->linelen/psd->xres; 
   
   if( (psd->bpp/8) != rbpp ) 
   {
   	   printf("real bpp(%d) is not equal read bpp(%d)\n", rbpp*8, psd->bpp);
   	   printf("change bpp to real bpp\n");
   	
   	   psd->bpp = rbpp*8;
   }
   
   #ifndef X86FB
   #ifdef FBMMSP2CTRL
   {
   	   	int err;
	   	Msgdummy dummymsg, *pdmsg;
		PMsgGetRect      prect;
		
   		memset( (char *)&dummymsg, 0x00, sizeof(Msgdummy) );
   		pdmsg = &dummymsg;
   		prect = (PMsgGetRect)pdmsg;
   		MSG(prect)   = MMSP2_FB_GET_RECT_INFO;
   		LEN(prect)   = sizeof(MsgGetRect);
   		if( (err = ioctl(fb, FBMMSP2CTRL, prect) < 0) )
   		{
       		printf("FBMMSP2CTRL error\n");
       		exit(1);
   		}
   		printf("0 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(prect), LEN(prect) );
   		printf("prect->xoff = %d\n", prect->xoff);
   		printf("prect->yoff = %d\n", prect->yoff);
   		printf("prect->w    = %d\n", prect->w);
   		printf("prect->h    = %d\n", prect->h);
   
		psd->xoffset = prect->xoff;
		psd->yoffset = prect->yoff;
		psd->planew  = prect->w;
		psd->planeh  = prect->h;
		
		if(disp_mode != DISPLAY_TV)  // hhsong add..
// later TV, monitor add
		{
			psd->xoffset = prect->xoff;
			psd->yoffset = prect->yoff;
			psd->planew  = prect->w;
			psd->planeh  = prect->h;
		}
   }
   #endif
   #endif
   
   
   psd->size = 0;      /* force subdriver init of size*/

   if( select_fb_subdriver(psd) < 0 )
   {
   	   printf("fb subdriver init fail\n");
   	   goto fail;
   }

   #ifdef X86FB
   /* open tty, enter graphics mode*/
   tty = open ("/dev/tty0", O_RDWR);
   if(tty < 0) {
      printf("Error can't open /dev/tty0: %m\n");
      goto fail;
   }
   if(ioctl (tty, KDSETMODE, KD_GRAPHICS) == -1) {
      printf("Error setting graphics mode: %m\n");
      close(tty);
      goto fail;
   }
   close(tty);
   #endif	
   

   /* mmap framebuffer into this address space*/
   psd->size = (psd->size + getpagesize () - 1) / getpagesize () * getpagesize ();
   psd->addr = mmap(NULL, psd->size, PROT_READ|PROT_WRITE,MAP_SHARED, fb, 0);
   
   if(psd->addr == NULL || psd->addr == (unsigned char *)-1) 
   {
      printf("Error mmaping %s: %m\n", env);
      goto fail;
   }

   status = 2;
   
psd->fbfd = fb; /* ghcstop add: 20040902 mmsp2 fb ioctl test */    

   return psd;   /* success*/

fail:
   close(fb);
   
   return NULL;
}

/* close framebuffer*/
static void
fb_close(PSD psd)
{
   int   tty;
   
   /* if not opened, return*/
   if(status != 2)
      return;
   status = 1;

   /* unmap framebuffer*/
   munmap(psd->addr, psd->size);

   #ifdef X86FB
   /* enter text mode*/
   tty = open ("/dev/tty0", O_RDWR);
   ioctl(tty, KDSETMODE, KD_TEXT);
   close(tty);
   #endif

   /* close framebuffer*/
   close(fb);
}




/*
 * Open low level graphics driver
 */
PSD GdOpenScreen(void)
{
   PSD         psd;

   psd = scrdev.Open(&scrdev);
   if (!psd)
      return NULL;

   /* fill black (actually fill to first palette entry or truecolor 0*/
   //psd->FillRect(psd, 0, 0, psd->xvirtres-1, psd->yvirtres-1, RGB2PIXEL(255, 255, 255));

   return psd;
}

/*
 * Close low level graphics driver
 */
void
GdCloseScreen(PSD psd)
{
   psd->Close(psd);
}


/*
 * fixghc: 2001.02.17
 */  
void gen_fillrect(PSD psd,MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2, MWPIXELVAL c)
{
   while(y1 <= y2)
      psd->DrawHorzLine(psd, x1, x2, y1++, c);
}



// ghcstop: 2001.02.17
void gfx_draw_sized_rect(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD width, MWCOORD height, MWPIXELVAL c)
{
  MWCOORD x2 = x1+width-1;
  MWCOORD y2 = y1+height-1;

  psd->FillRect(psd, x1, y1, x2, y2, c);
}




/* select a framebuffer subdriver based on planes and bpp*/
/* modify this procedure to add a new framebuffer subdriver*/
int select_fb_subdriver(PSD psd)
{
   switch(psd->bpp) 
   {
      case 16:
         set_sub_driver_16bpp(psd);
         break;
      case 24:
         set_sub_driver_24bpp(psd);
         break;
      case 32:
		 set_sub_driver_32bpp(psd);
         break;
      default:
		   return -1;
   }
   
    psd->Init(psd);

    /* return driver selected*/
    return 1;
}


