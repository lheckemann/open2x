/*	tv_daemon.c  TV tweaking daemon for Open2X. 
 	Copyright (C) 2008 Dan Silsby (Senor Quack)
	 Portions of code based on :

	parts (c) Rlyehs Work	(minlib)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stropts.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "mmsp2_regs.h"

#define MIN_PITCH		1
#define MAX_PITCH		2048
//#define MIN_XOFFSET	(-50)
//#define MAX_XOFFSET	35
//#define MIN_YOFFSET	(-15)
//#define MAX_YOFFSET	45
#define MIN_XOFFSET	(-50)
#define MAX_XOFFSET	35
#define MIN_YOFFSET	(-21)	 // Any less than this, my NTSC TV displays black
#define MAX_YOFFSET	85	// Can probably go further than this but, damn, do we need to?
#define MIN_XSCALE	12
#define MAX_XSCALE	200
#define MIN_YSCALE	12
#define MAX_YSCALE	200
#define MIN_VXSCALE	12
#define MAX_VXSCALE	200
#define MIN_VYSCALE	12
#define MAX_VYSCALE	200
#define DEFAULT_NTSC_XOFFSET	10
#define DEFAULT_NTSC_YOFFSET	(-7)
//Changed this to the NTSC default since someone showed that it was too far to the right:
//#define DEFAULT_PAL_XOFFSET	16
#define DEFAULT_PAL_XOFFSET	10
#define DEFAULT_PAL_YOFFSET	19
#define MIN_DELAY					1
#define MAX_DELAY					120
#define DEFAULT_DELAY			1
#define MIN_FIRST_DELAY			1
#define MAX_FIRST_DELAY			120
#define DEFAULT_FIRST_DELAY	4

//senquack - pulled from Rlyeh's minlib for improved TVout:
#define gp2x_cx25874_read(a)    gp2x_i2c_read(0x8A,(a))
#define gp2x_cx25874_write(a,v) gp2x_i2c_write(0x8A,(a),(v))
typedef struct { unsigned char id,addr,data;} i2cw;
typedef struct { unsigned char id,addr,*pdata;} i2cr;

// Found out from GPH sources that LCD is one, not zero as many seem to think:
enum  { LCD = 1, PAL = 4, NTSC = 3 };

extern int errno;
unsigned short *gp2x_memregs;
int memfd;
int cx25874 = 0;	// fd for chip device driver

int	xoffset = 999;		// Magic 999 tells program if xoffset wasn't given on command line
int 	yoffset = 999;		// ""
int	xscale_percent = 100;
int 	yscale_percent = 100;
int	vxscale_percent = 100;
int 	vyscale_percent = 100;
int	first_delay	= DEFAULT_FIRST_DELAY;  // How many seconds to wait before first tweak?
int	delay	= DEFAULT_DELAY;	// How many seconds to wait inbetween subsequent tweaks?
int	mode = NTSC;
int	tweak_only_once = 0;
int	tweak_yuv = 0;
int	enable_tvmode = 0;
//int	stubborn_fix = 0;  /* for apps like Tilematch and Blingo, their SDL has few bugs
//									 that leave registers for tvout incorrectly set, this 
//									 will fix that if enabled. */
int	scaling_tweak = 1;	/* Hit the scaling registers?  For apps like mame and lemonboy2x,
									if this is 1 it will cause problems.  They hit the scaling 
									registers intentionally but without at least some other 
									tweaks they can't display tv properly. */
int	pal_overscan_fix = 0;/* When in PAL mode, apply Rlyeh's overscan stuff. I have a suspicion
									that this is what is causing people problems with cut-off lower 
									portions of the picture on PAL TVs, so it is now turned off by
									default. */
int	pitch = 0;

void gp2x_video_RGB_setscaling(int W, int H);
void *trymmap (void *start, size_t length, int prot, int flags, int fd, off_t offset);
unsigned char initphys (void);
void gp2x_i2c_write(unsigned char id, unsigned char addr, unsigned char data);
void gp2x_video_RGB_setscaling(int W, int H);
void gp2x_video_YUV_setscaling(int region, int W, int H);
void tweaktvout(int pal, int init_tv, int force_phys_pitch);
void loop (void);
void closephys (void);
void displayhelp (void);

void *trymmap (void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	char *p;
	int aa;

	p = mmap (start, length, prot, flags, fd, offset);
	if (p == (char *)0xFFFFFFFF)
	{
		aa = errno;
		printf ("mmap failed. errno = %d\n", aa);
		exit(1);
	}

	return p;
}

unsigned char initphys (void)
{
	memfd = open("/dev/mem", O_RDWR);
	if (memfd == -1)
	{
		printf ("Opening /dev/mem failed\n");
		return 0;
	}

	gp2x_memregs = (unsigned short *)trymmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, 0xc0000000);
	return 1;
}

//senquack - pulled from Rlyeh's minlib for improved TVout:
/* Function: gp2x_i2c_write
   This function writes a byte into an I2C address.
   
   Parameters:
   id (0..) - i2c ID
   addr - address to write data to.
   data (0..255) - data to be written.

   See also:
   <gp2x_i2c_read>
   
   Credits:
   rlyeh */

void gp2x_i2c_write(unsigned char id, unsigned char addr, unsigned char data)
{
 i2cw a;
 a.id = id, a.addr = addr, a.data = data;
 ioctl(cx25874, _IOW('v', 0x00, i2cw), &a); //CX25874_I2C_WRITE_BYTE
}

//senquack - pulled from Rlyeh's minlib for improved TVout, extensively modified
/* Function: gp2x_video_RGB_setscaling
   This function adjusts a given resolution to fit the whole display (320,240).

   Notes:
   - Draw at (0,0) coordinates always, whatever resolution you are working with.
   - Call this function once. Do not call this function in every frame.
  
   Parameters:
   W (1..) - virtual width in pixels to scale to 320.
   H (1..) - virtual height in pixels to scale to 240.

   Default:
   - (W,H) are set to (320,240).
   
   Credits:
   rlyeh */

void gp2x_video_RGB_setscaling(int W, int H)
{
  short tmp;
  int bpp=(gp2x_memregs[0x28DA>>1]>>9)&0x3;
  int mul = (gp2x_memregs[DPC_CNTL] & 0x100 ? 512 : 1024);

  // some video modes are 360-pixels wide so we must account for that when
  // doing scaling ratio:
	int div = gp2x_memregs[MLC_STL_HW] >> (bpp - 1);
	if (div != 360)
	{
		div = 320;
	}

  // scale horizontal
  tmp = (unsigned short)((float)mul *((float)W / (float)div));
  // round to nearest multiple of 8
  tmp &= ~0x7;
  gp2x_memregs[MLC_STL_HSC]=tmp;
  
  // scale vertical
	tmp = (unsigned  short)((float)320.0*bpp *(H/240.0));
  // round to nearest multiple of 8
	tmp &= ~0x7;
	gp2x_memregs[MLC_STL_VSCL]=tmp;
	gp2x_memregs[MLC_STL_VSCH] = 0;
}

////senquack - pulled from Rlyeh's minlib for improved TVout, 
//		extensively modified.  Alters only the top of Region A
//		(for video playing)
///* Function: gp2x_video_YUV_setscaling
//   This function adjusts a given resolution to fit the whole display (320,240).
//
//   Notes:
//   - Draw at (0,0) coordinates of each framebuffer always, whatever resolution you are working with.
//   - Call this function once. Do not call this function in every frame.
//  
//   Parameters:
//   region (0..3) - YUV region (0..3)
//   W (1..) - virtual width in pixels to scale to 320.
//   H (1..) - virtual height in pixels to scale to 240.
//
//   Default:
//   - (W,H) are set to (320,240) for each region.
//   
//   Credits:
//   rlyeh */
//
void gp2x_video_YUV_setscaling(int region, int W, int H)
{
	short reg;

	//senquack - set region A Top pixel skip register to 0:
	gp2x_memregs[MLC_YUV_CNTL] &= ~(1<<3);
	gp2x_memregs[MLC_YUV_CNTL] &= ~(1<<2);
  
	//VERTICAL SCALING:
	reg = (float)gp2x_memregs[MLC_YUVA_TP_PXW] * ((float)H / 240.0);
	gp2x_memregs[MLC_VLA_TP_VSCL] = reg;
	gp2x_memregs[MLC_VLA_TP_VSCH] &= ~0x1FF;

	//HORIZONTAL SCALING:
	reg = 1024.0 * ((float)W / 320.0);
	reg &= 0x7FF;
	gp2x_memregs[MLC_YUVA_TP_HSC] = reg;
}

// Parameters: pal 					0: NTSC		1: PAL
//					init_tv 				0: Don't set tv mode		1: set tv mode indicated by pal
//					force_phys_pitch	Override physical pitch to be this number (720 needed for
//												gngeo)... disabled if 0
// Note: I got it to automatically deal with various pitches, so no need for the last
// 	parameter for now
void tweaktvout(int pal, int init_tv, int force_phys_pitch)
{
	static int set_scaling = 0;		// Done the initial setting of scaling yet?
//	static int initial_stubborn_fix = 0;	// Done the initial stubborn fix?

	unsigned int bytes_per_pixel = (gp2x_memregs[MLC_STL_CNTL]>>9)&0x3;
	unsigned int width = 320;
	unsigned int height = 240;
	unsigned int pitch, phys_pitch;
//	static int scaled_width = -1;
//	static int scaled_height = -1;
	int orig_w = 320;		// orig_w and orig_h get set to the user-specified scaled 
								//		percentage of 320x240 a little later before anything
								//		gets tweaked.  Later tweaks tweak based on the auto-
								//		detected height and width no longer need to do 
								//		any math, the stored values are already scaled properly.
	int orig_h = 240;

	static int scaled_vwidth = -1, scaled_vheight = -1;
	
	if ((scaled_vwidth == -1) || (scaled_vheight == -1))
	{
		// We can't do auto-detection of height and width on the YUV layers, so
		// these get set once and then YUV is set to these values that never change.
		scaled_vwidth = (int)(320.0 * (100.0 / (float)vxscale_percent));
		scaled_vheight = (int)(240.0 * (100.0 / (float)vyscale_percent));
	}


	if (init_tv)
	{
		cx25874 = open("/dev/cx25874",O_RDWR);
		if (cx25874 != -1)
		{
			ioctl(cx25874, _IOW('v', 0x02, unsigned char), pal ? PAL : NTSC);
			close(cx25874);
		} else 
		{
			cx25874 = 0;
		}
	}

	//	Open special Open2X device driver that doesn't reset the hardware on opening
	cx25874 = open("/dev/cx25874_open2x",O_RDWR);
	if (cx25874 == -1)
	{
		cx25874 = 0;
#if DEBUG
		printf("Error opening /dev/cx25874_open2x, some tweaks won't be applied.\n");
#endif
		return;
	}

	if (!(gp2x_memregs[DPC_CNTL]&0x100))
	{
#if DEBUG
		printf("Cannot tweak, TV output not enabled!  Exiting..\n");
#endif
		if (cx25874)
			close (cx25874);
		cx25874 = 0;
//		closephys();
//		exit(1);
		return;
	}

	// Do one-time setting of scaling registers before anything.
	if (scaling_tweak && !set_scaling)
//	if (!set_scaling)
	{
		gp2x_memregs[DPC_Y_MAX] = 239; 
		orig_w = (long)(320.0 * (100.0 / (float)xscale_percent));
		orig_h = (long)(240.0 * (100.0 / (float)yscale_percent)); 

		gp2x_video_RGB_setscaling( orig_w, orig_h );


		set_scaling = 1;
	}

//	if (tweak_yuv)
//	{
//		gp2x_video_YUV_setscaling(0, scaled_vwidth, scaled_vheight );
//	}

//	// This fixes apps that used a buggy SDL like some of Ruckage's fenix games
//	// and Tilematch.  When using TVout, they have horrible interlacing otherwise.
//	if (stubborn_fix && !initial_stubborn_fix && cx25874)
//	{
////		gp2x_memregs[MLC_STL_HW] = 640;
//		gp2x_memregs[MLC_STL_HW] = (bytes_per_pixel == 1) ? 320 : 640;
////		gp2x_memregs[MLC_STL1_ENDX] = 639;
////		gp2x_memregs[MLC_STL1_ENDX] = (bytes_per_pixel == 1) ? 319 : 639;
//		gp2x_memregs[MLC_STL1_ENDX] = 719;
//		gp2x_memregs[DPC_X_MAX] = 719;
//		gp2x_memregs[MLC_STL_MIXMUX] = 0;
//		gp2x_memregs[MLC_STL_ALPHAL] = 255;
//		gp2x_memregs[MLC_STL_ALPHAH] = 255;
//		gp2x_memregs[MLC_OVLAY_CNTR] |= DISP_STL1EN;
//		// This is the critical fix for interlacing:
////		gp2x_memregs[MLC_STL_EADRL] = gp2x_memregs[MLC_STL_OADRL];
////		gp2x_memregs[MLC_STL_EADRH] = gp2x_memregs[MLC_STL_OADRH];
//		initial_stubborn_fix = 1;
//	}
//
	// This is the critical fix for interlacing:
	gp2x_memregs[MLC_STL_EADRL] = gp2x_memregs[MLC_STL_OADRL];
	gp2x_memregs[MLC_STL_EADRH] = gp2x_memregs[MLC_STL_OADRH];
	

#if DEBUG
	printf("Detecting hw of %d\n", gp2x_memregs[MLC_STL_HW]);
	printf("Detecting endx of %d\n", gp2x_memregs[MLC_STL1_ENDX]);
	printf("Detecting YUV endx of %d\n", gp2x_memregs[MLC_VLA_ENDX]);
	printf("DPC_X_MAX: %d\n", gp2x_memregs[DPC_X_MAX]);
  	printf("DPC_Y_MAX: %d\n", gp2x_memregs[DPC_Y_MAX]);
	printf("MLC_STL_HSC: %d\n", gp2x_memregs[MLC_STL_HSC]);
	printf("MLC_STL_VSCL: %d\n", gp2x_memregs[MLC_STL_VSCL]);
#endif

	// Auto-detection of width
	width = gp2x_memregs[MLC_STL_HW] >> (bytes_per_pixel - 1);

	// Some SDLs set the width wrong, apparently (Blingo's fenix, spout)
	if (width == 640)
	{
		width = 320;
	}

	// Massage the vertical scaling register if it gets fubar'd
	if (gp2x_memregs[MLC_STL_VSCL] < 32)
	{
		// For some reason, it seems a lot of code out there (early minlibs)
		// ends up putting this at very low values, making it impossible to
		// see anyhthing.  Fix this to a sane value if this occurs.
#ifdef DEBUG
		printf("VSCL went out of whack, resetting scaling..\n");
#endif
//		gp2x_video_RGB_setscaling( orig_w, orig_h );
//		gp2x_video_RGB_setscaling( width, (int)(((float)width / 320.0) * 240.0)  );
//		gp2x_video_RGB_setscaling( width, (int)(((float)width / 320.0) * 240.0) + 8 );
		if (gp2x_memregs[MLC_STL_HSC] == 489)
		{
			// using an old version of minlib and program has changed to 320x240 mode
			gp2x_memregs[MLC_STL_VSCL] = (bytes_per_pixel == 1) ? 320 : 640;
		} else
		{
			// Since the VSCL register has been wiped out, we have no way of knowing
			// what the height is supposed to be.  All we can do is look at the width
			// and come up with a height that matches a 4:3 width:height ratio and hope
			// for the best.  Seems to work pretty well.	
			// If I don't have the +24 here, the image goes off the bottom of the screen.
			//  Who knows what it does with or without in PAL mode, I only have a NTSC tv.
			gp2x_video_RGB_setscaling( width, (int)(((float)width / 320.0) * 240.0) + 24 );
//			gp2x_video_RGB_setscaling( width, (int)(((float)width / 320.0) * 240.0) );
		}

#ifdef DEBUG
		printf("after reset, MLC_STL_HSC: %d\n", gp2x_memregs[MLC_STL_HSC]);
		printf("after reset, MLC_STL_VSCL: %d\n", gp2x_memregs[MLC_STL_VSCL]);
#endif

	}
	
	// Auto-detection of height (not so easy as the width)
	int tmp = gp2x_memregs[MLC_STL_VSCL];
	tmp += 7;
	tmp &= ~0x7;
	height = (int)(((float)tmp * 240.0) / (320.0 * (float)bytes_per_pixel));
	height += 7;
	height &= ~0x7;



#ifdef DEBUG
	printf("orig_w: %d orig_h: %d\n", orig_w, orig_h);
	printf("Detecting width, height of %d,%d\n", width, height);
	printf("Detecting bpp of %d\n", bytes_per_pixel);
#endif
	
	pitch = phys_pitch = width << (bytes_per_pixel == 1 ? 0 : 1);

	if (force_phys_pitch > 0)
	{
		phys_pitch=force_phys_pitch;
	}

//	// Do maintenance tweak:
	if (scaling_tweak)
	{
		gp2x_video_RGB_setscaling(width, height);
	}

	if (tweak_yuv)
	{
		gp2x_video_YUV_setscaling(0, scaled_vwidth, scaled_vheight );
		gp2x_memregs[MLC_VLA_ENDX] = 719;
	}
	
	// A few last tweaks for the display controller before moving to the encoder chip
	gp2x_memregs[MLC_STL1_ENDX]= 719;
	gp2x_memregs[MLC_STL_HW] = phys_pitch;
	
	//	BEGIN RLYEH'S TV IMAGE CENTERING:
	// --------------------------------
	int lines, syncs_start, syncs_end;

	//horizontal adjustment
	if (cx25874)
	{
		gp2x_cx25874_write(0x8c, (unsigned char) (50 - 11 + 3 - xoffset) );
	}

	
	//vertical adjustment
	if(pal) 
		lines = 288, syncs_start = 1, syncs_end = 24; 
	else 
		lines = 240, syncs_start = 1, syncs_end = 22;
	 
	lines -= yoffset, syncs_end += yoffset;
		 
	gp2x_memregs[DPC_Y_MAX]  =  lines - 1;
	gp2x_memregs[0x2820 >> 1] &= (0xFF00);
	gp2x_memregs[0x2820 >> 1] |= (syncs_start << 8);
	gp2x_memregs[0x2822 >> 1] &= ~(0x1FF);
	gp2x_memregs[0x2822 >> 1] |=  syncs_end; // syncs_end = verBackPorch+verFontPorch - 1

//	if(pal && cx25874)
	//senquack - think this might be the code causing PAL problems with bottom part of image
	//		cut off.  No way of knowing, I don't have a PAL tv.  This code is now turned off
	//		by default.
	if(pal && pal_overscan_fix && cx25874)
	{
		//bottom screen image cut off (PAL 320x288 full -> PAL 320x240 centered w/ black borders)
		int real_lines = 288, 
		wanted_lines = 240, 
		top_spacing = (real_lines - wanted_lines) << 1,
		active_lines = wanted_lines + top_spacing;

		active_lines += -top_spacing +5 + yoffset;

		gp2x_cx25874_write(0x84, active_lines & 0xFF); //reduce overscan, VACTIVE_0
		gp2x_cx25874_write(0x86, 0x26 | ((active_lines & 0x100)>>1) );

		gp2x_cx25874_write(0x94, active_lines & 0xFF); //reduce overscan, VACTIVE_1
		gp2x_cx25874_write(0x96, 0x31 | ((active_lines & 0x300)>>8) );
	}

	//	END RLYEH'S TV IMAGE CENTERING
	// --------------------------------

	if (cx25874)
	{
		close(cx25874);
	}
}

void loop (void)
{
	struct timespec timereq, timerem;
	timereq.tv_sec = first_delay;
	timereq.tv_nsec = 0;

	// Do initial tweak (but only after waiting)
	nanosleep(&timereq, &timerem);
	tweaktvout((mode == PAL), enable_tvmode, pitch );

	if (tweak_only_once)
		return;

	timereq.tv_sec = delay;
	timereq.tv_nsec = 0;

	while (1)
	{
		nanosleep(&timereq, &timerem);
#if DEBUG
		printf("Tweaking TV..\n");
#endif
		tweaktvout(mode == PAL, 0, pitch);
	}
}

void closephys (void)
{
	close (memfd);
}

void displayhelp (void)
{
	printf("Open2X TV tweaking daemon written by Senor Quack v1.0\n");
	printf("  Copyright (C) 2009 Daniel Silsby\n");
	printf("  Portions of code adapted from Rlyeh's minlib \n");
	printf("OPTIONS:\n");
	printf("-h\t\tDisplay help and version info\n");
	printf("-p\t\tUse PAL timings (default is NTSC)\n");
	printf("-P\t\tApply Rlyeh's PAL overscan tweak (untested)\n");
	printf("-e\t\tEnable TV mode initially (normally not needed)\n");
//	printf("-s\t\tEnable 'stubborn' fix for some older SDL/Fenix apps\n");
//	printf("\t\t\tlike Tilematch or Blingo (fixes flicker)\n");
	printf("-S\t\tDisable tweaking of scaling registers (for Mame,Lemonboy2x,etc)\n");
//	printf("-fPITCH\t\tForce physical pitch of PITCH.  Valid range: %d..%d\n",
//			MIN_PITCH, MAX_PITCH);
//	printf("\t\t\t(Normally 320 for 1bpp mode, 640 for 2bpp mode, but some\n"
//			"\t\t\t programs like gngeo need a setting of 720 forced here.)\n");
	printf("-xOFFSET\tMove image right/left. Valid range: %d..%d\n",
			MIN_XOFFSET, MAX_XOFFSET);
	printf("\t\t\t(if not specified, default is %d for NTSC, %d for PAL)\n",
			DEFAULT_NTSC_XOFFSET, DEFAULT_PAL_XOFFSET);
	printf("-yOFFSET\tMove image up/down. Valid range: %d..%d\n",
			MIN_YOFFSET, MAX_YOFFSET);
	printf("\t\t\t(if not specified, default is %d for NTSC, %d for PAL)\n",
			DEFAULT_NTSC_YOFFSET, DEFAULT_PAL_YOFFSET);
	printf("-XSCALE\t\tScale image horizontally SCALE%%. Valid range: %d..%d\n",
			MIN_XSCALE, MAX_XSCALE);
	printf("\t\t\t(if not specified, default is 100)\n");
	printf("-YSCALE\t\tScale image vertically SCALE%%. Valid range: %d..%d\n",
			MIN_YSCALE, MAX_YSCALE);
	printf("-vxSCALE\tScale YUV video image horizontally SCALE%%. Valid range: %d..%d\n",
			MIN_XSCALE, MAX_XSCALE);
	printf("\t\t\t(if not specified, default is 100)\n");
	printf("-vySCALE\tScale YUV video image vertically SCALE%%. Valid range: %d..%d\n",
			MIN_YSCALE, MAX_YSCALE);
	printf("\t\t\t(if not specified, default is 100)\n");
	printf("-V\t\tTweak YUV video layer as well as RBG layer.\n");
	printf("-dSECS\t\tSleep this many seconds inbetween tweaks.\n");
	printf("\t\t\tValid range: %d..%d   Default is %d seconds\n", 
			MIN_DELAY, MAX_DELAY, DEFAULT_DELAY);
	printf("-DSECS\t\tSleep this many seconds before the first tweak.\n");
	printf("\t\t\tValid range: %d..%d   Default is %d seconds\n", 
			MIN_FIRST_DELAY, MAX_FIRST_DELAY, DEFAULT_FIRST_DELAY);
	printf("-t\t\tOnly tweak once and then terminate\n");
	printf("\t\t\t(if not specified, tweak infinitely)\n");
}

int main(int argc, char *argv[])
{
	// We are gonna be killall'd a lot, so try to be submissive:
	signal(SIGINT,&exit);
	signal(SIGTERM, &exit);
	// but don't disappear when run from shell scripts
	signal(SIGHUP, SIG_IGN);

	if (!initphys()) {
		printf("Error mapping registers!\n");
		printf("Exiting..\n");
		return 1;
	}

	int cur_arg = 1;
	while (cur_arg < argc)
	{
		// We have been passed command-line parameters
		switch (argv[cur_arg][1])
		{
			case 'h':
				displayhelp();
				return(0);
				break;
			case 'p':
				mode = PAL;
				break;
			case 'P':
				pal_overscan_fix = 1;
				break;
			case 'e':
				enable_tvmode = 1;
				break;
//			case 's':
//				stubborn_fix = 1;
//				break;
			case 'S':
				scaling_tweak = 0;
				break;
			case 'v':
				switch (argv[cur_arg][2])
				{
					case 'x':
						if (strlen(&argv[cur_arg][3]) <= 3)
						{
							vxscale_percent = atoi(&argv[cur_arg][3]);
							if (vxscale_percent < MIN_VXSCALE || vxscale_percent > MAX_VXSCALE)
							{
								printf("ERROR: Video X-axis scaling percentage out of range\n\n");
								displayhelp();
								exit(1);
							}
						} else 
						{ 
							printf("ERROR: bad video X-axis scaling percentage\n\n");
							displayhelp();
							exit(1);
						}
						break;
					case 'y':
						if (strlen(&argv[cur_arg][3]) <= 3)
						{
							vyscale_percent = atoi(&argv[cur_arg][3]);
							if (vyscale_percent < MIN_VYSCALE || vyscale_percent > MAX_VYSCALE)
							{
								printf("ERROR: Video Y-axis scaling percentage out of range\n\n");
								displayhelp();
								exit(1);
							}
						} else 
						{ 
							printf("ERROR: bad video Y-axis scaling percentage\n\n");
							displayhelp();
							exit(1);
						}
						break;
					default:
						printf("ERROR: bad video scaling parameter\n\n");
						displayhelp();
						exit(1);
						break;
				}
				break;
			case 'V':
				tweak_yuv = 1;
				break;
			case 't':
				tweak_only_once = 1;
				break;
//			case 'f':
//				if (strlen(&argv[cur_arg][2]) <= 4)
//				{
//					pitch = atoi(&argv[cur_arg][2]);
//					if (pitch < MIN_PITCH || pitch > MAX_PITCH)
//					{
//						printf("ERROR: pitch out of range\n\n");
//						displayhelp();
//						exit(1);
//					}
//				} else 
//				{ 
//					printf("ERROR: bad pitch\n\n");
//					displayhelp();
//					exit(1);
//				}
//				break;
			case 'x':
				if (strlen(&argv[cur_arg][2]) <= 3)
				{
					xoffset = atoi(&argv[cur_arg][2]);
					if (xoffset < MIN_XOFFSET || xoffset > MAX_XOFFSET)
					{
						printf("ERROR: X offset out of range\n\n");
						displayhelp();
						exit(1);
					}
				} else 
				{ 
					printf("ERROR: bad X offset\n\n");
					displayhelp();
					exit(1);
				}
				break;
			case 'y':
				if (strlen(&argv[cur_arg][2]) <= 3)
				{
					yoffset = atoi(&argv[cur_arg][2]);
					if (yoffset < MIN_YOFFSET || yoffset > MAX_YOFFSET)
					{
						printf("ERROR: Y offset out of range\n\n");
						displayhelp();
						exit(1);
					}
				} else 
				{ 
					printf("ERROR: bad Y offset\n\n");
					displayhelp();
					exit(1);
				}
				break;
			case 'X':
				if (strlen(&argv[cur_arg][2]) <= 3)
				{
					xscale_percent = atoi(&argv[cur_arg][2]);
					if (xscale_percent < MIN_XSCALE || xscale_percent > MAX_XSCALE)
					{
						printf("ERROR: X-axis scaling percentage out of range\n\n");
						displayhelp();
						exit(1);
					}
				} else 
				{ 
					printf("ERROR: bad X-axis scaling percentage\n\n");
					displayhelp();
					exit(1);
				}
				break;
			case 'Y':
				if (strlen(&argv[cur_arg][2]) <= 3)
				{
					yscale_percent = atoi(&argv[cur_arg][2]);
					if (yscale_percent < MIN_YSCALE || yscale_percent > MAX_YSCALE)
					{
						printf("ERROR: Y-axis scaling percentage out of range\n\n");
						displayhelp();
						exit(1);
					}
				} else 
				{ 
					printf("ERROR: bad Y-axis scaling percentage\n\n");
					displayhelp();
					exit(1);
				}
				break;
			case 'd':
				if (strlen(&argv[cur_arg][2]) <= 3)
				{
					delay = atoi(&argv[cur_arg][2]);
					if (delay < MIN_DELAY || delay > MAX_DELAY)
					{
						printf("ERROR: delay out of range\n\n");
						displayhelp();
						exit(1);
					}
				} else 
				{ 
					printf("ERROR: bad delay given\n\n");
					displayhelp();
					exit(1);
				}
				break;
			case 'D':
				if (strlen(&argv[cur_arg][2]) <= 3)
				{
					first_delay = atoi(&argv[cur_arg][2]);
					if (first_delay < MIN_FIRST_DELAY || first_delay > MAX_FIRST_DELAY)
					{
						printf("ERROR: 'first delay' out of range\n\n");
						displayhelp();
						exit(1);
					}
				} else 
				{ 
					printf("ERROR: bad 'first delay' given\n\n");
					displayhelp();
					exit(1);
				}
				break;
			default:
				// invalid parameter
				printf("ERROR: invalid parameter passed\n\n");
				displayhelp();
				exit(1);
		} // switch
		cur_arg++;
	} // while (argc > 1) 

	if (xoffset == 999)
	{
		// parameter wasn't specified, set to default for the given mode
		xoffset = (mode == PAL ? DEFAULT_PAL_XOFFSET : DEFAULT_NTSC_XOFFSET);
	}

	if (yoffset == 999)
	{
		// parameter wasn't specified, set to default for the given mode
		yoffset = (mode == PAL ? DEFAULT_PAL_YOFFSET : DEFAULT_NTSC_YOFFSET);
	}

	// main loop
	loop();

	closephys();
	return 0;
}

