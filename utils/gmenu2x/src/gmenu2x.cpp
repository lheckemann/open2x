/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo   *
 *   massimiliano.torromeo@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <iostream>
//senquack - needed for bugfix involving skin.conf:
#include <iomanip>


#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <signal.h>

#include <SDL_gp2x.h>

//senquack - tvout stuff:
#include "mmsp2_regs.h"

//senquack - Open2X defines
#include "open2x.h"

#include <sys/statvfs.h>
#include <errno.h>

#include "gp2x.h"
#include <sys/fcntl.h> //for battery

//for browsing the filesystem
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

//for soundcard
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "linkapp.h"
#include "linkaction.h"
#include "menu.h"
#include "asfont.h"
#include "sfontplus.h"
#include "surface.h"
#include "filedialog.h"
#include "gmenu2x.h"
#include "filelister.h"

//senquack - added for access to getDiskFree()
#include "utilities.h"

#include "iconbutton.h"
#include "messagebox.h"
#include "inputdialog.h"
#include "settingsdialog.h"
#include "wallpaperdialog.h"
#include "textdialog.h"
#include "menusettingint.h"
#include "menusettingbool.h"
#include "menusettingrgba.h"
#include "menusettingstring.h"
#include "menusettingmultistring.h"
#include "menusettingfile.h"
#include "menusettingimage.h"
#include "menusettingdir.h"


#include <sys/mman.h>

#ifdef TARGET_PANDORA
//#include <pnd_container.h>
//#include <pnd_conf.h>
//#include <pnd_discovery.h>
#endif

using namespace std;
using namespace fastdelegate;

int main(int argc, char *argv[]) {
	cout << "----" << endl;
	cout << "GMenu2X starting: If you read this message in the logs, check http://gmenu2x.sourceforge.net/page/Troubleshooting for a solution" << endl;
	cout << "----" << endl;

	signal(SIGINT,&exit);
	GMenu2X app(argc,argv);
	return 0;
}

void GMenu2X::gp2x_init() {
#ifdef TARGET_GP2X
	gp2x_mem = open("/dev/mem", O_RDWR);
	gp2x_memregs=(unsigned short *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_mem, 0xc0000000);
	MEM_REG=&gp2x_memregs[0];

	batteryHandle = open(f200 ? "/dev/mmsp2adc" : "/dev/batt", O_RDONLY);
	if (f200) {
		//if wm97xx fails to open, set f200 to false to prevent any further access to the touchscreen
		f200 = ts.init();
	}

	gp2x_tv_mode = gp2x_memregs[0x2800>>1] & 0x100;
	
#endif
}

void GMenu2X::gp2x_deinit() {
#ifdef TARGET_GP2X
	if (gp2x_mem!=0) {
	//senquack
//		gp2x_memregs[0x28DA>>1]=0x4AB;
//		gp2x_memregs[0x290C>>1]=640;
		close(gp2x_mem);
	}
	if (batteryHandle!=0) close(batteryHandle);
	if (f200) ts.deinit();

	if (cx25874)
	{
		close(cx25874);
		cx25874 = 0;
	}
#endif
}

//senquack - pulled from Rlyeh's minlib for improved TVout:
// NOTE: this appears to wait indefinitely, might not be compatible with SDL, disabling.
/* Function: gp2x_video_waitvsync
   This function halts the program until a vertical sync is done.

   See also:
   <gp2x_video_waithsync> 

   Credits:
   rlyeh (original code)
   K-teto (fixed this function for firmware 2.0.0) */

//void GMenu2X::gp2x_video_waitvsync(void)
//{
//    while(  gp2x_memregs[0x1182>>1]&(1<<4))
//    {
//        asm volatile ("nop");
//        asm volatile ("nop");
//        asm volatile ("nop");
//        asm volatile ("nop");
//
//        //asm volatile ("" ::: "memory");
//    }
//
//    while(!(gp2x_memregs[0x1182>>1]&(1<<4)))
//    {
//        asm volatile ("nop");
//        asm volatile ("nop");
//        asm volatile ("nop");
//        asm volatile ("nop");
//
//        //asm volatile ("" ::: "memory");
//    }
//}

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

void GMenu2X::gp2x_i2c_write(unsigned char id, unsigned char addr, unsigned char data)
{
#ifdef TARGET_GP2X
 i2cw a;
 a.id = id, a.addr = addr, a.data = data;
 ioctl(cx25874, _IOW('v', 0x00, i2cw), &a); //CX25874_I2C_WRITE_BYTE
#endif
}

//senquack - pulled from Rlyeh's minlib for improved TVout:
/* Function: gp2x_i2c_read
   This function reads a byte from an I2C address.
   
   Parameters:
   id (0..) - i2c ID
   addr - address to write data to.
   
   See also:
   <gp2x_i2c_write>
   
   Credits:
   rlyeh */

unsigned char GMenu2X::gp2x_i2c_read(unsigned char id, unsigned char addr)
{
#ifdef TARGET_GP2X
 unsigned char temp;
 i2cr a;
 a.id = id, a.addr = addr, a.pdata = &temp ;
 ioctl(cx25874, _IOW('v', 0x01, i2cr), &a); //CX25874_I2C_READ_BYTE

 return (*a.pdata);
#endif
}

//senquack - pulled from Rlyeh's minlib for improved TVout:
//int GMenu2X::gp2x_tv_getmode(void)
//{
//// return gp2x_tv_lastmode;
//	return gp2x_tv_mode;	// LCD, PAL or NTSC
//}

//senquack - pulled from Rlyeh's minlib for improved TVout (modified slightly)
/* Function: gp2x_misc_lcd
   This function enables or disables the LCD backlight.

   Parameters:
   on (0..1) - turns LCD backlight off (0) or on (1)  

   Credits:
   RobBrown, Coder_TimT */
 
void GMenu2X::gp2x_misc_lcd(int on)
{
#ifdef TARGET_GP2X
 if(f200)
 {
  if(on) gp2x_memregs[0x1076 >> 1] |= 0x0800; else gp2x_memregs[0x1076 >> 1] &= ~0x0800;
 }
 else
 {
  if(on) gp2x_memregs[0x106E>>1] |= 4; else gp2x_memregs[0x106E>>1] &= ~4;
 }
#endif
}


void GMenu2X::gp2x_video_RGB_setscaling(int W, int H)
{
#ifdef TARGET_GP2X
  int bpp=(gp2x_memregs[0x28DA>>1]>>9)&0x3;

  float mul = (gp2x_memregs[0x2800>>1] & 0x100 ? 512.0 : 1024.0);
  
  // scale horizontal
  gp2x_memregs[0x2906>>1]=(unsigned short)((float)mul *(W/320.0));
  // scale vertical
//  gp2x_memregs[0x2908>>1]=(unsigned  long)((float)320.0*bpp *(H/240.0));
  gp2x_memregs[0x2908>>1]=(unsigned  short)((float)320.0*bpp *(H/240.0));
	gp2x_memregs[0x290A>>1]=0;
#endif
}

//senquack - pulled from Rlyeh's minlib for improved TVout:
/* Function: gp2x_tv_setmode
   This function set TV out mode on or off.
   
   Parameters:
   mode - set mode to <LCD>, <PAL> or <NTSC>. 
   addr - address to write data to.
   
   See also:
   <gp2x_tv_adjust>
   
   Credits:
   rlyeh */

//void GMenu2X::gp2x_tv_setmode(unsigned char mode)
//{
//#ifdef TARGET_GP2X
// if(mode != LCD && mode != PAL && mode != NTSC) return;
// 
// gp2x_tv_lastmode = mode;
//  
//// if(!gp2x_dev[0]) gp2x_dev[0] = open("/dev/cx25874",O_RDWR);
// if(!cx25874) cx25874 = open("/dev/cx25874",O_RDWR);
//
// gp2x_misc_lcd(1);
// 
// if(mode == LCD)
// {
//  ioctl(cx25874, _IOW('v', 0x02, unsigned char), 0);
//  close(cx25874);
//  cx25874 = 0;
//   
//  return;
// }
// 
// ioctl(cx25874, _IOW('v', 0x02, unsigned char), mode);
//  
// //gp2x_video_RGB_setwindows(0x11,-1,-1,-1,319,239);              
// gp2x_video_RGB_setscaling(320,240); 
// //gp2x_video_YUV_setparts(-1,-1,-1,-1,319,239);
// gp2x_video_YUV_setscaling(0,320,240);
// gp2x_video_YUV_setscaling(1,320,240);
// gp2x_video_YUV_setscaling(2,320,240);
// gp2x_video_YUV_setscaling(3,320,240);
//  
// gp2x_misc_lcd(0);
//
// if(mode == NTSC)
// {
//    gp2x_tv_adjust(25, -7);
// }
// else
// {
//    gp2x_tv_adjust(25,  20);
// }
//
// //senquack - experiment (this seems to allow the tv to show the whole screen instead of half
// //now, but it still flickers on the vertical axis
////		gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1];
////		gp2x_memregs[0x28E8>>1]=239;
// 
//#endif
//}

////senquack - pulled from Rlyeh's minlib for improved TVout:
///* Function: gp2x_tv_adjust
//   This function adjusts and centers screen to TV.
//   
//   Parameters:
//   horizontal (-50..0..35) - number of pixels to move the image horizontally.
//   vertical (-15..0..45) - number of pixels to move the image vertically.
//   
//   Note:
//   - horizontal and vertical are both signed.
//   - default setting for NTSC is (16, -7) 
//   - default setting for PAL  is (16, 19)
//   
//   See also:
//   <gp2x_tv_setmode>
//   
//   Credits:
//   rlyeh */
//
//void GMenu2X::gp2x_tv_adjust(signed char horizontal, signed char vertical)
//{
//#ifdef TARGET_GP2X
//    int lines, syncs_start, syncs_end;
//
//    //horizontal adjustment
////    gp2x_video_waitvsync();
//    gp2x_cx25784_write(0x8c, (unsigned char) (50 - 11 + 3 - horizontal) );
//    
//    //vertical adjustment
//    if(gp2x_tv_mode == PAL) 
//        lines = 288, syncs_start = 1, syncs_end = 24; 
//    else 
//        lines = 240, syncs_start = 1, syncs_end = 22;
//    
//    lines -= vertical, syncs_end += vertical;
//       
////    gp2x_video_waitvsync();
//    gp2x_memregs[0x2818 >> 1]  =  lines - 1;
//    gp2x_memregs[0x2820 >> 1] &= (0xFF00);
//    gp2x_memregs[0x2820 >> 1] |= (syncs_start << 8);
//    gp2x_memregs[0x2822 >> 1] &= ~(0x1FF);
//    gp2x_memregs[0x2822 >> 1] |=  syncs_end; // syncs_end = verBackPorch+verFontPorch - 1
//
//    if(gp2x_tv_mode == PAL)
//    {
//     //bottom screen image cut off (PAL 320x288 full -> PAL 320x240 centered w/ black borders)
//     int real_lines = 288, 
//         wanted_lines = 240, 
//         top_spacing = (real_lines - wanted_lines) / 2,
//         active_lines = wanted_lines + top_spacing;
//     
////     gp2x_video_waitvsync();
//    
//     active_lines += -top_spacing +5 + vertical;
//     
//     gp2x_cx25784_write(0x84, active_lines & 0xFF); //reduce overscan, VACTIVE_0
//     gp2x_cx25784_write(0x86, 0x26 | ((active_lines & 0x100)>>1) );
//      
//     gp2x_cx25784_write(0x94, active_lines & 0xFF); //reduce overscan, VACTIVE_1
//     gp2x_cx25784_write(0x96, 0x31 | ((active_lines & 0x300)>>8) );
//    }
//#endif
//}

//senquack - first two are originals:
//void GMenu2X::gp2x_tvout_on(bool pal) {
//#ifdef TARGET_GP2X
//	if (gp2x_mem!=0) {
//		/*Ioctl_Dummy_t *msg;
//		int TVHandle = ioctl(SDL_videofd, FBMMSP2CTRL, msg);*/
//		if (cx25874!=0) gp2x_tvout_off();
//		//if tv-out is enabled without cx25874 open, stop
//		//if (gp2x_memregs[0x2800>>1]&0x100) return;
//		cx25874 = open("/dev/cx25874",O_RDWR);
//		ioctl(cx25874, _IOW('v', 0x02, unsigned char), pal ? 4 : 3);
//		gp2x_memregs[0x2906>>1]=512;
//		gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1];
//		//		senquack - trying to fix vertical flickering:
////		gp2x_memregs[0x28E8>>1]=239;
//	}
//#endif
//}
//
//void GMenu2X::gp2x_tvout_off() {
//#ifdef TARGET_GP2X
//	if (gp2x_mem!=0) {
//		close(cx25874);
//		cx25874 = 0;
//		gp2x_memregs[0x2906>>1]=1024;
//	}
//#endif
//}
//void GMenu2X::gp2x_tvout_on(bool pal) {
//#ifdef TARGET_GP2X
//	
//	SDL_GP2X_TV(1);
//	SDL_GP2X_TVMode(pal ? PAL : NTSC);
////	gp2x_tv_setmode(pal ? PAL : NTSC);
//	
//	//relaunch GMenu2X:
////		SDL_Quit();
////		chdir(getExePath().c_str());
////		execlp("./gmenu2x", "./gmenu2x", NULL);
//#endif
//}
//
//void GMenu2X::gp2x_tvout_off() {
//#ifdef TARGET_GP2X
////	gp2x_tv_setmode(LCD);
//	SDL_GP2X_TVMode(LCD);
//	SDL_GP2X_TV(0);
//
//	//relaunch GMenu2X:
////		SDL_Quit();
////		chdir(getExePath().c_str());
////		execlp("./gmenu2x", "./gmenu2x", NULL);
//#endif
//}
void GMenu2X::gp2x_tvout_on(bool pal) {
#ifdef TARGET_GP2X
	unsigned int bytes_per_pixel = (gp2x_memregs[0x28DA>>1]>>9)&0x3;
	unsigned int width = 320;
	unsigned int height = 240;
	unsigned int pitch, phys_pitch;

	cx25874 = open("/dev/cx25874",O_RDWR);
//	open special open2x device driver that doesn't reset the hardware on opening
//	cx25874 = open("/dev/cx25874_open2x",o_rdwr);
	if (cx25874 == -1)
	{
		cx25874 = 0;
#if debug
		printf("error opening /dev/cx25874.\n");
#endif
		return;
	}

	ioctl(cx25874, _IOW('v', 0x02, unsigned char), pal ? PAL : NTSC);

	close(cx25874);

	//relaunch GMenu2X (screen is garbled until we do).. we'll tweak it after restart
	gp2x_deinit();
	SDL_Quit();
	chdir(getExePath().c_str());
	execlp("./gmenu2x", "./gmenu2x", NULL);

//	gp2x_tv_mode = true;
//	
//	if (bytes_per_pixel == 1)
//	{
//		// in 8bpp mode
//		printf("8bpp tweak\n");
//		pitch = phys_pitch = width;
//		gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1] << 1;
//	} else 
//	{
//		// in 16bpp mode
//		printf("16bpp tweak\n");
//		pitch = phys_pitch = width << 1;
////		gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1];
//		gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1] << 1;
//	}
//		
////	int w_passed = (long)(320.0 * (100.0 / (float)xscale_percent));
////	int h_passed = (long)(240.0 * (100.0 / (float)yscale_percent)); 
////	gp2x_video_RGB_setscaling( w_passed, h_passed );
////senquack -for now ,just pass 320x240
//	gp2x_video_RGB_setscaling( 320, 240 );
//
//	gp2x_memregs[MLC_STL_HW] = phys_pitch;
//  gp2x_memregs[MLC_STL_CNTL] = MLC_STL_BPP_16 | MLC_STL1ACT;
//  gp2x_memregs[MLC_STL_MIXMUX] = 0;
//  gp2x_memregs[MLC_STL_ALPHAL] = 255;
//  gp2x_memregs[MLC_STL_ALPHAH] = 255;
//  gp2x_memregs[MLC_OVLAY_CNTR] |= DISP_STL1EN;
//
//	//		Rlyeh's tv image centering:
//	int xoffset, yoffset;
//	if (pal)
//	{
//		cout << "pal mode" << endl;
//		xoffset = confInt["tvoutXOffsetPAL"];
//		yoffset = confInt["tvoutYOffsetPAL"];
//	} else
//	{
//		cout << "ntsc mode" << endl;
//		xoffset = confInt["tvoutXOffsetNTSC"];
//		yoffset = confInt["tvoutYOffsetNTSC"];
//	}
//
//	 int lines, syncs_start, syncs_end;
//
//	 //horizontal adjustment
//	//    gp2x_video_waitvsync();
//	 gp2x_cx25874_write(0x8c, (unsigned char) (50 - 11 + 3 - xoffset) );
//	 
//	 //vertical adjustment
//	//    if(gp2x_tv_getmode() == PAL) 
//	 if(pal) 
//		  lines = 288, syncs_start = 1, syncs_end = 24; 
//	 else 
//		  lines = 240, syncs_start = 1, syncs_end = 22;
//	 
//	 lines -= yoffset, syncs_end += yoffset;
//		 
//	//    gp2x_video_waitvsync();
//	 gp2x_memregs[0x2818 >> 1]  =  lines - 1;
//	 gp2x_memregs[0x2820 >> 1] &= (0xFF00);
//	 gp2x_memregs[0x2820 >> 1] |= (syncs_start << 8);
//	 gp2x_memregs[0x2822 >> 1] &= ~(0x1FF);
//	 gp2x_memregs[0x2822 >> 1] |=  syncs_end; // syncs_end = verBackPorch+verFontPorch - 1
//
//	 if(pal)
//	 {
//	  //bottom screen image cut off (PAL 320x288 full -> PAL 320x240 centered w/ black borders)
//	  int real_lines = 288, 
//			wanted_lines = 240, 
//			top_spacing = (real_lines - wanted_lines) / 2,
//			active_lines = wanted_lines + top_spacing;
//	  
//	//     gp2x_video_waitvsync();
//	  active_lines += -top_spacing +5 + yoffset;
//	  
//	  gp2x_cx25874_write(0x84, active_lines & 0xFF); //reduce overscan, VACTIVE_0
//	  gp2x_cx25874_write(0x86, 0x26 | ((active_lines & 0x100)>>1) );
//		
//	  gp2x_cx25874_write(0x94, active_lines & 0xFF); //reduce overscan, VACTIVE_1
//	  gp2x_cx25874_write(0x96, 0x31 | ((active_lines & 0x300)>>8) );
//	}
//
//	close(cx25874);
//	cx25874 = 0;
#endif
}

void GMenu2X::gp2x_tvout_off() {
#ifdef TARGET_GP2X
	gp2x_tv_mode = false;

	if (cx25874 != 0)
		close(cx25874);

	cx25874 = open("/dev/cx25874",O_RDWR);
	if (cx25874 != -1)
	{
		ioctl(cx25874, _IOW('v', 0x02, unsigned char), LCD);
		gp2x_video_RGB_setscaling( 320, 240 );
		close(cx25874);
		cx25874=0;	
	}
	gp2x_misc_lcd(1);		// turn backlight back on

	return;
#endif
}

//senquack - TV out 
void GMenu2X::toggleTvOut() {
#ifdef TARGET_GP2X
//senquack
//	if (cx25874!=0)
	if (gp2x_tv_mode)
		gp2x_tvout_off();
	else
		gp2x_tvout_on(confStr["tvoutEncoding"] == "PAL");
#endif
}

////senquack - new function to make tv out better
//void GMenu2X::tweakTvOut(bool pal)	{
//#ifdef TARGET_GP2X
//	if (gp2x_tv_mode == LCD)
//	{
//		cout << "Cannot tweak TV, not in TV mode." << endl;
//		return;
//	}
//
//	if (gp2x_mem!=0) {
//		cout << "Tweaking TV output" << endl;
//		
//		if (!cx25874)
//		{
//			cx25874 = open("/dev/cx25874",O_RDWR);
//			if (cx25874 == -1)
//			{
//				cx25874 = 0;
//				cout << "Error opening /dev/cx25874" << endl;
//				return;
//			}
//		}
//
//	  unsigned int phys_width = gp2x_memregs[DPC_X_MAX] + 1;
//	  unsigned int phys_height = gp2x_memregs[DPC_Y_MAX] + 1;
//	  unsigned int phys_ilace = (gp2x_memregs[DPC_CNTL] & DPC_INTERLACE) ? 1 : 0;
//	  // Set up the new mode framebuffer, making sanity adjustments
//	  // 64 <= width <= 1024, multiples of 8 only
//	  //senquack - note: changing this causes the screen to be totally corrupted
//	  unsigned int width = 320;
//
//		width = (width + 7) & 0x7f8;
//	  if (width < 64) width = 64;
//	  if (width > 1024) width = 1024;
//
//	  // 64 <= height <= 768
//	  unsigned int height = 240;
//	  //  senquack - adding 8 or 16 here allows the entire screen to be displayed but with cutout
//	  //  lines
//	//  unsigned int height = 256;
//	  if (height < 64) height = 64;
//	  if (height > 768) height = 768;
//
//	  unsigned int phys_pitch;
//	  unsigned int pitch = phys_pitch = width * 2;
//
//	  //	senquack-enabling this causes pixels to be doubled horizontally:
////	  phys_pitch *= 2;
//
//	  unsigned int scale_x = (1024 * width) / phys_width;
//	  // and y-scale is scale * pitch
//	  unsigned int scale_y = (height * pitch) / phys_height;
//	  // xscale and yscale are set so that virtual_x * xscale = phys_x (16.16)
//	  unsigned int xscale = (phys_width << 16) / width;
//	  unsigned int yscale = (phys_height << 16) / height;
//	  //	  senquack - rgb_setscaling sets these two registers, let's try it instead
////	  gp2x_memregs[MLC_STL_HSC] = scale_x;
////	  gp2x_memregs[MLC_STL_VSCL] = scale_y & 0xffff;
//	  gp2x_video_RGB_setscaling(320, 240);
//
//	  gp2x_memregs[MLC_STL_VSCH] = scale_y >> 16;
//	  gp2x_memregs[MLC_STL_HW] = phys_pitch;
////	  gp2x_memregs[MLC_STL_CNTL] = MLC_STL_BPP_16 | MLC_STL1ACT;
//	  
//	  //senquack - without this, only half the screen is shown:
//		gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1];
//	  
//	//		Rlyeh's tvadjust:
//
//		// load Rlyeh's default offsets
//		int horizontal, vertical;
////		if (pal)
////		{
////			horizontal = 16; vertical = 19;
////		} else
////		{
////			//senquack - on my NTSC TV, 10 is better than 16 for horizontal:
////			horizontal = 10, vertical = -7;
////		}
//		if (pal)
//		{
//			cout << "pal mode" << endl;
//			horizontal = confInt["tvoutXOffsetPAL"];
//		  	vertical = confInt["tvoutYOffsetPAL"];
//		} else
//		{
//			cout << "ntsc mode" << endl;
//			horizontal = confInt["tvoutXOffsetNTSC"];
//		  	vertical = confInt["tvoutYOffsetNTSC"];
//		}
//		cout << "tweaks: horiz= " << horizontal << " vert= " << vertical << endl;
//
//		 int lines, syncs_start, syncs_end;
//
//		 //horizontal adjustment
//	//    gp2x_video_waitvsync();
//		 gp2x_cx25784_write(0x8c, (unsigned char) (50 - 11 + 3 - horizontal) );
//		 
//		 //vertical adjustment
//	//    if(gp2x_tv_getmode() == PAL) 
//		 if(pal) 
//			  lines = 288, syncs_start = 1, syncs_end = 24; 
//		 else 
//			  lines = 240, syncs_start = 1, syncs_end = 22;
//		 
//		 lines -= vertical, syncs_end += vertical;
//			 
//	//    gp2x_video_waitvsync();
//		 gp2x_memregs[0x2818 >> 1]  =  lines - 1;
//		 gp2x_memregs[0x2820 >> 1] &= (0xFF00);
//		 gp2x_memregs[0x2820 >> 1] |= (syncs_start << 8);
//		 gp2x_memregs[0x2822 >> 1] &= ~(0x1FF);
//		 gp2x_memregs[0x2822 >> 1] |=  syncs_end; // syncs_end = verBackPorch+verFontPorch - 1
//
//	//    if(gp2x_tv_getmode() == PAL)
//		 if(pal)
//		 {
//		  //bottom screen image cut off (PAL 320x288 full -> PAL 320x240 centered w/ black borders)
//		  int real_lines = 288, 
//				wanted_lines = 240, 
//				top_spacing = (real_lines - wanted_lines) / 2,
//				active_lines = wanted_lines + top_spacing;
//		  
//	//     gp2x_video_waitvsync();
//		 
//		  active_lines += -top_spacing +5 + vertical;
//		  
//		  gp2x_cx25784_write(0x84, active_lines & 0xFF); //reduce overscan, VACTIVE_0
//		  gp2x_cx25784_write(0x86, 0x26 | ((active_lines & 0x100)>>1) );
//			
//		  gp2x_cx25784_write(0x94, active_lines & 0xFF); //reduce overscan, VACTIVE_1
//		  gp2x_cx25784_write(0x96, 0x31 | ((active_lines & 0x300)>>8) );
//		 }
//	}
//#endif
//}
//senquack - new function to make tv out better
void GMenu2X::tweakTvOut(bool pal)	{
#ifdef TARGET_GP2X
	if (cx25874 != 0)
		close(cx25874);

// Every time you open this stupid device is resets itself. Must reset display mode first thing.
	cx25874 = open("/dev/cx25874",O_RDWR);
	if (cx25874 != -1)
	{
		ioctl(cx25874, _IOW('v', 0x02, unsigned char), pal ? PAL : NTSC);
	}

	unsigned int bytes_per_pixel = (gp2x_memregs[0x28DA>>1]>>9)&0x3;
	unsigned int width = 320;
	unsigned int height = 240;
	unsigned int pitch, phys_pitch;

	if (bytes_per_pixel == 1)
	{
		// in 8bpp mode
		printf("8bpp tweak\n");
		pitch = phys_pitch = width;
		// pretty sure endx should be one less than this (after messing more with tv out)
//		gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1] << 1;
		gp2x_memregs[0x28E4>>1] = (gp2x_memregs[0x290C>>1] << 1) - 1;
	} else 
	{
		// in 16bpp mode
		printf("16bpp tweak\n");
		pitch = phys_pitch = width << 1;
		// pretty sure endx should be one less than this (after messing more with tv out)
//		gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1];
		gp2x_memregs[0x28E4>>1] = (gp2x_memregs[0x290C>>1]) - 1;
	}
		
//	int w_passed = (long)(320.0 * (100.0 / (float)xscale_percent));
//	int h_passed = (long)(240.0 * (100.0 / (float)yscale_percent)); 
//	gp2x_video_RGB_setscaling( w_passed, h_passed );
//senquack -for now ,just pass 320x240
	int xscale_percent = confInt["tvoutXScale"];
	if (xscale_percent < TV_MIN_XSCALE || xscale_percent > TV_MAX_YSCALE)
	{
		xscale_percent = confInt["tvoutXScale"] = 100;
	}
	int yscale_percent = confInt["tvoutYScale"];
	if (yscale_percent < TV_MIN_YSCALE || yscale_percent > TV_MAX_YSCALE)
	{
		yscale_percent = confInt["tvoutYScale"] = 100;
	}

//	gp2x_video_RGB_setscaling( 320, 240 );
	int w_passed = (long)(320.0 * (100.0 / (float)xscale_percent));
	int h_passed = (long)(240.0 * (100.0 / (float)yscale_percent)); 
	gp2x_video_RGB_setscaling( w_passed, h_passed );

	//		Rlyeh's tv image centering:
	int xoffset, yoffset;
	if (pal)
	{
		cout << "pal mode" << endl;
		xoffset = confInt["tvoutXOffsetPAL"];
		yoffset = confInt["tvoutYOffsetPAL"];
	} else
	{
		cout << "ntsc mode" << endl;
		xoffset = confInt["tvoutXOffsetNTSC"];
		yoffset = confInt["tvoutYOffsetNTSC"];
	}

	 int lines, syncs_start, syncs_end;

	 //horizontal adjustment
	//    gp2x_video_waitvsync();
	 gp2x_cx25874_write(0x8c, (unsigned char) (50 - 11 + 3 - xoffset) );
	 
	 //vertical adjustment
	//    if(gp2x_tv_getmode() == PAL) 
	 if(pal) 
		  lines = 288, syncs_start = 1, syncs_end = 24; 
	 else 
		  lines = 240, syncs_start = 1, syncs_end = 22;
	 
	 lines -= yoffset, syncs_end += yoffset;
		 
	//    gp2x_video_waitvsync();
	 gp2x_memregs[0x2818 >> 1]  =  lines - 1;
	 gp2x_memregs[0x2820 >> 1] &= (0xFF00);
	 gp2x_memregs[0x2820 >> 1] |= (syncs_start << 8);
	 gp2x_memregs[0x2822 >> 1] &= ~(0x1FF);
	 gp2x_memregs[0x2822 >> 1] |=  syncs_end; // syncs_end = verBackPorch+verFontPorch - 1

	 //	 Changed this block to be optional and by default, off, because PAL users
	 //	 report problems with cut-off bottom image.  Not completely sure if this has
	 //	 anything to do with it yet, though:
//	 if(pal)
	 if(pal && confInt["tvoutPalOverscanFix"])
	 {
	  //bottom screen image cut off (PAL 320x288 full -> PAL 320x240 centered w/ black borders)
	  int real_lines = 288, 
			wanted_lines = 240, 
			top_spacing = (real_lines - wanted_lines) / 2,
			active_lines = wanted_lines + top_spacing;
	  
	//     gp2x_video_waitvsync();
	  active_lines += -top_spacing +5 + yoffset;
	  
	  gp2x_cx25874_write(0x84, active_lines & 0xFF); //reduce overscan, VACTIVE_0
	  gp2x_cx25874_write(0x86, 0x26 | ((active_lines & 0x100)>>1) );
		
	  gp2x_cx25874_write(0x94, active_lines & 0xFF); //reduce overscan, VACTIVE_1
	  gp2x_cx25874_write(0x96, 0x31 | ((active_lines & 0x300)>>8) );
	}

	close(cx25874);
	cx25874 = 0;
}


GMenu2X::GMenu2X(int argc, char *argv[]) {
	//senquack - added ability to detect Open2X fw version number
//	//Detect firmware version and type
//	if (fileExists("/etc/open2x")) {
//		fwType = "open2x";
//		fwVersion = "";
//	} else {
//		fwType = "gph";
//		fwVersion = "";
//	}
	if (fileExists(OPEN2X_VERSION_FILENAME)) {
		fwType = "open2x";
//		ifstream inf(OPEN2X_VERSION_FILENAME.c_str());
		ifstream inf(OPEN2X_VERSION_FILENAME);
		if (inf.is_open()) {
			string line;
			if (!inf.eof() )
			{
				getline (inf,line);
				fwVersion = trim(line);
			}
			inf.close();
		} else {
			fwVersion = "";
		}
	} else {
		fwType = "gph";
		fwVersion = "";
	}

#ifdef TARGET_GP2X
	f200 = fileExists("/dev/touchscreen/wm97xx");
#else
	f200 = true;
#endif

	//senquack - New functions used under Open2X.  Everytime GMenu2X starts, it takes a look at all processes
	//		currently running.  It tells the kernel all the PIDs it finds (excluding GMenu2X itself).  The kernel
	//		keeps this list in memory when programs are run.  If the user presses a specific button combo, the
	//		kernel will kill off all PIDs not in this whitelist and then restart GMenu2X.  This is so users can
	//		recover from program crashes or hangs (or when a program won't let you exit!).
	if (fwType == "open2x")
	{
		generatePidWhitelist();
	}


	confStr.set_empty_key(" ");
	confStr.set_deleted_key("");
	confInt.set_empty_key(" ");
	confInt.set_deleted_key("");
	skinConfInt.set_empty_key(" ");
	skinConfInt.set_deleted_key("");
	skinConfStr.set_empty_key(" ");
	skinConfStr.set_deleted_key("");
	skinConfColors.set_empty_key(" ");
	skinConfColors.set_deleted_key("");

	//open2x
	o2x_savedVolumeMode = 0;
	o2x_volumeMode = VOLUME_MODE_NORMAL;
	o2x_volumeScalerNormal = VOLUME_SCALER_NORMAL;
	o2x_volumeScalerPhones = VOLUME_SCALER_PHONES;

	o2x_usb_net_on_boot = false;
	o2x_usb_net_ip = "";
	o2x_ftp_on_boot = false;
	o2x_telnet_on_boot = false;
	o2x_gp2xjoy_on_boot = false;
	o2x_usb_host_on_boot = false;
	o2x_usb_hid_on_boot = false;
	o2x_usb_storage_on_boot = false;

	// SD stuff
	// senquack - Initialize new options:
	o2x_auto_import_links = true;
	o2x_links_folder = "/mnt/sd/.open2x_gmenu2x";
	o2x_store_links_on_SD = true;
	o2x_use_autorun_on_SD = false;	// Never autoexec /mnt/sd/autorun.gpu by default.
	o2x_SD_read_only = false;

	// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
	// 	a USB gamepad.  It should be configurable because it will greatly interfere
	// 	with apps like Picodrive that already know how to use USB joysticks.
	o2x_gmenu2x_starts_joy2xd = true; 	// Is Joy2Xd enabled?
												// If this is true, GMenu2X will start the joy2xd
												// daemon at startup, and it will remain loaded
												// during program executions and will be restarted 
												// every time GMenu2X reloads) If an app specifically
												// disabled joy2xd in its link settings, joy2xd is
												// killed before launch of the program, then restarted
												// when gmenu2x comes back up, unless this bool is false.

	// senquack - stick click emulation for Open2X:
	o2x_stick_click_mode = f200 ? OPEN2X_STICK_CLICK_DPAD : OPEN2X_STICK_CLICK_DISABLED;	

	usbnet = samba = inet = web = false;
	useSelectionPng = false;

	

	//load config data
	readConfig();
	if (fwType=="open2x") {
		// senquack - new Open2x TV tweaking daemon should be killed when GMenu2X restarts
		system("killall tv_daemon");
		
		readConfigOpen2x();

		//	VOLUME SCALING
		switch(o2x_volumeMode) {
			case VOLUME_MODE_MUTE:   setVolumeScaler(VOLUME_SCALER_MUTE); break;
			case VOLUME_MODE_PHONES: setVolumeScaler(o2x_volumeScalerPhones);	break;
			case VOLUME_MODE_NORMAL: setVolumeScaler(o2x_volumeScalerNormal); break;
		}

		// senquack - STICK CLICK EMULATION:
		setStickClickEmulation(o2x_stick_click_mode); 

		// senquack - GPIO REMAPPING
		disableGpioRemapping();

		// senquack - UPPER MEMORY CACHING
		setUpperMemoryCaching(0);

		// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
		// 	a USB gamepad.  It should be configurable because it will greatly interfere
		// 	with apps like Picodrive that already know how to use USB joysticks.
		if (o2x_gmenu2x_starts_joy2xd)
		{
			activateJoy2xd();	// launch the daemon (kill off old instances first)
		}
		
	} else
		readCommonIni();

	halfX = resX/2;
	halfY = resY/2;
	bottomBarIconY = resY-18;
	bottomBarTextY = resY-10;

	path = "";
	getExePath();

#ifdef TARGET_GP2X
	gp2x_mem = 0;
	cx25874 = 0;
	batteryHandle = 0;

	gp2x_init();

	//senquack
	//Fix tv-out
//	if (gp2x_mem!=0) {
//		if (gp2x_memregs[0x2800>>1]&0x100) {
//			gp2x_memregs[0x2906>>1]=512;
////			gp2x_memregs[0x290C>>1]=640;
//			gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1];
//		}
//	}
	//senquack - pulled from Rlyeh's minlib for improved TVout:
//	gp2x_tv_lastmode = LCD;
#endif

	//Screen
	if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK)<0 ) {
		cout << "\033[0;34mGMENU2X:\033[0;31m Could not initialize SDL:\033[0m " << SDL_GetError() << endl;
		quit();
	}

	s = new Surface();
#ifdef TARGET_GP2X
	{
		//I use a tmp variable to hide the cursor as soon as possible (and create the double buffer surface only after that)
		//I'm forced to use SW surfaces since with HW there are issuse with changing the clock frequency
		SDL_Surface *tmps = SDL_SetVideoMode(resX, resY, confInt["videoBpp"], SDL_SWSURFACE);
		SDL_ShowCursor(0);
		s->enableVirtualDoubleBuffer(tmps);
		
		//senquack
		//Fix tv-out
//		if (gp2x_mem!=0) {
//			if (gp2x_memregs[0x2800>>1]&0x100) {
//				tweakTvOut(confStr["tvoutEncoding"] == "PAL");
//			}
//		}
		// After SDL initialization, it is necessary to tweak tv-related registers:
		if (gp2x_tv_mode) 
		{
			// TV out appears to be enabled, let's assume it is in the mode that is stored in
			// GMenu2X's configuration 
			cx25874 = open("/dev/cx25874",O_RDWR);
			if (cx25874 == -1)
			{
				cout << "Error opening /dev/cx25874" << endl;
				cx25874 = 0;
			} else 
			{
//				senquack - tweakTvOut will do this for us:
//				ioctl(cx25874, _IOW('v', 0x02, unsigned char), 
//					(confStr["tvoutEncoding"] == "PAL") ? PAL: NTSC);
				tweakTvOut(confStr["tvoutEncoding"] == "PAL");
			}
		}
	}
#endif
#else
	s->raw = SDL_SetVideoMode(resX, resY, confInt["videoBpp"], SDL_HWSURFACE|SDL_DOUBLEBUF);
#endif
	
	bg = NULL;
	font = NULL;
	menu = NULL;
	setSkin(confStr["skin"], false);
	initMenu();

	if (!fileExists(confStr["wallpaper"])) {
#ifdef DEBUG
		cout << "Searching wallpaper" << endl;
#endif
		FileLister fl("skins/"+confStr["skin"]+"/wallpapers",false,true);
		fl.setFilter(".png,.jpg,.jpeg,.bmp");
		fl.browse();
		if (fl.files.size()<=0 && confStr["skin"] != "Default")
			fl.setPath("skins/Default/wallpapers",true);
		if (fl.files.size()>0)
			confStr["wallpaper"] = fl.getPath()+fl.files[0];
	}

	initBG();
	input.init(path+"input.conf");
	setInputSpeed();
	initServices();

	//senquack - we now support a separate gamma setting for TV mode
#ifdef TARGET_GP2X
	if (gp2x_tv_mode)
	{
		setGamma(confInt["tvoutGamma"]);
	}
	else
	{
		setGamma(confInt["gamma"]);
	}
#else
	setGamma(confInt["gamma"]);
#endif

	setVolume(confInt["globalVolume"]);

	//senquack - New option, alwaysUseFastTimings, allows us to set fast ram timings as
	//	universal default.
	if (confInt["alwaysUseFastTimings"])
	{
		applyRamTimings();
	}
	else
	{
		applyDefaultTimings();
	}
	setClock(confInt["menuClock"]);

	//recover last session
	readTmp();
	if (lastSelectorElement>-1 && menu->selLinkApp()!=NULL && (!menu->selLinkApp()->getSelectorDir().empty() || !lastSelectorDir.empty()))
		menu->selLinkApp()->selector(lastSelectorElement,lastSelectorDir);

#ifdef DEBUG
	cout << "Starting main()" << endl;
#endif
	main();
	writeConfig();
	if (fwType=="open2x") writeConfigOpen2x();

	quit();
	exit(0);
}

GMenu2X::~GMenu2X() {
	//senquack - these should use delete not free since they were called with new
//	free(menu);
//	free(s);
//	free(font);
	delete menu;
	delete s;
	delete font;
}

void GMenu2X::quit() {
	fflush(NULL);
	sc.clear();
	s->free();
	SDL_Quit();
#ifdef TARGET_GP2X
	if (gp2x_mem!=0) {
		//		senquack
//		//Fix tv-out
//		if (gp2x_memregs[0x2800>>1]&0x100) {
//			gp2x_memregs[0x2906>>1]=512;
//			gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1];
//			//senquack - copied this line from the other "fix tv out" section, it seemed to be
//			//missing
//		}

		//senquack - make sure SDL didn't change any of our tv tweaks when exiting
//		if (gp2x_memregs[0x2800>>1]&0x100) {
//		if (gp2x_tv_mode != LCD) {
//			tweakTvOut(gp2x_tv_mode == PAL);
//		}
		gp2x_deinit();
	}
#endif
}

void GMenu2X::initBG() {
	sc.del("bgmain");

	if (bg != NULL) free(bg);

	if (!fileExists(confStr["wallpaper"])) {
		bg = new Surface(s);
		bg->box(0,0,resX,resY,0,0,0);
	} else {
		bg = new Surface(confStr["wallpaper"],false);
	}

	drawTopBar(bg);
	drawBottomBar(bg);

	Surface *bgmain = new Surface(bg);
	sc.add(bgmain,"bgmain");

	Surface sd("imgs/sd.png", confStr["skin"]);
	//senquack - added new icon to show SD is removed (grayscale sd icon) under open2x:
//	Surface sd_removed("imgs/sd_removed.png", confStr["skin"]);
	Surface cpu("imgs/cpu.png", confStr["skin"]);
	Surface volume("imgs/volume.png", confStr["skin"]);
	
	//senquack - added support for removable SDs and appropriate visual indication
	if (fwType != "open2x") 
	{
		// If running on open2x, this all gets blitted in the main() loop instead:
		string df = getDiskFree();
		sd.blit( sc["bgmain"], 3, bottomBarIconY );
		sc["bgmain"]->write( font, df, 22, bottomBarTextY, SFontHAlignLeft, SFontVAlignMiddle );
	}

	
//	//senquack 
//	if (strlen(df.c_str()) > 0)
//	{
//		volumeX = 27+font->getTextWidth(df);
//	}
//	else
//	{
//		volumeX = 0;
//	}

	//senquack - modified a bit here:
	volumeX = 27+font->getTextWidth("00000/00000MB");
	volume.blit( sc["bgmain"], volumeX, bottomBarIconY );
	volumeX += 19;
	cpuX = volumeX+font->getTextWidth("100")+5;
	cpu.blit( sc["bgmain"], cpuX, bottomBarIconY );
	cpuX += 19;
	manualX = cpuX+font->getTextWidth("300Mhz")+5;

	//senquack - new optional display of uptime:
	uptimeX = manualX + 5;
				
	//senquack - Display of services icons and display of volume scaling icons are mutually
	//				exclusive.  Services icons are displayed under non-open2x firmwares,
	//				volume scaling mode icon is displayed under open2x firmwares. 

	//senquack - furthermore, display of battery icon (rightmost icon) is dependent on 
	//whether or not we're running on an F100.  F200s don't support battery monitoring.

	scalerX = resX - (f200 ? 19 : 38);

	int serviceX = resX - (f200 ? 19 : 38);

	if (fwType != "open2x")
	{
		if (confInt["showServices"])
		{
			if (usbnet) {
				if (web) {
					Surface webserver("imgs/webserver.png", confStr["skin"]);
					webserver.blit( sc["bgmain"], serviceX, bottomBarIconY );
					serviceX -= 19;
				}
				if (samba) {
					Surface sambaS("imgs/samba.png", confStr["skin"]);
					sambaS.blit( sc["bgmain"], serviceX, bottomBarIconY );
					serviceX -= 19;
				}
				if (inet) {
					Surface inetS("imgs/inet.png", confStr["skin"]);
					inetS.blit( sc["bgmain"], serviceX, bottomBarIconY );
					serviceX -= 19;
				}
			}
		}
	}
}

void GMenu2X::initFont() {
	if (font != NULL) {
		free(font);
		font = NULL;
	}

	string fontFile = sc.getSkinFilePath("imgs/font.png");
	if (fontFile.empty()) {
		cout << "Font png not found!" << endl;
		quit();
		exit(-1);
	}
	font = new ASFont(fontFile);
}

void GMenu2X::initMenu() {
	//Menu structure handler
	menu = new Menu(this);
	for (uint i=0; i<menu->sections.size(); i++) {
		//Add virtual links in the applications section
		if (menu->sections[i]=="applications") {
			menu->addActionLink(i,"Explorer",MakeDelegate(this,&GMenu2X::explorer),tr["Launch an application"],"skin:icons/explorer.png");
		}

		//Add virtual links in the setting section
		else if (menu->sections[i]=="settings") {
			menu->addActionLink(i,"GMenu2X",MakeDelegate(this,&GMenu2X::options),tr["Configure GMenu2X's options"],"skin:icons/configure.png");
			//senquack - added new virtual link(s)
			if (fwType=="open2x")
			{
				menu->addActionLink(i,tr["About Open2X"],MakeDelegate(this,&GMenu2X::aboutOpen2X),tr["Info about Open2X"],"skin:icons/about.png");
				menu->addActionLink(i,tr["Open2x"],MakeDelegate(this,&GMenu2X::settingsOpen2x),tr["Configure Open2x system settings"],"skin:icons/o2xconfigure.png");
				menu->addActionLink(i,tr["Restore Apps"],MakeDelegate(this,&GMenu2X::restoreO2XAppSection),tr["Restore Open2X application section's links to defaults"],"skin:icons/o2xconfigure.png");
			}
			menu->addActionLink(i,tr["Skin"],MakeDelegate(this,&GMenu2X::skinMenu),tr["Configure skin"],"skin:icons/skin.png");
			menu->addActionLink(i,tr["Wallpaper"],MakeDelegate(this,&GMenu2X::changeWallpaper),tr["Change GMenu2X wallpaper"],"skin:icons/wallpaper.png");
#ifdef TARGET_GP2X
			menu->addActionLink(i,"TV",MakeDelegate(this,&GMenu2X::toggleTvOut),tr["Activate/deactivate tv-out"],"skin:icons/tv.png");
			//senquack
			if (fwType=="gph" && !f200)
			{
				menu->addActionLink(i,"USB Nand",MakeDelegate(this,&GMenu2X::activateNandUsb),tr["Activate Usb on Nand"],"skin:icons/usb.png");
				menu->addActionLink(i,"USB Sd",MakeDelegate(this,&GMenu2X::activateSdUsb),tr["Activate Usb on SD"],"skin:icons/usb.png");
			//menu->addActionLink(i,"USB Root",MakeDelegate(this,&GMenu2X::activateRootUsb),tr["Activate Usb on the root of the Gp2x Filesystem"],"skin:icons/usb.png");
			}
#endif
			if (fileExists(path+"log.txt"))
				menu->addActionLink(i,tr["Log Viewer"],MakeDelegate(this,&GMenu2X::viewLog),tr["Displays last launched program's output"],"skin:icons/ebook.png");
			menu->addActionLink(i,tr["About"],MakeDelegate(this,&GMenu2X::about),tr["Info about GMenu2X"],"skin:icons/about.png");
		}
	}

	menu->setSectionIndex(confInt["section"]);
	menu->setLinkIndex(confInt["link"]);

	menu->loadIcons();
	
#ifdef DEBUG
	//DEBUG
//	menu->addLink( "/mnt/sd/development/gmenu2x/", "sample.pxml", "applications" );
#endif
}

void GMenu2X::about() {
	vector<string> text;
	split(text,"GMenu2X is developed by Massimiliano \"Ryo\" Torromeo, and is released under the GPL-v2 license.\n\
Website: http://gmenu2x.sourceforge.net\n\
E-Mail & PayPal account: massimiliano.torromeo@gmail.com\n\
\n\
Thanks goes to...\n\
\n\
 Contributors\n\
----\n\
NoidZ for his gp2x' buttons graphics\n\
\n\
 Beta testers\n\
----\n\
Goemon4, PokeParadox, PSyMastR and Tripmonkey_uk\n\
\n\
 Translators\n\
----\n\
English & Italian by me\n\
French by Yodaz\n\
Danish by claus\n\
Dutch by superfly\n\
Spanish by pedator\n\
Portuguese (Portugal) by NightShadow\n\
Slovak by Jozef\n\
Swedish by Esslan and Micket\n\
German by fusion_power, johnnysnet and Waldteufel\n\
Finnish by Jontte and Atte\n\
Norwegian by cowai\n\
Russian by XaMMaX90\n\
\n\
 Donors\n\
----\n\
EvilDragon (www.gp2x.de)\n\
Tecnologie Creative (www.tecnologiecreative.it)\n\
TelcoLou\n\
gaterooze\n\
deepmenace\n\
superfly\n\
halo9\n\
sbock\n\
b._.o._.b\n\
Jacopastorius\n\
lorystorm90\n\
and all the anonymous donors...\n\
(If I missed to list you or if you want to be removed, contact me.)","\n");
	TextDialog td(this, "GMenu2X", tr.translate("Version $1 (Build date: $2)","0.10-test4",__DATE__,NULL), "icons/about.png", &text);
	td.exec();
}

void GMenu2X::aboutOpen2X() {
	vector<string> text;

	split(text,"Open2X was made possible by:\n\
----\n\
Coder\n\
DJWillis\n\
Jycet\n\
Lithosphere\n\
Mindless\n\
Orkie\n\
PokeParadox\n\
Ryo\n\
Senor Quack\n\
Wejp\n\
\n\
Open2X boot graphic made by Alex\n\
Gamepad icon for joy2xd made by Gort\n\
Credit for much hardware info to Rlyeh and Squidge\n\
Credit for HW-accelerated SDL to Paeryn\n\
(If you are missing from this list, please contact us in #open2x on freenode)","\n");
	TextDialog td(this, "Open2X", tr.translate("Version $1 (Build date: $2)",fwVersion.c_str(),__DATE__,NULL), "icons/about.png", &text);
	td.exec();
}

void GMenu2X::viewLog() {
	string logfile = path+"log.txt";
	if (fileExists(logfile)) {
		ifstream inf(logfile.c_str(), ios_base::in);
		if (inf.is_open()) {
			vector<string> log;

			string line;
			while (getline(inf, line, '\n'))
				log.push_back(line);
			inf.close();

			TextDialog td(this, tr["Log Viewer"], tr["Displays last launched program's output"], "icons/ebook.png", &log);
			td.exec();

			MessageBox mb(this, tr["Do you want to delete the log file?"], "icons/ebook.png");
			mb.buttons[ACTION_B] = tr["Yes"];
			mb.buttons[ACTION_X] = tr["No"];
			if (mb.exec() == ACTION_B) {
				ledOn();
				unlink(logfile.c_str());
				sync();
				menu->deleteSelectedLink();
				ledOff();
			}
		}
	}
}

void GMenu2X::readConfig() {
	string conffile = path+"gmenu2x.conf";
	if (fileExists(conffile)) {
		ifstream inf(conffile.c_str(), ios_base::in);
		if (inf.is_open()) {
			string line;
			while (getline(inf, line, '\n')) {
				string::size_type pos = line.find("=");
				string name = trim(line.substr(0,pos));
				string value = trim(line.substr(pos+1,line.length()));

				if (value.length()>1 && value.at(0)=='"' && value.at(value.length()-1)=='"')
					confStr[name] = value.substr(1,value.length()-2);
				else
					confInt[name] = atoi(value.c_str());
			}
			inf.close();
		}
	}

	if (!confStr["lang"].empty()) tr.setLang(confStr["lang"]);
	if (!confStr["wallpaper"].empty() && !fileExists(confStr["wallpaper"])) confStr["wallpaper"] = "";
	if (confStr["skin"].empty() || !fileExists("skins/"+confStr["skin"])) confStr["skin"] = "Default";

	evalIntConf( &confInt["outputLogs"], 0, 0,1 );
	evalIntConf( &confInt["maxClock"], 300, 200,300 );
	evalIntConf( &confInt["menuClock"], f200 ? 136 : 100, 50,300 );
	evalIntConf( &confInt["globalVolume"], 67, 0,100 );
	evalIntConf( &confInt["gamma"], 1, 1,100 );
	evalIntConf( &confInt["videoBpp"], 16, 8,32 );

	//senquack - added three options:
	evalIntConf( &confInt["showUptime"], 1, 0, 1 );
	evalIntConf( &confInt["showServices"], 0, 0, 1 );
	evalIntConf( &confInt["alwaysUseFastTimings"], 0, 0, 1 );

	//senquack - added adjustable TV-out offsets for proper centering and
	//				adjustable scaling:
	//   These value ranges and defaults were taken from Rlyeh's minlib:
//   horizontal (-50..0..35) - number of pixels to move the image horizontally.
//   vertical (-15..0..45) - number of pixels to move the image vertically.
//   - default setting for NTSC is (16, -7) 
//   - default setting for PAL  is (16, 19)
// NOTE: on my NTSC TV, I found 10 to be better than 16 for the X offset
	evalIntConf( &confInt["tvoutXOffsetNTSC"], 10, TV_MIN_XOFFSET, TV_MAX_XOFFSET);
	evalIntConf( &confInt["tvoutYOffsetNTSC"], -7, TV_MIN_YOFFSET, TV_MAX_YOFFSET);
	evalIntConf( &confInt["tvoutXOffsetPAL"], 16, TV_MIN_XOFFSET, TV_MAX_XOFFSET);
	evalIntConf( &confInt["tvoutYOffsetPAL"], 19, TV_MIN_YOFFSET, TV_MAX_YOFFSET);
	evalIntConf( &confInt["tvoutXScale"], 100, TV_MIN_XSCALE, TV_MAX_XSCALE);
	evalIntConf( &confInt["tvoutYScale"], 100, TV_MIN_YSCALE, TV_MAX_YSCALE);
	evalIntConf( &confInt["tvoutGamma"], 1, 1,100 );
	evalIntConf( &confInt["tvoutPalOverscanFix"], 0, 0, 1 );
	if (confStr["tvoutEncoding"] != "PAL") confStr["tvoutEncoding"] = "NTSC";
	resX = constrain( confInt["resolutionX"], 320,1920 );
	resY = constrain( confInt["resolutionY"], 240,1200 );
}

void GMenu2X::writeConfig() {
	ledOn();
	string conffile = path+"gmenu2x.conf";
	ofstream inf(conffile.c_str());
	if (inf.is_open()) {
		ConfStrHash::iterator endS = confStr.end();
		for(ConfStrHash::iterator curr = confStr.begin(); curr != endS; curr++)
			inf << curr->first << "=\"" << curr->second << "\"" << endl;

		ConfIntHash::iterator endI = confInt.end();
		for(ConfIntHash::iterator curr = confInt.begin(); curr != endI; curr++)
			inf << curr->first << "=" << curr->second << endl;

		inf.close();
		sync();
	}
	ledOff();
}


void GMenu2X::readConfigOpen2x() {
	string conffile = "/etc/config/open2x.conf";
	if (fileExists(conffile)) {
		ifstream inf(conffile.c_str(), ios_base::in);
		if (inf.is_open()) {
			string line;
			while (getline(inf, line, '\n')) {
				string::size_type pos = line.find("=");
				string name = trim(line.substr(0,pos));
				string value = trim(line.substr(pos+1,line.length()));

				if (name=="USB_NET_ON_BOOT") o2x_usb_net_on_boot = value == "y" ? true : false;
				else if (name=="USB_NET_IP") o2x_usb_net_ip = value;
				else if (name=="TELNET_ON_BOOT") o2x_telnet_on_boot = value == "y" ? true : false;
				else if (name=="FTP_ON_BOOT") o2x_ftp_on_boot = value == "y" ? true : false;
//				else if (name=="GP2XJOY_ON_BOOT") o2x_gp2xjoy_on_boot = value == "y" ? true : false;
				// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
				// 	a USB gamepad.  It should be configurable because it will greatly interfere
				// 	with apps like Picodrive that already know how to use USB joysticks.
				else if (name=="GMENU2X_STARTS_JOY2XD") o2x_gmenu2x_starts_joy2xd = value == "y" ? true : false;
				else if (name=="USB_HOST_ON_BOOT") o2x_usb_host_on_boot = value == "y" ? true : false;
				else if (name=="USB_HID_ON_BOOT") o2x_usb_hid_on_boot = value == "y" ? true : false;
				else if (name=="USB_STORAGE_ON_BOOT") o2x_usb_storage_on_boot = value == "y" ? true : false;
				else if (name=="VOLUME_MODE") o2x_volumeMode = o2x_savedVolumeMode = constrain( atoi(value.c_str()), 0, 2);
				else if (name=="PHONES_VALUE") o2x_volumeScalerPhones = constrain( atoi(value.c_str()), 0, 100);
				else if (name=="NORMAL_VALUE") o2x_volumeScalerNormal = constrain( atoi(value.c_str()), 0, 150);
				// SD stuff
				//senquack - new configurable settings for storage of links on SD:
				else if (name=="AUTO_IMPORT_OLD_LINKS") o2x_auto_import_links = value == "y" ? true : false;
				else if (name=="STORE_LINKS_ON_SD") o2x_store_links_on_SD = value == "y" ? true : false;
				else if (name=="USE_AUTORUN_ON_SD") o2x_use_autorun_on_SD = value == "y" ? true : false;
				//senquack - new Open2X stick click emulation:
				else if (name=="STICK_CLICK_EMULATION_MODE") o2x_stick_click_mode = constrain( atoi(value.c_str()), 0, 2);	
			}
			inf.close();
		}
	}
}
void GMenu2X::writeConfigOpen2x() {
	ledOn();
	string conffile = "/etc/config/open2x.conf";
	ofstream inf(conffile.c_str());
	if (inf.is_open()) {
		inf << "USB_NET_ON_BOOT=" << ( o2x_usb_net_on_boot ? "y" : "n" ) << endl;
		inf << "USB_NET_IP=" << o2x_usb_net_ip << endl;
		inf << "TELNET_ON_BOOT=" << ( o2x_telnet_on_boot ? "y" : "n" ) << endl;
		inf << "FTP_ON_BOOT=" << ( o2x_ftp_on_boot ? "y" : "n" ) << endl;
//		inf << "GP2XJOY_ON_BOOT=" << ( o2x_gp2xjoy_on_boot ? "y" : "n" ) << endl;
		// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
		// 	a USB gamepad.  It should be configurable because it can interfere
		// 	with apps specific apps that already know how to use USB joysticks. 
		// 	(Sometimes they're fine even then)
		inf << "GMENU2X_STARTS_JOY2XD=" << ( o2x_gmenu2x_starts_joy2xd ? "y" : "n" ) << endl;
		inf << "USB_HOST_ON_BOOT=" << ( (o2x_usb_host_on_boot || o2x_usb_hid_on_boot || o2x_usb_storage_on_boot) ? "y" : "n" ) << endl;
		inf << "USB_HID_ON_BOOT=" << ( o2x_usb_hid_on_boot ? "y" : "n" ) << endl;
		inf << "USB_STORAGE_ON_BOOT=" << ( o2x_usb_storage_on_boot ? "y" : "n" ) << endl;
		inf << "VOLUME_MODE=" << o2x_volumeMode << endl;
		if (o2x_volumeScalerPhones != VOLUME_SCALER_PHONES) inf << "PHONES_VALUE=" << o2x_volumeScalerPhones << endl;
		if (o2x_volumeScalerNormal != VOLUME_SCALER_NORMAL) inf << "NORMAL_VALUE=" << o2x_volumeScalerNormal << endl;
		// SD stuff
		//senquack - new configurable settings for storage of links on SD:
		inf << "AUTO_IMPORT_OLD_LINKS=" << ( o2x_auto_import_links ? "y" : "n" ) << endl;
		inf << "STORE_LINKS_ON_SD=" << ( o2x_store_links_on_SD ? "y" : "n" ) << endl;
		inf << "USE_AUTORUN_ON_SD=" << ( o2x_use_autorun_on_SD ? "y" : "n" ) << endl;
		inf << "STICK_CLICK_EMULATION_MODE=" << o2x_stick_click_mode << endl;
		inf.close();
		sync();
	}
	ledOff();
}

void GMenu2X::writeSkinConfig() {
	ledOn();
	string conffile = path+"skins/"+confStr["skin"]+"/skin.conf";
	ofstream inf(conffile.c_str());
	if (inf.is_open()) {
		ConfStrHash::iterator endS = skinConfStr.end();
		for(ConfStrHash::iterator curr = skinConfStr.begin(); curr != endS; curr++)
			inf << curr->first << "=\"" << curr->second << "\"" << endl;

		ConfIntHash::iterator endI = skinConfInt.end();
		for(ConfIntHash::iterator curr = skinConfInt.begin(); curr != endI; curr++)
			inf << curr->first << "=" << curr->second << endl;

		ConfRGBAHash::iterator endC = skinConfColors.end();
		for(ConfRGBAHash::iterator curr = skinConfColors.begin(); curr != endC; curr++)
			//	senquack - fixed bug where fields containing values less than 10
			//		 were output as a single digit, not double digits with leading zeroes:
//			inf << curr->first << "=#" << hex << curr->second.r << hex << curr->second.g << hex << curr->second.b << hex << curr->second.a << endl;
			inf << curr->first << "=#" 
				<< hex << setw(2) << setfill('0') << right << curr->second.r 
				<< hex << setw(2) << setfill('0') << right << curr->second.g 
				<< hex << setw(2) << setfill('0') << right << curr->second.b 
				<< hex << setw(2) << setfill('0') << right << curr->second.a << endl;

		inf.close();
		sync();
	}
	ledOff();
}

void GMenu2X::readCommonIni() {
	//senquack: not supported under open2x:
	if (fwType != "open2x" && fileExists("/usr/gp2x/common.ini")) {
		ifstream inf("/usr/gp2x/common.ini", ios_base::in);
		if (inf.is_open()) {
			string line;
			string section = "";
			while (getline(inf, line, '\n')) {
				line = trim(line);
				if (line[0]=='[' && line[line.length()-1]==']') {
					section = line.substr(1,line.length()-2);
				} else {
					string::size_type pos = line.find("=");
					string name = trim(line.substr(0,pos));
					string value = trim(line.substr(pos+1,line.length()));

					if (section=="usbnet") {
						if (name=="enable")
							usbnet = value=="true" ? true : false;
						else if (name=="ip")
							ip = value;

					} else if (section=="server") {
						if (name=="inet")
							inet = value=="true" ? true : false;
						else if (name=="samba")
							samba = value=="true" ? true : false;
						else if (name=="web")
							web = value=="true" ? true : false;
					}
				}
			}
			inf.close();
		}
	}
}

void GMenu2X::writeCommonIni() {}

void GMenu2X::readTmp() {
	lastSelectorElement = -1;
	if (fileExists("/tmp/gmenu2x.tmp")) {
		ifstream inf("/tmp/gmenu2x.tmp", ios_base::in);
		if (inf.is_open()) {
			string line;
			string section = "";
			while (getline(inf, line, '\n')) {
				string::size_type pos = line.find("=");
				string name = trim(line.substr(0,pos));
				string value = trim(line.substr(pos+1,line.length()));

				if (name=="section")
					menu->setSectionIndex(atoi(value.c_str()));
				else if (name=="link")
					menu->setLinkIndex(atoi(value.c_str()));
				else if (name=="selectorelem")
					lastSelectorElement = atoi(value.c_str());
				else if (name=="selectordir")
					lastSelectorDir = value;
			}
			inf.close();
		}
	}
}

void GMenu2X::writeTmp(int selelem, string selectordir) {
	string conffile = "/tmp/gmenu2x.tmp";
	ofstream inf(conffile.c_str());
	if (inf.is_open()) {
		inf << "section=" << menu->selSectionIndex() << endl;
		inf << "link=" << menu->selLinkIndex() << endl;
		if (selelem>-1)
			inf << "selectorelem=" << selelem << endl;
		if (selectordir!="")
			inf << "selectordir=" << selectordir << endl;
		inf.close();
	}
}

void GMenu2X::initServices() {
#ifdef TARGET_GP2X
	//senquack - services not controlled by GMenu2X under open2x
	if ((fwType != "open2x") && usbnet) {
		string services = "scripts/services.sh "+ip+" "+(inet?"on":"off")+" "+(samba?"on":"off")+" "+(web?"on":"off")+" &";
		system(services.c_str());
	}
#endif
}

void GMenu2X::ledOn() {
#ifdef TARGET_GP2X
	if (gp2x_mem!=0 && !f200) gp2x_memregs[0x106E >> 1] ^= 16;
	//SDL_SYS_JoystickGp2xSys(joy.joystick, BATT_LED_ON);
#endif
}

void GMenu2X::ledOff() {
#ifdef TARGET_GP2X
	if (gp2x_mem!=0 && !f200) gp2x_memregs[0x106E >> 1] ^= 16;
	//SDL_SYS_JoystickGp2xSys(joy.joystick, BATT_LED_OFF);
#endif
}

int GMenu2X::main() {
	uint linksPerPage = linkColumns*linkRows;
	int linkSpacingX = (resX-10 - linkColumns*skinConfInt["linkWidth"])/linkColumns;
	int linkSpacingY = (resY-35 - skinConfInt["topBarHeight"] - linkRows*skinConfInt["linkHeight"])/linkRows;
	uint sectionLinkPadding = (skinConfInt["topBarHeight"] - 32 - font->getLineHeight()) / 3;

	bool quit = false;

	int x,y, offset = menu->sectionLinks()->size()>linksPerPage ? 2 : 6, helpBoxHeight = fwType=="open2x" ? 154 : 139;

	uint i;
	long tickBattery = -60000, tickNow;
	string batteryIcon = "imgs/battery/0.png";
	stringstream ss;
	uint sectionsCoordX = 24;
	SDL_Rect re = {0,0,0,0};
	// senquack - keep track of if we need to reload the links/background because
	// 				SD has just been removed/inserted
	bool needToReloadLinks = false;
	bool SDinserted = (fwType == "open2x") ? isSDInserted() : true;
	string df = getDiskFree();
	

#ifdef DEBUG
	//framerate
	long tickFPS = SDL_GetTicks();
	int drawn_frames = 0;
	string fps = "";
#endif

	IconButton btnContextMenu(this,"skin:imgs/menu.png");
	btnContextMenu.setPosition(resX-38, bottomBarIconY);
	btnContextMenu.setAction(MakeDelegate(this, &GMenu2X::contextMenu));


	while (!quit) {
		tickNow = SDL_GetTicks();

		if (fwType=="open2x")
		{
		//senquack - need to track if an SD has just been removed or inserted.  If so, we need
		//					to reload the links and background sprite
			if (isSDInserted())
			{
				if (!SDinserted)
				{
					// SD has just been inserted
					SDinserted = true;
					needToReloadLinks = true;
				}
			}
			else
			{
				if (SDinserted)
				{
					// SD has just been removed
					SDinserted = false;
					needToReloadLinks = true;
				}
			}

			if (needToReloadLinks)
			{
				cout << "SD insertion/removal detected, reloading links & background" << endl;
				cout << "Pausing two seconds before reloading menu.." << endl;
				sleep(2);
				
				delete menu;
				initMenu();
				initBG();
				needToReloadLinks = false;
				df = getDiskFree();
			}
		}

		//background
		sc["bgmain"]->blit(s,0,0);

		//Sections
		sectionsCoordX = halfX - (constrain((uint)menu->sections.size(), 0 , linkColumns) * skinConfInt["linkWidth"]) / 2;

		if (menu->firstDispSection()>0)
			sc.skinRes("imgs/l_enabled.png")->blit(s,0,0);
		else
			sc.skinRes("imgs/l_disabled.png")->blit(s,0,0);
		if (menu->firstDispSection()+linkColumns<menu->sections.size())
			sc.skinRes("imgs/r_enabled.png")->blit(s,resX-10,0);
		else
			sc.skinRes("imgs/r_disabled.png")->blit(s,resX-10,0);
		for (i=menu->firstDispSection(); i<menu->sections.size() && i<menu->firstDispSection()+linkColumns; i++) {
			string sectionIcon = "skin:sections/"+menu->sections[i]+".png";
			x = (i-menu->firstDispSection())*skinConfInt["linkWidth"]+sectionsCoordX;
			if (menu->selSectionIndex()==(int)i)
				s->box(x, 0, skinConfInt["linkWidth"], skinConfInt["topBarHeight"], skinConfColors["selectionBg"]);
			x += skinConfInt["linkWidth"]/2;
			if (sc.exists(sectionIcon))
				sc[sectionIcon]->blit(s,x-16,sectionLinkPadding,32,32);
			else
			//		senquack - fixed bad position of icon here
//				sc.skinRes("icons/section.png")->blit(s,x,sectionLinkPadding);
				sc.skinRes("icons/section.png")->blit(s,x-16,sectionLinkPadding);
			s->write( font, menu->sections[i], x, skinConfInt["topBarHeight"]-sectionLinkPadding, SFontHAlignCenter, SFontVAlignBottom );
		}

		//Links
		s->setClipRect(offset,skinConfInt["topBarHeight"],resX-9,resY-74); //32*2+10
		for (i=menu->firstDispRow()*linkColumns; i<(menu->firstDispRow()*linkColumns)+linksPerPage && i<menu->sectionLinks()->size(); i++) {
			int ir = i-menu->firstDispRow()*linkColumns;
			x = (ir%linkColumns)*(skinConfInt["linkWidth"]+linkSpacingX)+offset;
			y = ir/linkColumns*(skinConfInt["linkHeight"]+linkSpacingY)+skinConfInt["topBarHeight"]+2;
			menu->sectionLinks()->at(i)->setPosition(x,y);

			//			senquack - this seems to be what paints the selection box over the links:
			if (i==(uint)menu->selLinkIndex())
				menu->sectionLinks()->at(i)->paintHover();

			menu->sectionLinks()->at(i)->paint();
		}
		s->clearClipRect();

		drawScrollBar(linkRows,menu->sectionLinks()->size()/linkColumns + ((menu->sectionLinks()->size()%linkColumns==0) ? 0 : 1),menu->firstDispRow(),43,resY-81);

		//senquack - made this conditional and made position dependent on scalerX
		if (fwType == "open2x")
		{
			switch(o2x_volumeMode) {
				case VOLUME_MODE_MUTE:   sc.skinRes("imgs/mute.png")->blit(s,scalerX,bottomBarIconY); break;
				case VOLUME_MODE_PHONES: sc.skinRes("imgs/phones.png")->blit(s,scalerX,bottomBarIconY); break;
				default: sc.skinRes("imgs/volume.png")->blit(s,scalerX,bottomBarIconY); break;
			}
		}

		//senquack - added support for removable SDs and appropriate visual indication
//		string df = getDiskFree();
//		if (df != "")		// SD is mounted
//		{
//			sc.skinRes("imgs/sd.png")->blit(s,3,bottomBarIconY);
//			s->write( font, df, 22, bottomBarTextY, SFontHAlignLeft, SFontVAlignMiddle );
//		}
//		else					// SD is removed
//		{
//			// New grayed-out "sd removed" icon
//			sc.skinRes("imgs/sd_removed.png")->blit( sc["bgmain"], 3, bottomBarIconY );
//		}
		if (SDinserted)		// SD is mounted
		{
//			string df = getDiskFree();
			sc.skinRes("imgs/sd.png")->blit(s,3,bottomBarIconY);
			s->write( font, df, 22, bottomBarTextY, SFontHAlignLeft, SFontVAlignMiddle );
		}
		else					// SD is removed
		{
			// New grayed-out "sd removed" icon
			sc.skinRes("imgs/sd_removed.png")->blit( sc["bgmain"], 3, bottomBarIconY );
		}

		//senquack - added optional display of uptime to help gauge battery life
		//	system("export OPEN2X_UPTIME=`uptime | sed 's/ //g; s/\\([0123456789]\\+\\)\\+up.*/\\1/g; s/:[0123456789]\\+$//g'`");
		if (confInt["showUptime"])
		{
			FILE *uptime_file;
			float uptime_secs, idle_secs;
			uptime_file = fopen("/proc/uptime", "r");
			if (uptime_file != NULL)
			{
				if (!feof(uptime_file))
				{
					if (fscanf(uptime_file, "%f %f", &uptime_secs, &idle_secs) != EOF)
					{
						uptimeX = manualX + 19 + 5;
						sc.skinRes("imgs/uptime.png")->blit(s,uptimeX,bottomBarIconY);
						char buf[30];
						sprintf(buf, "%1$d:%2$.2d", ((int)uptime_secs / 3600), 
								(((int)uptime_secs % 3600) / 60));
						s->write( font, (string)buf, uptimeX + 19, bottomBarTextY, 
								SFontHAlignLeft, SFontVAlignMiddle);
					}
				}
				fclose(uptime_file);
			}
		}

		if (menu->selLink()!=NULL) {
			s->write ( font, menu->selLink()->getDescription(), halfX, resY-19, SFontHAlignCenter, SFontVAlignBottom );
			if (menu->selLinkApp()!=NULL) {
				s->write ( font, menu->selLinkApp()->clockStr(confInt["maxClock"]), cpuX, bottomBarTextY, SFontHAlignLeft, SFontVAlignMiddle );
				s->write ( font, menu->selLinkApp()->volumeStr(), volumeX, bottomBarTextY, SFontHAlignLeft, SFontVAlignMiddle );
				//Manual indicator
				if (!menu->selLinkApp()->getManual().empty())
					sc.skinRes("imgs/manual.png")->blit(s,manualX,bottomBarIconY);
			}
		}

		if (f200) {
			btnContextMenu.paint();
		}

		//senquack - disable this for F200s since they don't monitor battery level
		if (!f200)
		{
			//check battery status every 60 seconds
			if (tickNow-tickBattery >= 60000) {
				tickBattery = tickNow;
				unsigned short battlevel = getBatteryLevel();
				if (battlevel>5) {
					batteryIcon = "imgs/battery/ac.png";
				} else {
					ss.clear();
					ss << battlevel;
					ss >> batteryIcon;
					batteryIcon = "imgs/battery/"+batteryIcon+".png";
				}
			}
			sc.skinRes(batteryIcon)->blit( s, resX-19, bottomBarIconY );
		}

		//On Screen Help
		if (input[ACTION_A]) {
			//			senquack - fixing height here
//			s->box(10,50,300,143, skinConfColors["messageBoxBg"]);
			s->box(10,50,300,helpBoxHeight + 5, skinConfColors["messageBoxBg"]);
			s->rectangle( 12,52,296,helpBoxHeight, skinConfColors["messageBoxBorder"] );
			s->write( font, tr["CONTROLS"], 20, 60 );
			s->write( font, tr["B, Stick press: Launch link / Confirm action"], 20, 80 );
			s->write( font, tr["L, R: Change section"], 20, 95 );
			s->write( font, tr["Y: Show manual/readme"], 20, 110 );
			s->write( font, tr["SELECT: Show contextual menu"], 20, 125 );
			s->write( font, tr["START: Show options menu"], 20, 140 );
			//senquack - vol up and down do different things on open2x:
			if (fwType == "open2x")
			{
				s->write( font, tr["VOLUP, VOLDOWN: Change volume scaler mode"], 20, 155 );
				s->write( font, tr["X: Unmount SD card for safe removal"], 20, 170 );
			}
			else
			{
				s->write( font, tr["VOLUP, VOLDOWN: Change cpu clock"], 20, 155 );
				s->write( font, tr["A+VOLUP, A+VOLDOWN: Change volume"], 20, 170 );
			}
		}

#ifdef DEBUG
		//framerate
		drawn_frames++;
		if (tickNow-tickFPS>=1000) {
			ss.clear();
			ss << drawn_frames*(tickNow-tickFPS+1)/1000;
			ss >> fps;
			tickFPS = tickNow;
			drawn_frames = 0;
		}
		s->write( font, fps+" FPS", resX-1,1 ,SFontHAlignRight );
#endif

		s->flip();

		//touchscreen
		if (f200) {
			ts.poll();
			btnContextMenu.handleTS();
			re.x = 0; re.y = 0; re.h = skinConfInt["topBarHeight"]; re.w = resX;
			if (ts.pressed() && ts.inRect(re)) {
				re.w = skinConfInt["linkWidth"];
				for (i=menu->firstDispSection(); !ts.handled() && i<menu->sections.size() && i<menu->firstDispSection()+linkColumns; i++) {
					sectionsCoordX = halfX - (constrain((uint)(linkColumns-menu->sections.size()), 0 , linkColumns) * skinConfInt["linkWidth"]) / 2;
					re.x = (i-menu->firstDispSection())*re.w+sectionsCoordX;

					if (ts.inRect(re)) {
						menu->setSectionIndex(i);
						ts.setHandled();
					}
				}
			}

			i=menu->firstDispRow()*linkColumns;
			while ( i<(menu->firstDispRow()*linkColumns)+linksPerPage && i<menu->sectionLinks()->size()) {
				if (menu->sectionLinks()->at(i)->isPressed())
					menu->setLinkIndex(i);
				if (menu->sectionLinks()->at(i)->handleTS())
					i = menu->sectionLinks()->size();
				i++;
			}
		}

//#ifdef TARGET_GP2X
		input.update();
		if ( input[ACTION_B] && menu->selLink()!=NULL ) menu->selLink()->run();
		else if ( input[ACTION_START]  ) options();
		else if ( input[ACTION_SELECT] ) contextMenu();
		// UNMOUNT SD CARD
		// senquack - new feature to safely unmount an SD card before removal (needed
		// 	when using EXT2/3-formatted cards, probably a good idea for other FSes too)
		else if ( fwType=="open2x" && input[ACTION_X] ) {
			unmountSD();
			df = getDiskFree();
		}
		// VOLUME SCALE MODIFIER
		//senquack - changing volume scaling mode modifer to be volume up/down on Open2X
		else if ( fwType=="open2x" && input[ACTION_VOLDOWN] ) {
			o2x_volumeMode--;
			if(o2x_volumeMode < VOLUME_MODE_MUTE)
				o2x_volumeMode = VOLUME_MODE_MUTE;
			switch(o2x_volumeMode) {
				case VOLUME_MODE_MUTE:   setVolumeScaler(VOLUME_SCALER_MUTE); break;
				case VOLUME_MODE_PHONES: setVolumeScaler(o2x_volumeScalerPhones); break;
				case VOLUME_MODE_NORMAL: setVolumeScaler(o2x_volumeScalerNormal); break;
			}
			setVolume(confInt["globalVolume"]);
			
		}
		else if ( fwType=="open2x" && input[ACTION_VOLUP] ) {
			o2x_volumeMode++;
			if(o2x_volumeMode > VOLUME_MODE_NORMAL)
				o2x_volumeMode = VOLUME_MODE_NORMAL;
			switch(o2x_volumeMode) {
				case VOLUME_MODE_MUTE:   setVolumeScaler(VOLUME_SCALER_MUTE); break;
				case VOLUME_MODE_PHONES: setVolumeScaler(o2x_volumeScalerPhones); break;
				case VOLUME_MODE_NORMAL: setVolumeScaler(o2x_volumeScalerNormal); break;
			}
			setVolume(confInt["globalVolume"]);
		}
		// LINK NAVIGATION
		else if ( input[ACTION_LEFT ]  ) menu->linkLeft();
		else if ( input[ACTION_RIGHT]  ) menu->linkRight();
		else if ( input[ACTION_UP   ]  ) menu->linkUp();
		else if ( input[ACTION_DOWN ]  ) menu->linkDown();
		// SELLINKAPP SELECTED
		else if (menu->selLinkApp()!=NULL) {
			if ( input[ACTION_Y] ) menu->selLinkApp()->showManual();
			else if ( input.isActive(ACTION_A) ) {
				// senquack - OK, I hate that volume up/down changes clock speed, I think it
				// just confuses most people when they suddenly find a program crashes after
				// inadvertently changing it to 290mhz when just wanting to change GP2X's
				// volume.  We are disabling this for Open2X.
				if (fwType != "open2x")
				{
					// VOLUME
					if ( input[ACTION_VOLDOWN] && !input.isActive(ACTION_VOLUP) )
						menu->selLinkApp()->setVolume( constrain(menu->selLinkApp()->volume()-1,0,100) );
					if ( input[ACTION_VOLUP] && !input.isActive(ACTION_VOLDOWN) )
						menu->selLinkApp()->setVolume( constrain(menu->selLinkApp()->volume()+1,0,100) );;
					if ( input.isActive(ACTION_VOLUP) && input.isActive(ACTION_VOLDOWN) ) menu->selLinkApp()->setVolume(-1);
				}
			} else {
				// senquack - OK, I hate that volume up/down changes clock speed, I think it
				// just confuses most people when they suddenly find a program crashes after
				// inadvertently changing it to 290mhz when just wanting to change GP2X's
				// volume.  We are disabling this for Open2X.
				if (fwType != "open2x")
				{
					// CLOCK
					if ( input[ACTION_VOLDOWN] && !input.isActive(ACTION_VOLUP) )
						menu->selLinkApp()->setClock( constrain(menu->selLinkApp()->clock()-1,50,confInt["maxClock"]) );
					if ( input[ACTION_VOLUP] && !input.isActive(ACTION_VOLDOWN) )
						menu->selLinkApp()->setClock( constrain(menu->selLinkApp()->clock()+1,50,confInt["maxClock"]) );
					if ( input.isActive(ACTION_VOLUP) && input.isActive(ACTION_VOLDOWN) ) menu->selLinkApp()->setClock(200);
				}
			}
		}
		if ( input.isActive(ACTION_A) ) {
			if (input.isActive(ACTION_L) && input.isActive(ACTION_R))
				saveScreenshot();
		} else {
			// SECTIONS
			if ( input[ACTION_L     ] ) {
				menu->decSectionIndex();
				offset = menu->sectionLinks()->size()>linksPerPage ? 2 : 6;
			} else if ( input[ACTION_R     ] ) {
				menu->incSectionIndex();
				offset = menu->sectionLinks()->size()>linksPerPage ? 2 : 6;
			}
		}
		
		usleep(LOOP_DELAY);
	}

	return -1;
}

void GMenu2X::explorer() {
	FileDialog fd(this,tr["Select an application"],".gpu,.gpe,.sh");
	if (fd.exec()) {
		if (confInt["saveSelection"] && (confInt["section"]!=menu->selSectionIndex() || confInt["link"]!=menu->selLinkIndex()))
			writeConfig();
		if (fwType == "open2x" && o2x_savedVolumeMode != o2x_volumeMode)
			writeConfigOpen2x();

		//string command = cmdclean(fd.path()+"/"+fd.file) + "; sync & cd "+cmdclean(getExePath())+"; exec ./gmenu2x";
		string command = cmdclean(fd.path()+"/"+fd.file);
		chdir(fd.path().c_str());
		quit();
		setClock(200);
		execlp("/bin/sh","/bin/sh","-c",command.c_str(),NULL);

		//if execution continues then something went wrong and as we already called SDL_Quit we cannot continue
		//try relaunching gmenu2x
		fprintf(stderr, "Error executing selected application, re-launching gmenu2x\n");
		chdir(getExePath().c_str());
		execlp("./gmenu2x", "./gmenu2x", NULL);
	}
}

void GMenu2X::options() {
	int curMenuClock = confInt["menuClock"];
	int curGlobalVolume = confInt["globalVolume"];
	//G
	int prevgamma = confInt["gamma"];
	bool showRootFolder = fileExists("/mnt/root");

	//senquack - made display of services optional since they could overlap uptime display:
	//					NOTE: services icons never displayed under Open2X (not yet anways)
	bool showServices = confInt["showServices"];

	//senquack - new option to always use fast RAM timings
	bool alwaysUseFastTimings = confInt["alwaysUseFastTimings"];

	//senquack - added adjustable TV-out offsets for proper centering:
	int tvoutXOffsetNTSC = confInt["tvoutXOffsetNTSC"];
	int tvoutYOffsetNTSC = confInt["tvoutYOffsetNTSC"];
	int tvoutXOffsetPAL = confInt["tvoutXOffsetPAL"];
	int tvoutYOffsetPAL = confInt["tvoutYOffsetPAL"];
	int tvoutXScale = confInt["tvoutXScale"];
	int tvoutYScale = confInt["tvoutYScale"];
	int tvoutGamma = confInt["tvoutGamma"];
	int tvoutPalOverscanFix = confInt["tvoutPalOverscanFix"];

	FileLister fl_tr("translations");
	fl_tr.browse();
	fl_tr.files.insert(fl_tr.files.begin(),"English");
	string lang = tr.lang();

	vector<string> encodings;
	encodings.push_back("NTSC");
	encodings.push_back("PAL");

	SettingsDialog sd(this,tr["Settings"]);
	sd.addSetting(new MenuSettingMultiString(this,tr["Language"],tr["Set the language used by GMenu2X"],&lang,&fl_tr.files));
	sd.addSetting(new MenuSettingBool(this,tr["Save last selection"],tr["Save the last selected link and section on exit"],&confInt["saveSelection"]));
	sd.addSetting(new MenuSettingInt(this,tr["Clock for GMenu2X"],tr["Set the cpu working frequency when running GMenu2X"],&confInt["menuClock"],50,325));
	sd.addSetting(new MenuSettingInt(this,tr["Maximum overclock"],tr["Set the maximum overclock for launching links"],&confInt["maxClock"],50,325));
	sd.addSetting(new MenuSettingInt(this,tr["Global Volume"],tr["Set the default volume for the gp2x soundcard"],&confInt["globalVolume"],0,100));
	sd.addSetting(new MenuSettingBool(this,tr["Output logs"],tr["Logs the output of the links. Use the Log Viewer to read them."],&confInt["outputLogs"]));
	//G
	sd.addSetting(new MenuSettingInt(this,tr["Gamma"],tr["Set gp2x gamma value (default: 10)"],&confInt["gamma"],1,100));
	sd.addSetting(new MenuSettingMultiString(this,tr["Tv-Out encoding"],tr["Encoding of the tv-out signal"],&confStr["tvoutEncoding"],&encodings));
	
	//senquack - added adjustable TV-out offsets for proper centering:
	//   These value ranges and defaults were taken from Rlyeh's minlib:
//   horizontal (-50..0..35) - number of pixels to move the image horizontally.
//   vertical (-15..0..45) - number of pixels to move the image vertically.
//   - default setting for NTSC is (16, -7) 
//   - default setting for PAL  is (16, 19)
// NOTE: on my NTSC TV, I found 10 to be better than 16 for the X offset
	sd.addSetting(new MenuSettingInt(this,tr["TV X Offset (NTSC)"],tr["NTSC TV image offset left/right (default: 10)"],&confInt["tvoutXOffsetNTSC"],TV_MIN_XOFFSET,TV_MAX_XOFFSET));
	sd.addSetting(new MenuSettingInt(this,tr["TV Y Offset (NTSC)"],tr["NTSC TV image offset up/down (default: -7)"],&confInt["tvoutYOffsetNTSC"],TV_MIN_YOFFSET,TV_MAX_YOFFSET));
	sd.addSetting(new MenuSettingInt(this,tr["TV X Offset (PAL)"],tr["PAL TV image offset left/right (default: 16)"],&confInt["tvoutXOffsetPAL"],TV_MIN_XOFFSET,TV_MAX_XOFFSET));
	sd.addSetting(new MenuSettingInt(this,tr["TV Y Offset (PAL)"],tr["PAL TV image offset up/down (default: 19)"],&confInt["tvoutYOffsetPAL"],TV_MIN_YOFFSET,TV_MAX_YOFFSET));
	sd.addSetting(new MenuSettingInt(this,tr["TV X Scaling %"],tr["TV image horizontal stretch/shrink (default: 100%)"],&confInt["tvoutXScale"],TV_MIN_XSCALE,TV_MAX_XSCALE));
	sd.addSetting(new MenuSettingInt(this,tr["TV Y Scaling %"],tr["TV image vertical stretch/shrink (default: 100%)"],&confInt["tvoutYScale"],TV_MIN_YSCALE,TV_MAX_YSCALE));
	sd.addSetting(new MenuSettingInt(this,tr["TV Gamma"],tr["Set gp2x tv-out gamma value (default: 8)"],&confInt["tvoutGamma"],1,100));
	sd.addSetting(new MenuSettingBool(this,tr["TV PAL Overscan Fix"],tr["Enable Rlyeh's PAL-mode tweak (untested)"],&confInt["tvoutPalOverscanFix"]));

	sd.addSetting(new MenuSettingBool(this,tr["Show root"],tr["Show root folder in the file selection dialogs"],&showRootFolder));
	
	//senquack - added new option to always use fast RAM timings:
	sd.addSetting(new MenuSettingBool(this,tr["RAM timings always fast"],tr["Always use fast RAM timings (overrides link settings)"],&confInt["alwaysUseFastTimings"]));
	//senquack - added optional display of uptime :
	sd.addSetting(new MenuSettingBool(this,tr["Show uptime"],tr["Show uptime in status bar"],&confInt["showUptime"]));
	//senquack - under Open2X, services are not displayed as icons:
	if (fwType != "open2x")
		sd.addSetting(new MenuSettingBool(this,tr["Show services"],tr["Show services icons in status bar"],&confInt["showServices"]));


	if (sd.exec() && sd.edited()) {
		//G
		//senquack - tv now has a separate gamma setting:
#ifdef TARGET_GP2X
		if (prevgamma != confInt["gamma"] && !gp2x_tv_mode) setGamma(confInt["gamma"]);
#else
		if (prevgamma != confInt["gamma"]) setGamma(confInt["gamma"]);
#endif
		if (curMenuClock!=confInt["menuClock"]) setClock(confInt["menuClock"]);
		if (curGlobalVolume!=confInt["globalVolume"]) setVolume(confInt["globalVolume"]);
		if (lang == "English") lang = "";
		if (lang != tr.lang()) tr.setLang(lang);
		if (fileExists("/mnt/root") && !showRootFolder)
			unlink("/mnt/root");
		else if (!fileExists("/mnt/root") && showRootFolder)
			symlink("/","/mnt/root");
		//senquack -  this option affects the BG so be sure to update it if needed:
		if (fwType != "open2x" && showServices != confInt["showServices"])
		{
			initBG();
		}
		// if new RAM timings option is changed change RAM timings immediately:
		if (alwaysUseFastTimings != confInt["alwaysUseFastTimings"])
		{
			if (confInt["alwaysUseFastTimings"])
			{
				applyRamTimings();
			}
			else
			{
				applyDefaultTimings();
			}
		}
#ifdef TARGET_GP2X
		if (	gp2x_tv_mode	&&
				(	tvoutXOffsetNTSC != confInt["tvoutXOffsetNTSC"] 	||
					tvoutYOffsetNTSC != confInt["tvoutYOffsetNTSC"] 	||
					tvoutXOffsetPAL != confInt["tvoutXOffsetPAL"] 		||
					tvoutYOffsetPAL != confInt["tvoutYOffsetPAL"] 		||
					tvoutXScale != confInt["tvoutXScale"] 					||
					tvoutYScale != confInt["tvoutYScale"] 					||	
					tvoutYScale != confInt["tvoutGamma"] 					||
					tvoutPalOverscanFix != confInt["tvoutPalOverscanFix"] 					))
		{
			// User altered some tvout settings, apply immediately if in tv mode
			tweakTvOut(confStr["tvoutEncoding"] == "PAL");
			setGamma(confInt["tvoutGamma"]);
		}
#endif

		writeConfig();
	}
}

void GMenu2X::settingsOpen2x() {
	// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
	// 	a USB gamepad.  It should be configurable because it will greatly interfere
	// 	with apps like Picodrive that already know how to use USB joysticks.
	int gmenu2x_starts_joy2xd = o2x_gmenu2x_starts_joy2xd;
	
	SettingsDialog sd(this,tr["Open2x Settings"]);
	sd.addSetting(new MenuSettingBool(this,tr["USB net on boot"],tr["Allow USB networking to be started at boot time"],&o2x_usb_net_on_boot));
	sd.addSetting(new MenuSettingString(this,tr["USB net IP"],tr["IP address to be used for USB networking"],&o2x_usb_net_ip));
	sd.addSetting(new MenuSettingBool(this,tr["Telnet on boot"],tr["Allow telnet to be started at boot time"],&o2x_telnet_on_boot));
	sd.addSetting(new MenuSettingBool(this,tr["FTP on boot"],tr["Allow FTP to be started at boot time"],&o2x_ftp_on_boot));
	sd.addSetting(new MenuSettingBool(this,tr["Joy2xd USB->GP2X control"],tr["Enable daemon allowing control of GP2X from USB gamepads"],&o2x_gmenu2x_starts_joy2xd));
//	sd.addSetting(new MenuSettingBool(this,tr["GP2XJOY on boot"],tr["Create a js0 device for GP2X controls"],&o2x_gp2xjoy_on_boot));
	sd.addSetting(new MenuSettingBool(this,tr["USB host on boot"],tr["Allow USB host to be started at boot time"],&o2x_usb_host_on_boot));
	sd.addSetting(new MenuSettingBool(this,tr["USB HID on boot"],tr["Allow USB HID to be started at boot time"],&o2x_usb_hid_on_boot));
	sd.addSetting(new MenuSettingBool(this,tr["USB storage on boot"],tr["Allow USB storage to be started at boot time"],&o2x_usb_storage_on_boot));
	//sd.addSetting(new MenuSettingInt(this,tr["Speaker Mode on boot"],tr["Set Speaker mode. 0 = Mute, 1 = Phones, 2 = Speaker"],&o2x_volumeMode,0,2));
	sd.addSetting(new MenuSettingInt(this,tr["Speaker Scaler"],tr["Set the Speaker Mode scaling 0-150\% (default is 100\%)"],&o2x_volumeScalerNormal,0,150));
	sd.addSetting(new MenuSettingInt(this,tr["Headphones Scaler"],tr["Set the Headphones Mode scaling 0-100\% (default is 65\%)"],&o2x_volumeScalerPhones,0,100));
	//senquack - new SD stuff:
	sd.addSetting(new MenuSettingBool(this,tr["Store Links on SDs"],tr["Storage of Open2X's gmenu2x links on SD (default:on)"],&o2x_store_links_on_SD));
	sd.addSetting(new MenuSettingBool(this,tr["Auto-import old links on SDs"],tr["Automatic import of old gmenu2x links on SDs (default:on)"],&o2x_auto_import_links));
	sd.addSetting(new MenuSettingBool(this,tr["Run SD's autorun.gpu"],tr["Auto-execution of autorun.gpu on SD (default:off)"],&o2x_use_autorun_on_SD));
	//senquack - new stick-click emulation:
//#define OPEN2X_STICK_CLICK_DISABLED		0	// stick-click emulation disabled (default)
//#define OPEN2X_STICK_CLICK_DPAD			1	// stick-click emulated by pressing UP+DOWN+LEFT+RIGHT
//#define OPEN2X_STICK_CLICK_VOLUPDOWN	2	// stick click emulated by pressing VOLUP+VOLDOWN	
	sd.addSetting(new MenuSettingInt(this,tr["Stick-click emulation combo"], tr["0:disabled   1:Up+Down+Left+Right   2:VolUp+VolDown"], &o2x_stick_click_mode, 0, 2));

	if (sd.exec() && sd.edited()) {
		writeConfigOpen2x();
		switch(o2x_volumeMode) {
			case VOLUME_MODE_MUTE:   setVolumeScaler(VOLUME_SCALER_MUTE); break;
			case VOLUME_MODE_PHONES: setVolumeScaler(o2x_volumeScalerPhones);   break;
			case VOLUME_MODE_NORMAL: setVolumeScaler(o2x_volumeScalerNormal); break;
		}
		setVolume(confInt["globalVolume"]);
		// senquack - STICK CLICK EMULATION:
		setStickClickEmulation(o2x_stick_click_mode); 
		// senquack - USB joystick -> GP2X control daemon:
		if (gmenu2x_starts_joy2xd != o2x_gmenu2x_starts_joy2xd)
		{
			if (o2x_gmenu2x_starts_joy2xd)
			{
				activateJoy2xd();
			} else {
				deactivateJoy2xd();
			}
		}
	}
}

void GMenu2X::skinMenu() {
	FileLister fl_sk("skins",true,false);
	fl_sk.exclude.push_back("..");
	fl_sk.browse();
	string curSkin = confStr["skin"];

	SettingsDialog sd(this,tr["Skin"]);
	sd.addSetting(new MenuSettingMultiString(this,tr["Skin"],tr["Set the skin used by GMenu2X"],&confStr["skin"],&fl_sk.directories));
	sd.addSetting(new MenuSettingRGBA(this,tr["Top Bar Color"],tr["Color of the top bar"],&skinConfColors["topBarBg"]));
	sd.addSetting(new MenuSettingRGBA(this,tr["Bottom Bar Color"],tr["Color of the bottom bar"],&skinConfColors["bottomBarBg"]));
	sd.addSetting(new MenuSettingRGBA(this,tr["Selection Color"],tr["Color of the selection and other interface details"],&skinConfColors["selectionBg"]));
	sd.addSetting(new MenuSettingRGBA(this,tr["Message Box Color"],tr["Background color of the message box"],&skinConfColors["messageBoxBg"]));
	sd.addSetting(new MenuSettingRGBA(this,tr["Message Box Border Color"],tr["Border color of the message box"],&skinConfColors["messageBoxBorder"]));
	sd.addSetting(new MenuSettingRGBA(this,tr["Message Box Selection Color"],tr["Color of the selection of the message box"],&skinConfColors["messageBoxSelection"]));

	if (sd.exec() && sd.edited()) {
		if (curSkin != confStr["skin"]) {
			setSkin(confStr["skin"]);
			writeConfig();
		}
		writeSkinConfig();
		initBG();
	}
}


void GMenu2X::setSkin(string skin, bool setWallpaper) {
	confStr["skin"] = skin;

	//Clear previous skin settings
	skinConfColors.clear();
	skinConfStr.clear();
	skinConfInt.clear();

	//clear collection and change the skin path
	sc.clear();
	sc.setSkin(skin);

	//reset colors to the default values
	//senquack - changed these for open2x
	if (fwType == "open2x")
	{

		skinConfColors["topBarBg"] = (RGBAColor){200,200,200,130};
		skinConfColors["bottomBarBg"] = (RGBAColor){200,200,200,130};
		skinConfColors["selectionBg"] = (RGBAColor){0,0,0,130};
		skinConfColors["messageBoxBg"] = (RGBAColor){255,255,255,255};
		skinConfColors["messageBoxBorder"] = (RGBAColor){80,80,80,255};
		skinConfColors["messageBoxSelection"] = (RGBAColor){160,160,160,255};
	} 
	else
	{
		skinConfColors["topBarBg"] = (RGBAColor){255,255,255,130};
		skinConfColors["bottomBarBg"] = (RGBAColor){255,255,255,130};
		skinConfColors["selectionBg"] = (RGBAColor){255,255,255,130};
		skinConfColors["messageBoxBg"] = (RGBAColor){255,255,255,255};
		skinConfColors["messageBoxBorder"] = (RGBAColor){80,80,80,255};
		skinConfColors["messageBoxSelection"] = (RGBAColor){160,160,160,255};
	}

	//load skin settings
	string skinconfname = "skins/"+skin+"/skin.conf";
	if (fileExists(skinconfname)) {
		ifstream skinconf(skinconfname.c_str(), ios_base::in);
		if (skinconf.is_open()) {
			string line;
			while (getline(skinconf, line, '\n')) {
				line = trim(line);
				cout << "skinconf: " << line << endl;
				string::size_type pos = line.find("=");
				string name = trim(line.substr(0,pos));
				string value = trim(line.substr(pos+1,line.length()));

				if (value.length()>0) {
					if (value.length()>1 && value.at(0)=='"' && value.at(value.length()-1)=='"')
						skinConfStr[name] = value.substr(1,value.length()-2);
					else if (value.at(0) == '#')
						skinConfColors[name] = strtorgba( value.substr(1,value.length()) );
					else
						skinConfInt[name] = atoi(value.c_str());
				}
			}
			skinconf.close();

			if (setWallpaper && !skinConfStr["wallpaper"].empty() && fileExists("skins/"+skin+"/wallpapers/"+skinConfStr["wallpaper"]))
				confStr["wallpaper"] = "skins/"+skin+"/wallpapers/"+skinConfStr["wallpaper"];
		}
	}

	evalIntConf( &skinConfInt["topBarHeight"], 40, 32,120 );
	evalIntConf( &skinConfInt["linkHeight"], 40, 32,120 );
	evalIntConf( &skinConfInt["linkWidth"], 60, 32,120 );

	//recalculate some coordinates based on the new element sizes
	linkColumns = (resX-10)/skinConfInt["linkWidth"];
	linkRows = (resY-35-skinConfInt["topBarHeight"])/skinConfInt["linkHeight"];

	if (menu != NULL) menu->loadIcons();

	//Selection png
	useSelectionPng = sc.addSkinRes("imgs/selection.png") != NULL;

	//font
	initFont();
}

void GMenu2X::activateSdUsb() {
	if (usbnet) {
		MessageBox mb(this,tr["Operation not permitted."]+"\n"+tr["You should disable Usb Networking to do this."]);
		mb.exec();
	} else {
		system("scripts/usbon.sh sd");
		MessageBox mb(this,tr["USB Enabled (SD)"],"icons/usb.png");
		mb.buttons[ACTION_B] = tr["Turn off"];
		mb.exec();
		system("scripts/usboff.sh sd");
	}
}

void GMenu2X::activateNandUsb() {
	if (usbnet) {
		MessageBox mb(this,tr["Operation not permitted."]+"\n"+tr["You should disable Usb Networking to do this."]);
		mb.exec();
	} else {
		system("scripts/usbon.sh nand");
		MessageBox mb(this,tr["USB Enabled (Nand)"],"icons/usb.png");
		mb.buttons[ACTION_B] = tr["Turn off"];
		mb.exec();
		system("scripts/usboff.sh nand");
	}
}

void GMenu2X::activateRootUsb() {
	if (usbnet) {
		MessageBox mb(this,tr["Operation not permitted."]+"\n"+tr["You should disable Usb Networking to do this."]);
		mb.exec();
	} else {
		system("scripts/usbon.sh root");
		MessageBox mb(this,tr["USB Enabled (Root)"],"icons/usb.png");
		mb.buttons[ACTION_B] = tr["Turn off"];
		mb.exec();
		system("scripts/usboff.sh root");
	}
}

void GMenu2X::activateJoy2xd() {
#ifdef TARGET_GP2X
	// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
	// 	a USB gamepad.  
	if (fwType == "open2x") {
//		string cmdline = "/etc/init.d/Ojoy2xd start";
		// very important this is not start, only should be called with start
		// on first startup from the init scripts.
		string cmdline = "/etc/init.d/Ojoy2xd restart";
		system(cmdline.c_str());
	}
#endif
}

void GMenu2X::deactivateJoy2xd() {
#ifdef TARGET_GP2X
	// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
	// 	a USB gamepad.  
	if (fwType == "open2x") {
		string cmdline = "/etc/init.d/Ojoy2xd stop";
		system(cmdline.c_str());
	}
#endif
}

//senquack - new action allows safe SD removal (removal of SDs with EXT filesystems
//	causes problems without explicitly unmounting the FS first.)
void GMenu2X::unmountSD() {
	MessageBox mb(this, tr["Unmount SD card?"], "icons/explorer.png");
	mb.buttons[ACTION_B] = tr["Yes"];
	mb.buttons[ACTION_X] = tr["No"];
	if (mb.exec() == ACTION_B) {
		sync();
		system("/bin/umount /mnt/sd -f");
		sleep(1);
		delete menu;
		initMenu();
		initBG();
	}
}

//senquack - new Open2X virtual settings link that allows users to restore all
//		the links in the NAND's "applications" section to the originals in case
//		an edit was incorrectly made. 
void GMenu2X::restoreO2XAppSection() {
	MessageBox mb(this, tr["Are you sure?"]);
	mb.buttons[ACTION_B] = tr["Yes"];
	mb.buttons[ACTION_X] = tr["No"];
	if (mb.exec() == ACTION_B) {
		ledOn();
		delete menu;
		system("scripts/restore_o2x_apps.sh");
		cout << "Open2X applications links restored to defaults, reloading menu" << endl;
		initMenu();
		initBG();
		sync();
		ledOff();
	}

}

void GMenu2X::contextMenu() {
	vector<MenuOption> voices;
	{
	MenuOption opt = {tr.translate("Add link in $1",menu->selSection().c_str(),NULL), MakeDelegate(this, &GMenu2X::addLink)};
	voices.push_back(opt);
	}

	if (menu->selLinkApp()!=NULL) {
		{
		MenuOption opt = {tr.translate("Edit $1",menu->selLink()->getTitle().c_str(),NULL), MakeDelegate(this, &GMenu2X::editLink)};
		voices.push_back(opt);
		}{
		MenuOption opt = {tr.translate("Delete $1 link",menu->selLink()->getTitle().c_str(),NULL), MakeDelegate(this, &GMenu2X::deleteLink)};
		voices.push_back(opt);
		}
	}

	{
	MenuOption opt = {tr["Add section"], MakeDelegate(this, &GMenu2X::addSection)};
	voices.push_back(opt);
	}{
	MenuOption opt = {tr["Rename section"], MakeDelegate(this, &GMenu2X::renameSection)};
	voices.push_back(opt);
	}{
	MenuOption opt = {tr["Delete section"], MakeDelegate(this, &GMenu2X::deleteSection)};
	voices.push_back(opt);
	}{
	MenuOption opt = {tr["Scan for applications and games"], MakeDelegate(this, &GMenu2X::scanner)};
	voices.push_back(opt);
	}

	bool close = false;
	uint i, sel=0, fadeAlpha=0;

	int h = font->getHeight();
	int h2 = font->getHalfHeight();
	SDL_Rect box;
	box.h = (h+2)*voices.size()+8;
	box.w = 0;
	for (i=0; i<voices.size(); i++) {
		int w = font->getTextWidth(voices[i].text);
		if (w>box.w) box.w = w;
	}
	box.w += 23;
	box.x = halfX - box.w/2;
	box.y = halfY - box.h/2;

	SDL_Rect selbox = {box.x+4, 0, box.w-8, h+2};
	long tickNow, tickStart = SDL_GetTicks();

	Surface bg(s);
	/*//Darken background
	bg.box(0, 0, resX, resY, 0,0,0,150);
	bg.box(box.x, box.y, box.w, box.h, skinConfColors["messageBoxBg"]);
	bg.rectangle( box.x+2, box.y+2, box.w-4, box.h-4, skinConfColors["messageBoxBorder"] );*/
	while (!close) {
		tickNow = SDL_GetTicks();

		selbox.y = box.y+4+(h+2)*sel;
		bg.blit(s,0,0);
		
		if (fadeAlpha<200) fadeAlpha = intTransition(0,200,tickStart,500,tickNow);
		s->box(0, 0, resX, resY, 0,0,0,fadeAlpha);
		s->box(box.x, box.y, box.w, box.h, skinConfColors["messageBoxBg"]);
		s->rectangle( box.x+2, box.y+2, box.w-4, box.h-4, skinConfColors["messageBoxBorder"] );


		//draw selection rect
		s->box( selbox.x, selbox.y, selbox.w, selbox.h, skinConfColors["messageBoxSelection"] );
		for (i=0; i<voices.size(); i++)
			s->write( font, voices[i].text, box.x+12, box.y+h2+5+(h+2)*i, SFontHAlignLeft, SFontVAlignMiddle );
		s->flip();

		//touchscreen
		if (f200) {
			ts.poll();
			if (ts.released()) {
				if (!ts.inRect(box))
					close = true;
				else if (ts.x>=selbox.x && ts.x<=selbox.x+selbox.w)
					for (i=0; i<voices.size(); i++) {
						selbox.y = box.y+4+(h+2)*i;
						if (ts.y>=selbox.y && ts.y<=selbox.y+selbox.h) {
							voices[i].action();
							close = true;
							i = voices.size();
						}
					}
			} else if (ts.pressed() && ts.inRect(box)) {
				for (i=0; i<voices.size(); i++) {
					selbox.y = box.y+4+(h+2)*i;
					if (ts.y>=selbox.y && ts.y<=selbox.y+selbox.h) {
						sel = i;
						i = voices.size();
					}
				}
			}
		}

		input.update();
		if ( input[ACTION_SELECT] ) close = true;
		if ( input[ACTION_UP    ] ) sel = max(0, sel-1);
		if ( input[ACTION_DOWN  ] ) sel = min((int)voices.size()-1, sel+1);
		if ( input[ACTION_B] ) { voices[sel].action(); close = true; }
	}
}

void GMenu2X::changeWallpaper() {
	WallpaperDialog wp(this);
	if (wp.exec() && confStr["wallpaper"] != wp.wallpaper) {
		confStr["wallpaper"] = wp.wallpaper;
		initBG();
		writeConfig();
	}
}

void GMenu2X::saveScreenshot() {
	ledOn();
	uint x = 0;
	stringstream ss;
	string fname;
	do {
		x++;
		fname = "";
		ss.clear();
		ss << x;
		ss >> fname;
		fname = "screen"+fname+".bmp";
	} while (fileExists(fname));
	SDL_SaveBMP(s->raw,fname.c_str());
	sync();
	ledOff();
}

void GMenu2X::addLink() {
	//senquack - SD stuff
	if (fwType == "open2x")
	{
		if ((strcasecmp((menu->selSection()).c_str(), "applications") == 0) ||
			(strcasecmp((menu->selSection()).c_str(), "settings") == 0) )
		{
			MessageBox mb(this,tr["Adding links in this section is not allowed."]);
			mb.exec();
			return;
		}
	}

	FileDialog fd(this,tr["Select an application"]);
	if (fd.exec()) {
		ledOn();
		menu->addLink(fd.path(), fd.file);
		sync();
		ledOff();
	}
}

void GMenu2X::editLink() {
	//senquack - SD stuff
	if (fwType == "open2x")
	{
		//		senquack - allow editing of applications links since people might
		//						want to change clockspeeds, etc.
		if	(strcasecmp((menu->selSection()).c_str(), "settings") == 0)
		{
			MessageBox mb(this,tr["Editing links in this section is not allowed."]);
			mb.exec();
			return;
		}
	}

	if (menu->selLinkApp()==NULL) return;

	vector<string> pathV;
	split(pathV,menu->selLinkApp()->file,"/");
	string oldSection = "";
	if (pathV.size()>1)
		oldSection = pathV[pathV.size()-2];
	string newSection = oldSection;

	string linkTitle = menu->selLinkApp()->getTitle();
	string linkDescription = menu->selLinkApp()->getDescription();
	string linkIcon = menu->selLinkApp()->getIcon();
	string linkManual = menu->selLinkApp()->getManual();
	string linkParams = menu->selLinkApp()->getParams();
	string linkSelFilter = menu->selLinkApp()->getSelectorFilter();
	string linkSelDir = menu->selLinkApp()->getSelectorDir();
	bool linkSelBrowser = menu->selLinkApp()->getSelectorBrowser();
	bool linkUseRamTimings = menu->selLinkApp()->getUseRamTimings();
	string linkSelScreens = menu->selLinkApp()->getSelectorScreens();
	string linkSelAliases = menu->selLinkApp()->getAliasFile();
	int linkClock = menu->selLinkApp()->clock();
	int linkVolume = menu->selLinkApp()->volume();
	//G
	int linkGamma = menu->selLinkApp()->gamma();
	
	//senquack - new open2x /dev/gpio (SDL) button remapping support:
	int link_o2x_gpio_mapping[19];
	bool link_o2x_gpio_remapping = false;
	// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
	// 	no longer necessary:
	bool link_o2x_upper_memory_cached;
	//senquack - new Open2X support for TV tweaking daemon:
	bool link_o2x_tv_daemon_enabled;
	// Tweak YUV layer?
	bool link_o2x_tv_daemon_tweak_yuv;
	// Hit scaling registers?
	bool link_o2x_tv_daemon_scaling_tweak;
	int link_o2x_tv_daemon_xoffset;
	int link_o2x_tv_daemon_yoffset;
	int link_o2x_tv_daemon_xscale;
	int link_o2x_tv_daemon_yscale;
	int link_o2x_tv_daemon_vxscale;
	int link_o2x_tv_daemon_vyscale;
	// Delay before first tweaking:
	int link_o2x_tv_daemon_first_delay;
	// Delay inbetween subsequent tweaks:
	int link_o2x_tv_daemon_delay;
	// Tweak just once after pausing for the delay above and terminate?
	bool link_o2x_tv_daemon_tweak_only_once;
//	// Needed for some things like gngeo:
//	bool link_o2x_tv_daemon_force_720_pitch;
	// Not normally needed, since tv mode should already be enabled:
	bool link_o2x_tv_daemon_enable_tv_mode;
	//	DEPRECATED:
//	// Special fix for some games that used a buggy SDL, like Tilematch and some
//	// of Ruckage's earlier games:
//	bool link_o2x_tv_daemon_stubborn_fix;
	// Rage2X segfaults when started in TVout mode, but will allow you to 
	// turn TV mode on from inside it.  This option lets you launch the tv tweaker
	// even when TV mode isn't enabled (it actually works)
	bool link_o2x_tv_daemon_always_started;

	// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
	// 	a USB gamepad.  It should be configurable because it will greatly interfere
	// 	with apps like Picodrive that already know how to use USB joysticks.
	bool link_o2x_link_uses_joy2xd;
	// When this is true, a hack is implemented that presents a dummy js0 
	// joystick to all applications, and the first USB joystick is thus
	// not seen at all by any apps.  This is very useful for some apps like
	// PocketSNES that don't have very flexible button remapping and won't
	// let player 2 use the second joystick otherwise.
	bool link_o2x_joy2xd_hides_js0;	

	if (fwType == "open2x")
	{
		link_o2x_gpio_remapping = menu->selLinkApp()->getGpioRemappingStatus();
		int i;
		for (i=0; i < 19; i++)
		{
			link_o2x_gpio_mapping[i] = menu->selLinkApp()->getGpioMapping(i);
		}

		link_o2x_upper_memory_cached = menu->selLinkApp()->getUpperMemoryCachingStatus();

		link_o2x_tv_daemon_enabled = menu->selLinkApp()->getTVDaemonStatus();
		link_o2x_tv_daemon_tweak_yuv = menu->selLinkApp()->getTVDaemonTweakYuv();
		link_o2x_tv_daemon_scaling_tweak = menu->selLinkApp()->getTVDaemonScalingTweak();
		link_o2x_tv_daemon_xoffset = menu->selLinkApp()->getTVDaemonXOffset();
		link_o2x_tv_daemon_yoffset = menu->selLinkApp()->getTVDaemonYOffset();
		link_o2x_tv_daemon_xscale = menu->selLinkApp()->getTVDaemonXScale();
		link_o2x_tv_daemon_yscale = menu->selLinkApp()->getTVDaemonYScale();
		link_o2x_tv_daemon_vxscale = menu->selLinkApp()->getTVDaemonVXScale();
		link_o2x_tv_daemon_vyscale = menu->selLinkApp()->getTVDaemonVYScale();
		link_o2x_tv_daemon_delay = menu->selLinkApp()->getTVDaemonDelay();
		link_o2x_tv_daemon_first_delay = menu->selLinkApp()->getTVDaemonFirstDelay();
		link_o2x_tv_daemon_tweak_only_once = menu->selLinkApp()->getTVDaemonTweakOnlyOnce();
//		link_o2x_tv_daemon_force_720_pitch = menu->selLinkApp()->getTVDaemonForce720Pitch();
		link_o2x_tv_daemon_enable_tv_mode = menu->selLinkApp()->getTVDaemonEnableTVMode();
		//		Deprecated:
//		link_o2x_tv_daemon_stubborn_fix = menu->selLinkApp()->getTVDaemonStubbornFix();
		link_o2x_tv_daemon_always_started = menu->selLinkApp()->getTVDaemonAlwaysStarted();

		link_o2x_link_uses_joy2xd = menu->selLinkApp()->getJoy2xdStatus();
		link_o2x_joy2xd_hides_js0 = menu->selLinkApp()->getJoy2xdHidesJs0();
	}

	string diagTitle = tr.translate("Edit link: $1",linkTitle.c_str(),NULL);
	string diagIcon = menu->selLinkApp()->getIconPath();

	SettingsDialog sd(this,diagTitle,diagIcon);
	sd.addSetting(new MenuSettingString(this,tr["Title"],tr["Link title"],&linkTitle, diagTitle,diagIcon));
	sd.addSetting(new MenuSettingString(this,tr["Description"],tr["Link description"],&linkDescription, diagTitle,diagIcon));
	sd.addSetting(new MenuSettingMultiString(this,tr["Section"],tr["The section this link belongs to"],&newSection,&menu->sections));
	sd.addSetting(new MenuSettingImage(this,tr["Icon"],tr.translate("Select an icon for the link: $1",linkTitle.c_str(),NULL),&linkIcon,".png,.bmp,.jpg,.jpeg"));
	sd.addSetting(new MenuSettingFile(this,tr["Manual"],tr["Select a graphic/textual manual or a readme"],&linkManual,".man.png,.txt"));
	sd.addSetting(new MenuSettingInt(this,tr["Clock (default: 200)"],tr["Cpu clock frequency to set when launching this link"],&linkClock,50,confInt["maxClock"]));
	sd.addSetting(new MenuSettingBool(this,tr["Tweak RAM Timings"],tr["This usually speeds up the application at the cost of stability"],&linkUseRamTimings));
	//	senquack - default volume is now 67 (that has always been the GP2X's default)  and the
	//		volume always gets set before launching a link now (so that one program cannot affect
	//		another's volume)
//	sd.addSetting(new MenuSettingInt(this,tr["Volume (default: -1)"],tr["Volume to set for this link"],&linkVolume,-1,100));
	sd.addSetting(new MenuSettingInt(this,tr["Volume (default: 67)"],tr["Volume to set for this link"],&linkVolume,-1,100));
	sd.addSetting(new MenuSettingString(this,tr["Parameters"],tr["Parameters to pass to the application"],&linkParams, diagTitle,diagIcon));
	sd.addSetting(new MenuSettingDir(this,tr["Selector Directory"],tr["Directory to scan for the selector"],&linkSelDir));
	sd.addSetting(new MenuSettingBool(this,tr["Selector Browser"],tr["Allow the selector to change directory"],&linkSelBrowser));
	sd.addSetting(new MenuSettingString(this,tr["Selector Filter"],tr["Filter for the selector (Separate values with a comma)"],&linkSelFilter, diagTitle,diagIcon));
	sd.addSetting(new MenuSettingDir(this,tr["Selector Screenshots"],tr["Directory of the screenshots for the selector"],&linkSelScreens));
	sd.addSetting(new MenuSettingFile(this,tr["Selector Aliases"],tr["File containing a list of aliases for the selector"],&linkSelAliases));
	//G
	sd.addSetting(new MenuSettingInt(this,tr["Gamma (default: 0)"],tr["Gamma value to set when launching this link"],&linkGamma,0,100));
	sd.addSetting(new MenuSettingBool(this,tr["Wrapper"],tr["Explicitly relaunch GMenu2X after this link's execution ends"],&menu->selLinkApp()->wrapper));
	sd.addSetting(new MenuSettingBool(this,tr["Don't Leave"],tr["Don't quit GMenu2X when launching this link"],&menu->selLinkApp()->dontleave));

	//senquack - new open2x gpio remapping support, upper memory caching support, TV daemon support:
	if (fwType == "open2x")
	{
		sd.addSetting(new MenuSettingBool(this,tr["Cache upper memory"],tr["Force full MMUhack - unnecessary and can cause glitches"], &link_o2x_upper_memory_cached));
		sd.addSetting(new MenuSettingBool(this,tr["TV Tweaker Daemon"],tr["Runs in background when TV mode is enabled"], &link_o2x_tv_daemon_enabled));
		sd.addSetting(new MenuSettingBool(this,tr["TV YUV Tweaked"],tr["Only useful for video and very rare apps like LemonBoy2X"], &link_o2x_tv_daemon_tweak_yuv));
		sd.addSetting(new MenuSettingBool(this,tr["TV Scaling Tweak"],tr["Known to be incompatible with LemonBoy2X"], &link_o2x_tv_daemon_scaling_tweak));
		sd.addSetting(new MenuSettingInt(this,tr["TV X Offset"],tr["Defaults - NTSC: 10  PAL: 10"],&link_o2x_tv_daemon_xoffset,TV_MIN_XOFFSET,TV_MAX_XOFFSET));
		sd.addSetting(new MenuSettingInt(this,tr["TV Y Offset"],tr["Defaults - NTSC: -7  PAL: 19"],&link_o2x_tv_daemon_yoffset,TV_MIN_YOFFSET,TV_MAX_YOFFSET));
		sd.addSetting(new MenuSettingInt(this,tr["TV X Scaling %"],tr["TV image horizontal stretch/shrink (default: 100%)"],&link_o2x_tv_daemon_xscale,TV_MIN_XSCALE,TV_MAX_XSCALE));
		sd.addSetting(new MenuSettingInt(this,tr["TV Y Scaling %"],tr["TV image vertical stretch/shrink (default: 100%)"],&link_o2x_tv_daemon_yscale,TV_MIN_YSCALE,TV_MAX_YSCALE));
		sd.addSetting(new MenuSettingInt(this,tr["TV YUV X Scaling %"],tr["YUV image horizontal stretch/shrink (default: 100%)"],&link_o2x_tv_daemon_vxscale,TV_MIN_VXSCALE,TV_MAX_VXSCALE));
		sd.addSetting(new MenuSettingInt(this,tr["TV YUV Y Scaling %"],tr["YUV image vertical stretch/shrink (default: 100%)"],&link_o2x_tv_daemon_vyscale,TV_MIN_VYSCALE,TV_MAX_VYSCALE));
		sd.addSetting(new MenuSettingInt(this,tr["TV First Tweak Delay"],tr["Seconds to wait before the first tweaking (default: 4)"],&link_o2x_tv_daemon_first_delay,TV_DAEMON_MIN_FIRST_DELAY,TV_DAEMON_MAX_FIRST_DELAY));
		sd.addSetting(new MenuSettingInt(this,tr["TV Tweaking Delay"],tr["Seconds to wait inbetween maintenance tweaks (default: 1)"],&link_o2x_tv_daemon_delay,TV_DAEMON_MIN_DELAY,TV_DAEMON_MAX_DELAY));
		sd.addSetting(new MenuSettingBool(this,tr["TV Tweaked Only Once "],tr["Daemon waits, tweaks only once, and terminates"], &link_o2x_tv_daemon_tweak_only_once));
		//		Deprecated:
//		sd.addSetting(new MenuSettingBool(this,tr["TV Forced to 720 Pitch"],tr["Try this if screen is garbled on apps like gngeo."], &link_o2x_tv_daemon_force_720_pitch));
//		sd.addSetting(new MenuSettingBool(this,tr["TV Fixed Stubbornly"],tr["Try this if screen is garbled or interlaced"], &link_o2x_tv_daemon_stubborn_fix));
		sd.addSetting(new MenuSettingBool(this,tr["TV Daemon Always Started"],tr["Always launch even when in LCD mode (Useful for Rage2X)"], &link_o2x_tv_daemon_always_started));
		sd.addSetting(new MenuSettingBool(this,tr["TV Mode Forced On"],tr["Daemon waits, forces TV mode on, and then tweaks"], &link_o2x_tv_daemon_enable_tv_mode));
		sd.addSetting(new MenuSettingBool(this,tr["SDL Control Remapping"],tr["Works for all SDL games and others that read /dev/GPIO"], &link_o2x_gpio_remapping));
		sd.addSetting(new MenuSettingInt(this,tr["Button 0 (Up)"],tr["Remap button 0 (Up)"],&(link_o2x_gpio_mapping[0]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 1 (UpLeft)"],tr["Remap button 1 (UpLeft)"],&(link_o2x_gpio_mapping[1]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 2 (Left)"],tr["Remap button 2 (Left)"],&(link_o2x_gpio_mapping[2]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 3 (DownLeft)"],tr["Remap button 3 (DownLeft)"],&(link_o2x_gpio_mapping[3]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 4 (Down)"],tr["Remap button 4 (Down)"],&(link_o2x_gpio_mapping[4]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 5 (DownRight)"],tr["Remap button 5 (DownRight)"],&(link_o2x_gpio_mapping[5]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 6 (Right)"],tr["Remap button 6 (Right)"],&(link_o2x_gpio_mapping[6]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 7 (UpRight)"],tr["Remap button 7 (UpRight)"],&(link_o2x_gpio_mapping[7]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 8 (Start)"],tr["Remap button 8 (Start)"],&(link_o2x_gpio_mapping[8]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 9 (Select)"],tr["Remap button 9 (Select)"],&(link_o2x_gpio_mapping[9]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 10 (R Trig.)"],tr["Remap button 10 (R Trigger)"],&(link_o2x_gpio_mapping[10]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 11 (L Trig.)"],tr["Remap button 11 (L Trigger)"],&(link_o2x_gpio_mapping[11]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 12 (A)"],tr["Remap button 12 (A)"],&(link_o2x_gpio_mapping[12]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 13 (B)"],tr["Remap button 13 (B)"],&(link_o2x_gpio_mapping[13]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 14 (Y)"],tr["Remap button 14 (Y)"],&(link_o2x_gpio_mapping[14]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 15 (X)"],tr["Remap button 15 (X)"],&(link_o2x_gpio_mapping[15]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 16 (Vol+)"],tr["Remap button 16 (Vol+)"],&(link_o2x_gpio_mapping[16]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 17 (Vol-)"],tr["Remap button 17 (Vol-)"],&(link_o2x_gpio_mapping[17]),0,18));
		sd.addSetting(new MenuSettingInt(this,tr["Button 18 (Click)"],tr["Remap button 18 (Stick Click)"],&(link_o2x_gpio_mapping[18]),0,18));
		sd.addSetting(new MenuSettingBool(this,tr["Joy2xd USB->GP2X control"],tr["Control GP2X buttons with USB Joy 0"], &link_o2x_link_uses_joy2xd));
		sd.addSetting(new MenuSettingBool(this,tr["Joy2xd hides Joy 0"],tr["No events reported from js0 (default: on)"], &link_o2x_joy2xd_hides_js0));
	}

	if (sd.exec() && sd.edited()) {
		ledOn();

		menu->selLinkApp()->setTitle(linkTitle);
		menu->selLinkApp()->setDescription(linkDescription);
		menu->selLinkApp()->setIcon(linkIcon);
		menu->selLinkApp()->setManual(linkManual);
		menu->selLinkApp()->setParams(linkParams);
		menu->selLinkApp()->setSelectorFilter(linkSelFilter);
		menu->selLinkApp()->setSelectorDir(linkSelDir);
		menu->selLinkApp()->setSelectorBrowser(linkSelBrowser);
		menu->selLinkApp()->setUseRamTimings(linkUseRamTimings);
		menu->selLinkApp()->setSelectorScreens(linkSelScreens);
		menu->selLinkApp()->setAliasFile(linkSelAliases);
		menu->selLinkApp()->setClock(linkClock);
		menu->selLinkApp()->setVolume(linkVolume);
		//G
		menu->selLinkApp()->setGamma(linkGamma);

		if (fwType == "open2x")
		{
			//senquack - new open2x gpio remapping support:
			menu->selLinkApp()->setGpioRemappingStatus(link_o2x_gpio_remapping);
			int i;
			for (i = 0; i < 19; i++)
			{
				menu->selLinkApp()->setGpioMapping(i, link_o2x_gpio_mapping[i]);
			}

			// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
			// 	no longer necessary:
			menu->selLinkApp()->setUpperMemoryCachingStatus(link_o2x_upper_memory_cached);
			//senquack - new support for TV tweaking daemon:
			menu->selLinkApp()->setTVDaemonStatus(link_o2x_tv_daemon_enabled);
			menu->selLinkApp()->setTVDaemonTweakYuv(link_o2x_tv_daemon_tweak_yuv);
			menu->selLinkApp()->setTVDaemonScalingTweak(link_o2x_tv_daemon_scaling_tweak);
			menu->selLinkApp()->setTVDaemonXOffset(link_o2x_tv_daemon_xoffset);
			menu->selLinkApp()->setTVDaemonYOffset(link_o2x_tv_daemon_yoffset);
			menu->selLinkApp()->setTVDaemonXScale(link_o2x_tv_daemon_xscale);
			menu->selLinkApp()->setTVDaemonYScale(link_o2x_tv_daemon_yscale);
			menu->selLinkApp()->setTVDaemonVXScale(link_o2x_tv_daemon_vxscale);
			menu->selLinkApp()->setTVDaemonVYScale(link_o2x_tv_daemon_vyscale);
			menu->selLinkApp()->setTVDaemonDelay(link_o2x_tv_daemon_first_delay);
			menu->selLinkApp()->setTVDaemonDelay(link_o2x_tv_daemon_delay);
			menu->selLinkApp()->setTVDaemonTweakOnlyOnce(link_o2x_tv_daemon_tweak_only_once);
			//			Deprecated:
//			menu->selLinkApp()->setTVDaemonForce720Pitch(link_o2x_tv_daemon_force_720_pitch);
//			menu->selLinkApp()->setTVDaemonStubbornFix(link_o2x_tv_daemon_stubborn_fix);
			menu->selLinkApp()->setTVDaemonAlwaysStarted(link_o2x_tv_daemon_always_started);
			menu->selLinkApp()->setTVDaemonEnableTVMode(link_o2x_tv_daemon_enable_tv_mode);

			// senquack - new Open2X joy2xd daemon allows control of all GP2X buttons from 
			// 	a USB gamepad.  It should be configurable because it will greatly interfere
			// 	with apps like Picodrive that already know how to use USB joysticks.
			menu->selLinkApp()->setJoy2xdStatus(link_o2x_link_uses_joy2xd);
			menu->selLinkApp()->setJoy2xdHidesJs0(link_o2x_joy2xd_hides_js0);
		}

#ifdef DEBUG
		cout << "New Section: " << newSection << endl;
#endif
		//if section changed move file and update link->file
		if (oldSection!=newSection) {
			vector<string>::iterator newSectionIndex = find(menu->sections.begin(),menu->sections.end(),newSection);
			if (newSectionIndex==menu->sections.end()) return;
			string newFileName = "sections/"+newSection+"/"+linkTitle;
			uint x=2;
			while (fileExists(newFileName)) {
				string id = "";
				stringstream ss; ss << x; ss >> id;
				newFileName = "sections/"+newSection+"/"+linkTitle+id;
				x++;
			}
			rename(menu->selLinkApp()->file.c_str(),newFileName.c_str());
			menu->selLinkApp()->file = newFileName;
#ifdef DEBUG
			cout << "New section index: " << newSectionIndex - menu->sections.begin() << endl;
#endif
			menu->linkChangeSection(menu->selLinkIndex(), menu->selSectionIndex(), newSectionIndex - menu->sections.begin());
		}
		menu->selLinkApp()->save();
		sync();

		ledOff();
	}
}

void GMenu2X::deleteLink() {
	//senquack - SD stuff
	if (fwType == "open2x")
	{
		if ((strcasecmp((menu->selSection()).c_str(), "applications") == 0) ||
			(strcasecmp((menu->selSection()).c_str(), "settings") == 0) )
		{
			MessageBox mb(this,tr["Deleting links in this section is not allowed."]);
			mb.exec();
			return;
		}
	}

	if (menu->selLinkApp()!=NULL) {
		MessageBox mb(this, tr.translate("Deleting $1",menu->selLink()->getTitle().c_str(),NULL)+"\n"+tr["Are you sure?"], menu->selLink()->getIconPath());
		mb.buttons[ACTION_B] = tr["Yes"];
		mb.buttons[ACTION_X] = tr["No"];
		if (mb.exec() == ACTION_B) {
			ledOn();
			menu->deleteSelectedLink();
			sync();
			ledOff();
		}
	}
}

void GMenu2X::addSection() {
	InputDialog id(this,tr["Insert a name for the new section"]);
	if (id.exec()) {
		//only if a section with the same name does not exist
		if (find(menu->sections.begin(),menu->sections.end(),id.input)==menu->sections.end()) {
			//section directory doesn't exists
			ledOn();
			if (menu->addSection(id.input)) {
				menu->setSectionIndex( menu->sections.size()-1 ); //switch to the new section
				sync();
			}
			ledOff();
		}
	}
}

//senquack - SD stuff
void GMenu2X::renameSection() {
	if (fwType == "open2x")
	{
		if ((strcasecmp((menu->selSection()).c_str(), "applications") == 0) ||
			(strcasecmp((menu->selSection()).c_str(), "settings") == 0) )
		{
			MessageBox mb(this,tr["Renaming of this section is not allowed."]);
			mb.exec();
			return;
		}
	}

	InputDialog id(this,tr["Insert a new name for this section"],menu->selSection());
	if (id.exec()) {
		//only if a section with the same name does not exist & !samename
		if (menu->selSection()!=id.input && find(menu->sections.begin(),menu->sections.end(),id.input)==menu->sections.end()) {
			//section directory doesn't exists

			string newsectiondir;
			string sectiondir;

			if (fwType == "open2x" && o2x_store_links_on_SD)
			{
				if (o2x_SD_read_only)
				{
					MessageBox mb(this,tr["Free space low on SD. Writes are disabled."]);
					mb.exec();
					return;
				}
				newsectiondir = o2x_links_folder + "/sections/"+id.input;
				sectiondir = o2x_links_folder + "/sections/"+menu->selSection();
			} 
			else
			{
				newsectiondir = "sections/"+id.input;
				sectiondir = "sections/"+menu->selSection();
			}

			ledOn();

			cout << "renaming section " << sectiondir << " to " << newsectiondir << endl;
			fflush(NULL);

			//senquack - having failures of renaming here, dunno why it does it this way
//			if (rename(sectiondir.c_str(), "tmpsection")==0 && rename("tmpsection", newsectiondir.c_str())==0) {
			if (rename(sectiondir.c_str(), newsectiondir.c_str())==0) {
				string oldpng = sectiondir+".png", newpng = newsectiondir+".png";
				string oldicon = sc.getSkinFilePath(oldpng), newicon = sc.getSkinFilePath(newpng);
				if (!oldicon.empty() && newicon.empty()) {
					newicon = oldicon;
          				newicon.replace(newicon.find(oldpng), oldpng.length(), newpng);

					if (!fileExists(newicon)) {
						rename(oldicon.c_str(), "tmpsectionicon");
						rename("tmpsectionicon", newicon.c_str());
						sc.move("skin:"+oldpng, "skin:"+newpng);
					}
				}
				menu->sections[menu->selSectionIndex()] = id.input;
				sync();
			} else
			{
				cout << "Rename failed." << endl;
				fflush(NULL);
			}
			ledOff();
		}
	}
}

//senquack - SD stuff
//Modified to allow optional open2x storage of links/sections on SDs
void GMenu2X::deleteSection() {
	if (fwType=="open2x")
	{
		if ((strcasecmp((menu->selSection()).c_str(), "applications") == 0) ||
			(strcasecmp((menu->selSection()).c_str(), "settings") == 0) )
		{
			MessageBox mb(this,tr["Deletion of this section is not allowed."]);
			mb.exec();
			return;
		}
	}

	string tree_path;

	if (fwType == "open2x" && o2x_store_links_on_SD)
	{
		tree_path = o2x_links_folder + "/sections/" + menu->selSection();
	}
	else
	{
		tree_path = path + "sections/" + menu->selSection();
	}

	MessageBox mb(this,tr["You will lose all the links in this section."]+"\n"+tr["Are you sure?"]);
	mb.buttons[ACTION_B] = tr["Yes"];
	mb.buttons[ACTION_X] = tr["No"];
	if (mb.exec() == ACTION_B) {
		ledOn();
//		if (rmtree(path+"sections/"+menu->selSection())) {
		if (rmtree(tree_path.c_str())) {
			menu->deleteSelectedSection();
			sync();
		}
		ledOff();
	}
}

void GMenu2X::scanner() {
	Surface scanbg(bg);
	drawButton(&scanbg, "x", tr["Exit"],
	drawButton(&scanbg, "b", "", 5)-10);
	scanbg.write(font,tr["Link Scanner"],halfX,7,SFontHAlignCenter,SFontVAlignMiddle);

	uint lineY = 42;

#ifdef _TARGET_PANDORA
	//char *configpath = pnd_conf_query_searchpath();
#else
	if (confInt["menuClock"]<200) {
		setClock(200);
		scanbg.write(font,tr["Raising cpu clock to 200Mhz"],5,lineY);
		scanbg.blit(s,0,0);
		s->flip();
		lineY += 26;
	}

	scanbg.write(font,tr["Scanning SD filesystem..."],5,lineY);
	scanbg.blit(s,0,0);
	s->flip();
	lineY += 26;

	vector<string> files;
	scanPath("/mnt/sd",&files);

	//Onyl gph firmware has nand
	if (fwType=="gph" && !f200) {
		scanbg.write(font,tr["Scanning NAND filesystem..."],5,lineY);
		scanbg.blit(s,0,0);
		s->flip();
		lineY += 26;
		scanPath("/mnt/nand",&files);
	}

	stringstream ss;
	ss << files.size();
	string str = "";
	ss >> str;
	scanbg.write(font,tr.translate("$1 files found.",str.c_str(),NULL),5,lineY);
	lineY += 26;
	scanbg.write(font,tr["Creating links..."],5,lineY);
	scanbg.blit(s,0,0);
	s->flip();
	lineY += 26;

	string path, file;
	string::size_type pos;
	uint linkCount = 0;

	ledOn();
	for (uint i = 0; i<files.size(); i++) {
		pos = files[i].rfind("/");
		if (pos!=string::npos && pos>0) {
			path = files[i].substr(0, pos+1);
			file = files[i].substr(pos+1, files[i].length());
			if (menu->addLink(path,file,"found "+file.substr(file.length()-3,3)))
				linkCount++;
		}
	}

	ss.clear();
	ss << linkCount;
	ss >> str;
	scanbg.write(font,tr.translate("$1 links created.",str.c_str(),NULL),5,lineY);
	scanbg.blit(s,0,0);
	s->flip();
	lineY += 26;

	if (confInt["menuClock"]<200) {
		setClock(confInt["menuClock"]);
		scanbg.write(font,tr["Decreasing cpu clock"],5,lineY);
		scanbg.blit(s,0,0);
		s->flip();
		lineY += 26;
	}

	sync();
	ledOff();
#endif

	bool close = false;
	while (!close) {
		input.update();
		if (input[ACTION_START] || input[ACTION_B] || input[ACTION_X]) close = true;
		usleep(30000);
	}
}

void GMenu2X::scanPath(string path, vector<string> *files) {
	DIR *dirp;
	struct stat st;
	struct dirent *dptr;
	string filepath, ext;

	if (path[path.length()-1]!='/') path += "/";
	if ((dirp = opendir(path.c_str())) == NULL) return;

	while ((dptr = readdir(dirp))) {
		if (dptr->d_name[0]=='.')
			continue;
		filepath = path+dptr->d_name;
		int statRet = stat(filepath.c_str(), &st);
		if (S_ISDIR(st.st_mode))
			scanPath(filepath, files);
		if (statRet != -1) {
			ext = filepath.substr(filepath.length()-4,4);
#ifdef TARGET_GP2X
			if (ext==".gpu" || ext==".gpe")
#else
			if (ext==".pxml")
#endif
				files->push_back(filepath);
		}
	}

	closedir(dirp);
}

unsigned short GMenu2X::getBatteryLevel() {
#ifdef TARGET_GP2X
	if (batteryHandle<=0) return 0;

	if (f200) {
		MMSP2ADC val;
		read(batteryHandle, &val, sizeof(MMSP2ADC));

		if (val.batt==0) return 5;
		if (val.batt==1) return 3;
		if (val.batt==2) return 1;
		if (val.batt==3) return 0;
	} else {
		int battval = 0;
		unsigned short cbv, min=900, max=0;

		for (int i = 0; i < BATTERY_READS; i ++) {
			if ( read(batteryHandle, &cbv, 2) == 2) {
				battval += cbv;
				if (cbv>max) max = cbv;
				if (cbv<min) min = cbv;
			}
		}

		battval -= min+max;
		battval /= BATTERY_READS-2;

		if (battval>=850) return 6;
		if (battval>780) return 5;
		if (battval>740) return 4;
		if (battval>700) return 3;
		if (battval>690) return 2;
		if (battval>680) return 1;
	}
	return 0;
#else
	return 6; //AC Power
#endif
}

void GMenu2X::setInputSpeed() {
	input.setInterval(150);
	//senquack - in Open2X, VolUp/Down change the volume scaler mode and need to be slower:
	if (fwType == "open2x")
	{
		input.setInterval(150,  ACTION_VOLDOWN);
		input.setInterval(150,  ACTION_VOLUP  );
	}
	else
	{
		input.setInterval(30,  ACTION_VOLDOWN);
		input.setInterval(30,  ACTION_VOLUP  );
	}
	input.setInterval(30,  ACTION_A      );
	input.setInterval(500, ACTION_START  );
	input.setInterval(500, ACTION_SELECT );
	input.setInterval(300, ACTION_X      );
	input.setInterval(300,  ACTION_Y      );
	input.setInterval(1000,ACTION_B      );
	//joy.setInterval(1000,ACTION_CLICK  );
	//senquack - delay here is too big:
//	input.setInterval(300, ACTION_L      );
//	input.setInterval(300, ACTION_R      );
	input.setInterval(150, ACTION_L      );
	input.setInterval(150, ACTION_R      );
	SDL_EnableKeyRepeat(1,150);
}

void GMenu2X::applyRamTimings() {
	cout << "applying fast ram timings" << endl;
	fflush(NULL);
#ifdef TARGET_GP2X
	// 6 4 1 1 1 2 2
	if (gp2x_mem!=0) {
		int tRC = 5, tRAS = 3, tWR = 0, tMRD = 0, tRFC = 0, tRP = 1, tRCD = 1;
		gp2x_memregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
		gp2x_memregs[0x3804>>1] = ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);
	}
#endif
}

void GMenu2X::applyDefaultTimings() {
	cout << "applying default ram timings" << endl;
	fflush(NULL);
#ifdef TARGET_GP2X
	// 8 16 3 8 8 8 8
	if (gp2x_mem!=0) {
		int tRC = 7, tRAS = 15, tWR = 2, tMRD = 7, tRFC = 7, tRP = 7, tRCD = 7;
		gp2x_memregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
		gp2x_memregs[0x3804>>1] = ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);
	}
#endif
}

void GMenu2X::setClock(unsigned mhz) {
	mhz = constrain(mhz,50,confInt["maxClock"]);
#ifdef TARGET_GP2X
	if (gp2x_mem!=0) {
		unsigned v;
		unsigned mdiv,pdiv=3,scale=0;
		mhz*=1000000;
		mdiv=(mhz*pdiv)/GP2X_CLK_FREQ;
		mdiv=((mdiv-8)<<8) & 0xff00;
		pdiv=((pdiv-2)<<2) & 0xfc;
		scale&=3;
		v=mdiv | pdiv | scale;
		MEM_REG[0x910>>1]=v;
	}
#endif
}

void GMenu2X::setGamma(int gamma) {
#ifdef TARGET_GP2X
	float fgamma = (float)constrain(gamma,1,100)/10;
	fgamma = 1 / fgamma;
	MEM_REG[0x2880>>1]&=~(1<<12);
	MEM_REG[0x295C>>1]=0;

	for (int i=0; i<256; i++) {
		unsigned char g = (unsigned char)(255.0*pow(i/255.0,fgamma));
		unsigned short s = (g<<8) | g;
		MEM_REG[0x295E>>1]= s;
		MEM_REG[0x295E>>1]= g;
	}
#endif
}

int GMenu2X::getVolume() {
	int vol = -1;
	int soundDev = open("/dev/mixer", O_RDONLY);
	if (soundDev != -1) {
		ioctl(soundDev, SOUND_MIXER_READ_PCM, &vol);
		close(soundDev);
		if (vol != -1) {
			//just return one channel , not both channels, they're hopefully the same anyways
			return vol & 0xFF;
		}
	}
	return vol;
}

void GMenu2X::setVolume(int vol) {
	vol = constrain(vol,0,100);
	int soundDev = open("/dev/mixer", O_RDWR);
	if (soundDev != -1) {
		vol = (vol << 8) | vol;
		ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
		close(soundDev);
	}
}

void GMenu2X::setVolumeScaler(int scale) {
	scale = constrain(scale,0,MAX_VOLUME_SCALE_FACTOR);
	int soundDev = open("/dev/mixer", O_WRONLY);
	if (soundDev != -1) {
		ioctl(soundDev, SOUND_MIXER_PRIVATE2, &scale);
		close(soundDev);
	}
}

int GMenu2X::getVolumeScaler() {
	int currentscalefactor = -1;
	int soundDev = open("/dev/mixer", O_RDONLY);
	if (soundDev != -1) {
		ioctl(soundDev, SOUND_MIXER_PRIVATE1, &currentscalefactor);
		close(soundDev);
	}
	return currentscalefactor;
}

//senquack - new stick click emulation
void GMenu2X::setStickClickEmulation(int mode) {
	int gpioDev = open("/dev/GPIO", O_WRONLY);
	if (gpioDev == -1)
		return;	
		
	if (mode == OPEN2X_STICK_CLICK_DISABLED		||
			mode == OPEN2X_STICK_CLICK_DPAD			||
			mode == OPEN2X_STICK_CLICK_VOLUPDOWN)
	{
		ioctl(gpioDev, GP2X_SET_STICK_CLICK_EMULATION_MODE, &mode);
	}
	else
	{
		cout << "Invalid parameter to GMenu2X::setStickClickEmulation" << endl;
	}

	close(gpioDev);
}

//senquack - new open2x gpio remapping support:
void GMenu2X::remapGpioButton(int button, int remapped_to) {
	if (button < 0 || button > 18 ||
			remapped_to < -1 || remapped_to > 18)
		return;

	int gpioDev = open("/dev/GPIO", O_WRONLY);
	if (gpioDev == -1)
		return;	
		
	ioctl(gpioDev, GP2X_REMAP_BUTTON_00 + button, &remapped_to);

	close(gpioDev);
}

void GMenu2X::disableGpioRemapping(void)
{
	int gpioDev = open("/dev/GPIO", O_WRONLY);
	if (gpioDev == -1)
		return;	
		
	int param = 0;
	ioctl(gpioDev, GP2X_DISABLE_REMAPPING, &param);

	close(gpioDev);
}

// senquack - new Open2X support for configurable caching of upper memory so mmuhack.o is 
// 	no longer necessary:
void GMenu2X::setUpperMemoryCaching(int caching_enabled) {
	int gpioDev = open("/dev/GPIO", O_WRONLY);
	if (gpioDev == -1)
		return;	
		
	ioctl(gpioDev, GP2X_SET_UPPER_MEMORY_CACHING, &caching_enabled);
	close(gpioDev);
}

//senquack - New functions used under Open2X.  Everytime GMenu2X starts, it takes a look at all processes
//		currently running.  It tells the kernel all the PIDs it finds (excluding GMenu2X itself).  The kernel
//		keeps this list in memory when programs are run.  If the user presses a specific button combo, the
//		kernel will kill off all PIDs not in this whitelist and then restart GMenu2X.  This is so users can
//		recover from program crashes or hangs (or when a program won't let you exit!).
void GMenu2X::clearPidWhitelist(void) {
	int gpioDev = open("/dev/GPIO", O_WRONLY);
	if (gpioDev == -1)
		return;	
		
	ioctl(gpioDev, GP2X_WHITELIST_CLEAR, 0);
	close(gpioDev);
}

void GMenu2X::addPidToWhitelist(int pid) {
	int gpioDev = open("/dev/GPIO", O_WRONLY);
	if (gpioDev == -1)
		return;	
		
	ioctl(gpioDev, GP2X_WHITELIST_ADD, &pid);
	close(gpioDev);
}

void GMenu2X::generatePidWhitelist(void)
{
	clearPidWhitelist();

	char buf[8192], buf2[8192];
	FILE *f;
	int pid;

	// Get the output of the ps command as a file
	f = popen("ps -w", "r");

	if (f)
	{
		if (!feof(f))
		{
			// skip empty lines
			fscanf(f, "%8192[\n\r]", buf);
			// read first line and discard it (it is just column descriptors)
			fscanf(f, "%8192[^\n^\r]", buf);
			// chomp endline
			fscanf(f, "%8192[\n\r]", buf);

			// read remaining lines
			while (!feof(f))
			{
//				// skip empty lines, chomp any endlines
//				fscanf(f, "%8192[\n\r]", buf);
				fscanf(f, "%8192[^\n^\r]", buf);
//				cout << "Processing line: " << endl << buf << endl;
				// add all PIDs listed to the whitelist, but exclude gmenu2x's and ps's 
				if (!strstr(buf, "gmenu2x") && !strstr(buf, "ps -w"))
				{
					sscanf(buf, "%d %8192[^\n^\r]", &pid, buf2);
					cout << "GMenu2X: Adding PID " << pid << " to whitelist" << endl;
					addPidToWhitelist(pid);
				}
				// chomp any endlines
				fscanf(f, "%8192[\n\r]", buf);
			}
		}
		pclose(f);
	}
}

string GMenu2X::getExePath() {
	if (path.empty()) {
		char buf[255];
		int l = readlink("/proc/self/exe",buf,255);

		path = buf;
		path = path.substr(0,l);
		l = path.rfind("/");
		path = path.substr(0,l+1);
	}
	return path;
}

// senquack - moved to utilities.cpp
//string GMenu2X::getDiskFree() {
//	stringstream ss;
//	string df = "";
//	struct statvfs b;
//
//	int ret = statvfs("/mnt/sd", &b);
//	if (ret==0) {
//		//DKS - fixing a bug where wrong sizes are reported because of overflowed int
//		printf("b.f_bfree: %d\tb.f_bsize: %d\n", b.f_bfree, b.f_bsize);
//		printf("b.f_blocks: %d\tb.f_frsize:%d\n", b.f_blocks, b.f_frsize);
////		unsigned long free = b.f_bfree*b.f_frsize/1048576;
//		double free = (double)b.f_bfree * (double)b.f_bsize / 1048576.0;
////		unsigned long total = b.f_blocks*b.f_frsize/1048576;
//		double total = (double)b.f_blocks * (double)b.f_frsize / 1048576.0;
////		ss << free << "/" << total << "MB";
//		ss << (unsigned long)free << "/" << (unsigned long)total << "MB";
//		ss >> df;
//	} else cout << "\033[0;34mGMENU2X:\033[0;31m statvfs failed with error '" << strerror(errno) << "'\033[0m" << endl;
//	return df;
//}

int GMenu2X::drawButton(IconButton *btn, int x, int y) {
	if (y<0) y = resY+y;
	btn->setPosition(x, y-7);
	btn->paint();
	return x+btn->getRect().w+6;
}

int GMenu2X::drawButton(Surface *s, string btn, string text, int x, int y) {
	if (y<0) y = resY+y;
	SDL_Rect re = {x, y-7, 0, 16};
	if (sc.skinRes("imgs/buttons/"+btn+".png") != NULL) {
		sc["imgs/buttons/"+btn+".png"]->blit(s, x, y-7);
		re.w = sc["imgs/buttons/"+btn+".png"]->raw->w+3;
		s->write(font, text, x+re.w, y, SFontHAlignLeft, SFontVAlignMiddle);
		re.w += font->getTextWidth(text);
	}
	return x+re.w+6;
}

int GMenu2X::drawButtonRight(Surface *s, string btn, string text, int x, int y) {
	if (y<0) y = resY+y;
	if (sc.skinRes("imgs/buttons/"+btn+".png") != NULL) {
		x -= 16;
		sc["imgs/buttons/"+btn+".png"]->blit(s, x, y-7);
		x -= 3;
		s->write(font, text, x, y, SFontHAlignRight, SFontVAlignMiddle);
		return x-6-font->getTextWidth(text);
	}
	return x-6;
}

void GMenu2X::drawScrollBar(uint pagesize, uint totalsize, uint pagepos, uint top, uint height) {
	if (totalsize<=pagesize) return;

	s->rectangle(resX-8, top, 7, height, skinConfColors["selectionBg"]);

	//internal bar total height = height-2
	//bar size
	uint bs = (height-2) * pagesize / totalsize;
	//bar y position
	uint by = (height-2) * pagepos / totalsize;
	by = top+2+by;
	if (by+bs>top+height-2) by = top+height-2-bs;


	s->box(resX-6, by, 3, bs, skinConfColors["selectionBg"]);
}

void GMenu2X::drawTitleIcon(string icon, bool skinRes, Surface *s) {
	if (s==NULL) s = this->s;

	Surface *i = NULL;
	if (!icon.empty()) {
		if (skinRes)
			i = sc.skinRes(icon);
		else
			i = sc[icon];
	}

	if (i==NULL)
		i = sc.skinRes("icons/generic.png");

	i->blit(s,4,(skinConfInt["topBarHeight"]-32)/2);
}

void GMenu2X::writeTitle(string title, Surface *s) {
	if (s==NULL) s = this->s;
	s->write(font,title,40, skinConfInt["topBarHeight"]/4, SFontHAlignLeft, SFontVAlignMiddle);
}

void GMenu2X::writeSubTitle(string subtitle, Surface *s) {
	if (s==NULL) s = this->s;
	s->write(font,subtitle,40, skinConfInt["topBarHeight"]/4*3, SFontHAlignLeft, SFontVAlignMiddle);
}

void GMenu2X::drawTopBar(Surface *s) {
	if (s==NULL) s = this->s;

	Surface *bar = sc.skinRes("imgs/topbar.png");
	if (bar != NULL)
		bar->blit(s, 0, 0);
	else
		s->box(0, 0, resX, skinConfInt["topBarHeight"], skinConfColors["topBarBg"]);
}

void GMenu2X::drawBottomBar(Surface *s) {
	if (s==NULL) s = this->s;

	Surface *bar = sc.skinRes("imgs/bottombar.png");
	if (bar != NULL)
		bar->blit(s, 0, resY-bar->raw->h);
	else
		s->box(0, resY-20, resX, 20, skinConfColors["bottomBarBg"]);
}
