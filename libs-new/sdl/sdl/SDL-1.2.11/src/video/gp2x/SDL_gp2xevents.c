/*
  SDL - Simple DirectMedia Layer
  Copyright (C) 1997-2004 Sam Lantinga

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Sam Lantinga
  slouken@libsdl.org
*/

#ifdef SAVE_RCSID
static char rcsid =
  "@(#) $Id: SDL_gp2xevents.c,v 1.1 2005/10/20 22:19:08 paeryn Exp $";
#endif
/* Handle the event stream, converting console events into SDL events */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/* For parsing /proc */
#include <dirent.h>
#include <ctype.h>

#include <linux/vt.h>
#include <linux/kd.h>
#include <linux/keyboard.h>

//DKS
#include <linux/input.h>
#include "tslib.h"

#include "SDL.h"
#include "SDL_mutex.h"
#include "../SDL_sysvideo.h"
#include "../../events/SDL_events_c.h"
#include "../../events/SDL_sysevents.h"
#include "SDL_gp2xvideo.h"
#include "SDL_gp2xevents_c.h"
 
//DKS - uncommented this to match 1.2.9
#include "SDL_gp2xkeys.h"

//#include "SDL_keysym.h"

//DKS - added for USB keyboard support
#define tty_fd		this->hidden->keyboard_fd

//DKS - this remains zero until a sample containing a non-zero pressure value has been
//			read from the touchscreen.  It is a work-around for a bug.  The bug is that
//			about 10% of the time the touchscreen device is opened, it returns nothing but
//			zero values until closed and reopened.  Once it starts returning true values, it
//			never needs to be closed and reopened.  Code uses this flag to see if it should
//			try to close and reopen the touchscreen device several times a second or so until
//			it does get good values.
unsigned int gotten_nonzero_sample = 0;

/***********
 *** Mouse stuff
 **********/
//=====================================================
// Temporarily enable mouse debug for open/close
//#define DEBUG_MOUSE 1
//=====================================================

static enum {
  MOUSE_NONE = -1,
  MOUSE_MSC,	/* Note: GPM uses the MSC protocol */
  MOUSE_PS2,
  MOUSE_IMPS2,
  MOUSE_MS,
  MOUSE_BM,
  MOUSE_ELO,
  MOUSE_TSLIB,
  NUM_MOUSE_DRVS
} mouse_drv = MOUSE_NONE;


//DKS
//void GP2X_CloseMouse(_THIS)
//{
//  if (this->hidden->mouse_fd > 0) {
//    close(this->hidden->mouse_fd);
//  }
//  this->hidden->mouse_fd = -1;
//}
void GP2X_CloseMouse(_THIS)
{
  if (this->hidden->ts_dev != NULL) {
    ts_close(this->hidden->ts_dev);
    this->hidden->ts_dev = NULL;
    this->hidden->mouse_fd = -1;
  }
  if (this->hidden->mouse_fd > 0) {
    close(this->hidden->mouse_fd);
  }
  this->hidden->mouse_fd = -1;
}


/* rcg06112001 Set up IMPS/2 mode, if possible. This gives
 *  us access to the mousewheel, etc. Returns zero if
 *  writes to device failed, but you still need to query the
 *  device to see which mode it's actually in.
 */
static int set_imps2_mode(int fd)
{
  /* If you wanted to control the mouse mode (and we do :)  ) ...
     Set IMPS/2 protocol:
     {0xf3,200,0xf3,100,0xf3,80}
     Reset mouse device:
     {0xFF}
  */
  Uint8 set_imps2[] = {0xf3, 200, 0xf3, 100, 0xf3, 80};
  Uint8 reset = 0xff;
  fd_set fdset;
  struct timeval tv;
  int retval = 0;

  if (write(fd, &set_imps2, sizeof(set_imps2)) == sizeof(set_imps2)) {
    if (write(fd, &reset, sizeof (reset)) == sizeof (reset)) {
      retval = 1;
    }
  }

  /* Get rid of any chatter from the above */
  FD_ZERO(&fdset);
  FD_SET(fd, &fdset);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  while (select(fd+1, &fdset, 0, 0, &tv) > 0) {
    char temp[32];
    read(fd, temp, sizeof(temp));
  }

  return retval;
}


/* Returns true if the mouse uses the IMPS/2 protocol */
static int detect_imps2(int fd)
{
  int imps2;

  imps2 = 0;

  if (getenv("SDL_MOUSEDEV_IMPS2")) {
    imps2 = 1;
  }
  if (!imps2) {
    Uint8 query_ps2 = 0xF2;
    fd_set fdset;
    struct timeval tv;

    /* Get rid of any mouse motion noise */
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    while (select(fd+1, &fdset, 0, 0, &tv) > 0) {
      char temp[32];
      read(fd, temp, sizeof(temp));
    }

    /* Query for the type of mouse protocol */
    if (write(fd, &query_ps2, sizeof (query_ps2)) == sizeof (query_ps2)) {
      Uint8 ch = 0;

      /* Get the mouse protocol response */
      do {
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (select(fd+1, &fdset, 0, 0, &tv) < 1) {
	  break;
	}
      } while ((read(fd, &ch, sizeof (ch)) == sizeof (ch)) &&
	       ((ch == 0xFA) || (ch == 0xAA)) );

      /* Experimental values (Logitech wheelmouse) */
#ifdef DEBUG_MOUSE
      fprintf(stderr, "Last mouse mode: 0x%x\n", ch);
#endif
      if ((ch == 3) || (ch == 4)) {
	imps2 = 1;
      }
    }
  }
  return imps2;
}

static struct stat sbuf;

//DKS
//int GP2X_OpenMouse(_THIS)
//{
//  int i;
//  int touchscreen_fd = -1;
//  const char *mousedev;
//  const char *mousedrv;
//
//  mousedrv = getenv("SDL_MOUSEDRV");
//  mousedev = getenv("SDL_MOUSEDEV");
//  this->hidden->mouse_fd = -1;
//
//  /* STD MICE */
//#ifdef DEBUG_MOUSE
//  fputs("SDL_GP2X: Looking for a mouse\n", stderr);
//#endif
//  if (mousedev == NULL) {
//    /* FIXME someday... allow multiple mice in this driver */
//    static const char * const ps2mice[] = {
//      "/dev/input/mouse/0", "/dev/usbmouse", "/dev/psaux", NULL
//    };
//    /* Now try to use a modern PS/2 mouse */
//    for (i=0; (this->hidden->mouse_fd < 0) && ps2mice[i]; ++i ) {
//      this->hidden->mouse_fd = open(ps2mice[i], O_RDWR, 0);
//      if (this->hidden->mouse_fd < 0) {
//	this->hidden->mouse_fd = open(ps2mice[i], O_RDONLY, 0);
//      }
//      if (this->hidden->mouse_fd >= 0) {
//	/* rcg06112001 Attempt to set IMPS/2 mode */
//	if (i == 0) {
//	  set_imps2_mode(this->hidden->mouse_fd);
//	}
//	if (detect_imps2(this->hidden->mouse_fd)) {
//#ifdef DEBUG_MOUSE
//	  fprintf(stderr, "Using IMPS2 mouse\n");
//#endif
//	  mouse_drv = MOUSE_IMPS2;
//	} else {
//#ifdef DEBUG_MOUSE
//	  fprintf(stderr, "Using PS2 mouse\n");
//#endif
//	  mouse_drv = MOUSE_PS2;
//	}
//	this->hidden->mouse_type = 1; /* GP2X_MOUSE_STD */
//      }
//    }
//    if (this->hidden->mouse_fd < 0) {
//      touchscreen_fd = open("/dev/touchscreen/wm97xx", O_RDONLY | O_NOCTTY);
//      if (touchscreen_fd) {
//	this->hidden->mouse_fd = touchscreen_fd;
//        read_calibration();
//	mouse_drv = MOUSE_TSLIB;
//	this->hidden->mouse_type = 2; /*GP2X_MOUSE_TOUCHSCREEN */
//#ifdef DEBUG_MOUSE
//	fputs("SDL_GP2X: F-200 touchscreen emulating mouse\n", stderr);
//#endif
//      }
//    }
//  }
//  if (this->hidden->mouse_fd < 0) {
//    this->hidden->mouse_type = 0; /* GP2X_MOUSE_NONE */
//    mouse_drv = MOUSE_NONE;
//#ifdef DEBUG_MOUSE
//	fputs("SDL_GP2X: No mice found\n", stderr);
//#endif
//  }
//  return this->hidden->mouse_fd;
//}
int GP2X_OpenMouse(_THIS)
{
  int i;
  const char *mousedev;
  const char *mousedrv;
  
  mousedrv = getenv("SDL_MOUSEDRV");
  mousedev = getenv("SDL_MOUSEDEV");
  this->hidden->mouse_fd = -1;
  
  /* STD MICE */
#ifdef DEBUG_MOUSE
  fputs("SDL_GP2X: Looking for a mouse\n", stderr);
#endif
  if (mousedev == NULL) {
    /* FIXME someday... allow multiple mice in this driver */
    static const char * const ps2mice[] = {
		//      "/dev/input/mouse/0", NULL
		// DKS - this is actually supposed to me mouse0 not mouse/0
			"/dev/input/mouse0", NULL
      //      "/dev/input/mouse/0", "/dev/usbmouse", "/dev/psaux", NULL
    };
    /* Now try to use a modern PS/2 mouse */
    for (i=0; (this->hidden->mouse_fd < 0) && ps2mice[i]; ++i ) {
      this->hidden->mouse_fd = open(ps2mice[i], O_RDWR, 0);
      if (this->hidden->mouse_fd < 0) {
			this->hidden->mouse_fd = open(ps2mice[i], O_RDONLY, 0);
      }
      if (this->hidden->mouse_fd >= 0) {
			/* rcg06112001 Attempt to set IMPS/2 mode */
			if (i == 0) {
			  set_imps2_mode(this->hidden->mouse_fd);
			}
			if (detect_imps2(this->hidden->mouse_fd)) {
#ifdef DEBUG_MOUSE
			  fprintf(stderr, "Using IMPS2 mouse\n");
#endif
			  mouse_drv = MOUSE_IMPS2;
			} else {
#ifdef DEBUG_MOUSE
			  fprintf(stderr, "Using PS2 mouse\n");
#endif
			  mouse_drv = MOUSE_PS2;
			}
				this->hidden->mouse_type = 1; /* GP2X_MOUSE_STD */
      }
    }
    if (this->hidden->mouse_fd < 0) {
      /* try the F-200 touchscreen */
      mousedev = "/dev/touchscreen/wm97xx";
      if (!stat(mousedev, &sbuf)) {
			this->hidden->ts_dev = ts_open(mousedev, 1);
			if ((this->hidden->ts_dev != NULL) &&
			 (ts_config(this->hidden->ts_dev) >= 0)) {
				  mouse_drv = MOUSE_TSLIB;
				  this->hidden->mouse_fd = ts_fd(this->hidden->ts_dev);
				  this->hidden->mouse_type = 2; /*GP2X_MOUSE_TOUCHSCREEN */
#ifdef DEBUG_MOUSE
				  fputs("SDL_GP2X: F-200 touchscreen emulating mouse\n", stderr);
#endif
			}
      }
    }
  }
  if (this->hidden->mouse_fd < 0) {
    this->hidden->mouse_type = 0; /* GP2X_MOUSE_NONE */
    mouse_drv = MOUSE_NONE;
#ifdef DEBUG_MOUSE
    fputs("SDL_GP2X: No mice found\n", stderr);
#endif
  }
  return this->hidden->mouse_fd;
}

//============================
#undef DEBUG_MOUSE
//============================

static int posted = 0;

void GP2X_vgamousecallback(int button, int relative, int dx, int dy)
{
  int button_1, button_3;
  int button_state;
  int state_changed;
  int i;
  Uint8 state;

  //senquack - support for new option to disable mouse motion events from touchscreen
  if (( dx || dy ) && current_video->hidden->mouse_motion_events_enabled) {
    posted += SDL_PrivateMouseMotion(0, relative, dx, dy);
  }

  //senquack - support for new option to disable mouse button events from touchscreen
	if (current_video->hidden->mouse_button_events_enabled)
	{
	  /* Swap button 1 and 3 */
	  button_1 = (button & 0x04) >> 2;
	  button_3 = (button & 0x01) << 2;
	  button &= ~0x05;
	  button |= (button_1|button_3);

	  /* See what changed */
	  button_state = SDL_GetMouseState(NULL, NULL);
	  state_changed = button_state ^ button;
	  for (i=0; i<8; ++i ) {
		 if (state_changed & (1<<i)) {
			if (button & (1<<i)) {
				state = SDL_PRESSED;
			} else {
				state = SDL_RELEASED;
			}
			posted += SDL_PrivateMouseButton(state, i+1, 0, 0);
		 }
	  }
	}
}

//DKS - pulled from my modifications to SDL 1.2.9
static void handle_tslib(_THIS)
{
  struct ts_sample sample;
  int button;

//DKS - changed this because there's no point sampling higher than 100hz, the driver
//	doesn't update more often than that
////  while (ts_read(this->hidden->ts_dev, &sample, 1) > 0) {

  if (ts_read(this->hidden->ts_dev, &sample, 1) > 0) {
    button = (sample.pressure > 0) ? 1 : 0;
    button <<= 2;  /* must report it as button 3 */
    // Store (semi-)raw touchscreen position (pre mouse clipping)
    this->hidden->touch_x = sample.x;
    this->hidden->touch_y = sample.y;
    this->hidden->touch_pressure = sample.pressure;

	 //DKS - once we have gotten a valid sample from the touchscreen, this flag tells 
	 // 	handle_mouse to stop trying to reopen the mouse device every second or so
	 // 	so as a workaround fora non-responsive device file.	
	 if ( sample.pressure > 0 )
	 {
		gotten_nonzero_sample = 1;
	 }

    sample.x = ((sample.x * this->hidden->invxscale) >> 16) +
               this->hidden->x_offset;
    sample.y = ((sample.y * this->hidden->invyscale) >> 16) +
               this->hidden->y_offset;
    GP2X_vgamousecallback(button, 0, sample.x, sample.y);
  }  
  return;
}

/* For now, use MSC, PS/2, and MS protocols
   Driver adapted from the SVGAlib mouse driver code (taken from gpm, etc.)
*/
//DKS - pulled newer version from better 1.2.9 code
//static void handle_mouse(_THIS)
//{
//  static int start = 0;
//  static unsigned char mousebuf[BUFSIZ];
//  static int relative = 1;
//  TS_EVENT *ts_event;
//
//  int i, nread;
//  int button = 0;
//  int dx = 0, dy = 0;
//  int packetsize = 0;
//  int realx, realy;
//
//  /* Figure out the mouse packet size */
//  switch (mouse_drv) {
//  case MOUSE_NONE:
//    /* Ack! */
//    read(this->hidden->mouse_fd, mousebuf, BUFSIZ);
//    return;
//  case MOUSE_MSC:
//    packetsize = 5;
//    break;
//  case MOUSE_IMPS2:
//    packetsize = 4;
//    break;
//  case MOUSE_PS2:
//  case MOUSE_MS:
//  case MOUSE_BM:
//    packetsize = 3;
//    break;
//  case MOUSE_ELO:
//    packetsize = 0;
//    break;
//  case MOUSE_TSLIB:
//    packetsize= sizeof(TS_EVENT);
//    break;
//  case NUM_MOUSE_DRVS:
//    /* Uh oh.. */
//    packetsize = 0;
//    break;
//  }
//
//  /* Read as many packets as possible */
//  nread = read(this->hidden->mouse_fd, &mousebuf[start], BUFSIZ-start);
//  if (nread < 0) {
//    return;
//  }
//  nread += start;
//#ifdef DEBUG_MOUSE
//  fprintf(stderr, "Read %d bytes from mouse, start = %d\n", nread, start);
//#endif
//  for (i=0; i<(nread-(packetsize-1)); i += packetsize) {
//    switch (mouse_drv) {
//    case MOUSE_NONE:
//      break;
//    case MOUSE_MSC:
//      /* MSC protocol has 0x80 in high byte */
//      if ((mousebuf[i] & 0xF8) != 0x80) {
//	/* Go to next byte */
//	i -= (packetsize-1);
//	continue;
//      }
//      /* Get current mouse state */
//      button = (~mousebuf[i]) & 0x07;
//      dx =   (signed char)(mousebuf[i+1]) +
//	(signed char)(mousebuf[i+3]);
//      dy = -((signed char)(mousebuf[i+2]) +
//	     (signed char)(mousebuf[i+4]));
//      relative = 1;
//      break;
//    case MOUSE_PS2:
//      /* PS/2 protocol has nothing in high byte */
//      if ((mousebuf[i] & 0xC0) != 0) {
//	/* Go to next byte */
//	i -= (packetsize-1);
//	continue;
//      }
//      /* Get current mouse state */
//      button = (mousebuf[i] & 0x04) >> 1 | /*Middle*/
//	(mousebuf[i] & 0x02) >> 1 | /*Right*/
//	(mousebuf[i] & 0x01) << 2;  /*Left*/
//      dx = (mousebuf[i] & 0x10) ?
//	mousebuf[i+1] - 256 : mousebuf[i+1];
//      dy = (mousebuf[i] & 0x20) ?
//	-(mousebuf[i+2] - 256) : -mousebuf[i+2];
//      relative = 1;
//      break;
//    case MOUSE_IMPS2:
//      /* Get current mouse state */
//      button = (mousebuf[i] & 0x04) >> 1 | /*Middle*/
//	(mousebuf[i] & 0x02) >> 1 | /*Right*/
//	(mousebuf[i] & 0x01) << 2 | /*Left*/
//	(mousebuf[i] & 0x40) >> 3 | /* 4 */
//	(mousebuf[i] & 0x80) >> 3;  /* 5 */
//      dx = (mousebuf[i] & 0x10) ?
//	mousebuf[i+1] - 256 : mousebuf[i+1];
//      dy = (mousebuf[i] & 0x20) ?
//	-(mousebuf[i+2] - 256) : -mousebuf[i+2];
//      switch (mousebuf[i+3]&0x0F) {
//      case 0x0E: /* DX = +1 */
//      case 0x02: /* DX = -1 */
//	break;
//      case 0x0F: /* DY = +1 (map button 4) */
//	GP2X_vgamousecallback(button | (1<<3),
//			      1, 0, 0);
//	break;
//      case 0x01: /* DY = -1 (map button 5) */
//	GP2X_vgamousecallback(button | (1<<4),
//			      1, 0, 0);
//	break;
//      }
//      break;
//    case MOUSE_MS:
//    case MOUSE_BM:
//    case MOUSE_ELO:
//      dx = 0;
//      dy = 0;
//      break;
//    case MOUSE_TSLIB:
//      ts_event = (TS_EVENT*)mousebuf;
//      button = (ts_event->pressure ? 0x04 : 0x00);
//      if (ts_cal[6] == 65536) {   // This seems to be what the F200 uses
//	dx = (ts_cal[2] + ts_cal[0] * (int)ts_event->x) >> 16;
//	dy = (ts_cal[5] + ts_cal[4] * (int)ts_event->y) >> 16;
//      } else {
//	dx = (ts_cal[2] + ts_cal[0] * (int)ts_event->x) / ts_cal[6];
//	dy = (ts_cal[5] + ts_cal[4] * (int)ts_event->y) / ts_cal[6];
//      }
//      /*
//      fprintf(stderr, "GP2X_TS: button %d, pos %d,%d\n", button, dx ,dy);
//      */
//      if ((dx <0) || (dx > 320) || (dy < 0) || (dy > 240)) {
//	dx = dy = 0;
//	relative = 1;
//      } else {
//	relative = 0;
//	dx = ((dx * this->hidden->invxscale) >> 16) + this->hidden->x_offset;
//	dy = ((dy * this->hidden->invyscale) >> 16) + this->hidden->y_offset;
//      }
//      break;
//    case NUM_MOUSE_DRVS:
//      /* Uh oh.. */
//      dx = 0;
//      dy = 0;
//      break;
//    }
//    GP2X_vgamousecallback(button, relative, dx, dy);
//  }
//  if ( i < nread ) {
//    memcpy(mousebuf, &mousebuf[i], (nread-i));
//    start = (nread-i);
//  } else {
//    start = 0;
//  }
//  return;
//}
static void handle_mouse(_THIS)
{
  static int start = 0;
  static unsigned char mousebuf[BUFSIZ];
  static int relative = 1;

  int i, nread;
  int button = 0;
  int dx = 0, dy = 0;
  int packetsize = 0;

  //DKS for determining if we should read the touchscreen or wait some more
	static struct timeval lastread = { 0, 0 };
	static struct timeval now = { 0, 0 };
	unsigned int sample_interval;
	static unsigned int touchscreen_reopen_ctr = 0;

  /* Figure out the mouse packet size */
  switch (mouse_drv) {
  case MOUSE_NONE:
    /* Ack! */
    read(this->hidden->mouse_fd, mousebuf, BUFSIZ);
    return;
  case MOUSE_MSC:
    packetsize = 5;
    break;
  case MOUSE_IMPS2:
    packetsize = 4;
    break;
  case MOUSE_PS2:
  case MOUSE_MS:
  case MOUSE_BM:
    packetsize = 3;
    break;
  case MOUSE_ELO:
    packetsize = 0;
    break;
  case MOUSE_TSLIB:
	//DKS we want to limit the frequency with which the touchscreen is read, as it never
	//data faster than 100hz anyways.
#define TSLIB_INTERVAL 5000 	// every 5 ms, which allows us to capture all events but
	 									// not waste cycles 
		gettimeofday(&now, NULL);
		sample_interval = (now.tv_sec * 1000000 + now.tv_usec) -
									(lastread.tv_sec * 1000000 + lastread.tv_usec);


		if ( !gotten_nonzero_sample && touchscreen_reopen_ctr >= 100)
		{
			// We are running on a GP2X with touchscreen, but we have not gotten 
			// a sample with non-zero pressure yet.  This workaround will continue
			// to try to close and reopen the touchscreen device every second or so
			// until we get a non-zero sample.  Once we do, don't worry about it as
			// the unit should be OK after that.  Once a second or so should not cause
			// too much overhead for games not using the touchscreen.

#ifdef DEBUG_MOUSE
			fprintf(stderr, "SDL_GP2X: Not gotten good F200 touchscreen sample yet.\n\t\tReopening device just in case.\n");
#endif
			ts_close(this->hidden->ts_dev);
			this->hidden->ts_dev = ts_open("/dev/touchscreen/wm97xx", 1);
			this->hidden->mouse_fd = this->hidden->ts_dev->fd;
			touchscreen_reopen_ctr = 0;
		}

		if (sample_interval > TSLIB_INTERVAL)
		{
			// 5ms has passed, read a touchscreen sample
			handle_tslib(this);
			gettimeofday(&lastread, NULL);
			if (!gotten_nonzero_sample)
			{
				touchscreen_reopen_ctr++;			
			}
		} else {
			dx = 0;
			dy = 0;
		}

//    handle_tslib(this);
    return; /* nothing left to do */
  case NUM_MOUSE_DRVS:
    /* Uh oh.. */
    packetsize = 0;
    break;
  }

  /* Read as many packets as possible */
  nread = read(this->hidden->mouse_fd, &mousebuf[start], BUFSIZ-start);
  if (nread < 0) {
    return;
  }
  nread += start;
#ifdef DEBUG_MOUSE
  fprintf(stderr, "Read %d bytes from mouse, start = %d\n", nread, start);
#endif
  for (i=0; i<(nread-(packetsize-1)); i += packetsize) {
    switch (mouse_drv) {
    case MOUSE_NONE:
      break;
    case MOUSE_MSC:
      /* MSC protocol has 0x80 in high byte */
      if ((mousebuf[i] & 0xF8) != 0x80) {
	/* Go to next byte */
	i -= (packetsize-1);
	continue;
      }
      /* Get current mouse state */
      button = (~mousebuf[i]) & 0x07;
      dx =   (signed char)(mousebuf[i+1]) +
	(signed char)(mousebuf[i+3]);
      dy = -((signed char)(mousebuf[i+2]) +
	     (signed char)(mousebuf[i+4]));
      relative = 1;
      break;
    case MOUSE_PS2:
      /* PS/2 protocol has nothing in high byte */
      if ((mousebuf[i] & 0xC0) != 0) {
	/* Go to next byte */
	i -= (packetsize-1);
	continue;
      }
      /* Get current mouse state */
      button = (mousebuf[i] & 0x04) >> 1 | /*Middle*/
	(mousebuf[i] & 0x02) >> 1 | /*Right*/
	(mousebuf[i] & 0x01) << 2;  /*Left*/
      dx = (mousebuf[i] & 0x10) ?
	mousebuf[i+1] - 256 : mousebuf[i+1];
      dy = (mousebuf[i] & 0x20) ?
	-(mousebuf[i+2] - 256) : -mousebuf[i+2];
      relative = 1;
      break;
    case MOUSE_IMPS2:
      /* Get current mouse state */
      button = (mousebuf[i] & 0x04) >> 1 | /*Middle*/
	(mousebuf[i] & 0x02) >> 1 | /*Right*/
	(mousebuf[i] & 0x01) << 2 | /*Left*/
	(mousebuf[i] & 0x40) >> 3 | /* 4 */
	(mousebuf[i] & 0x80) >> 3;  /* 5 */
      dx = (mousebuf[i] & 0x10) ?
	mousebuf[i+1] - 256 : mousebuf[i+1];
      dy = (mousebuf[i] & 0x20) ?
	-(mousebuf[i+2] - 256) : -mousebuf[i+2];
      switch (mousebuf[i+3]&0x0F) {
      case 0x0E: /* DX = +1 */
      case 0x02: /* DX = -1 */
	break;
      case 0x0F: /* DY = +1 (map button 4) */
	GP2X_vgamousecallback(button | (1<<3),
			      1, 0, 0);
	break;
      case 0x01: /* DY = -1 (map button 5) */
	GP2X_vgamousecallback(button | (1<<4),
			      1, 0, 0);
	break;
      }
      break;
    case MOUSE_MS:
    case MOUSE_BM:
    case MOUSE_ELO:
      dx = 0;
      dy = 0;
      break;
    case MOUSE_TSLIB:
    case NUM_MOUSE_DRVS:
      /* Uh oh.. */
      dx = 0;
      dy = 0;
      break;
    }
    GP2X_vgamousecallback(button, relative, dx, dy);
  }
  if ( i < nread ) {
    memcpy(mousebuf, &mousebuf[i], (nread-i));
    start = (nread-i);
  } else {
    start = 0;
  }
  return;
}

/* The translation tables from a console scancode to a SDL keysym */
#define NUM_VGAKEYMAPS	(1<<KG_CAPSSHIFT)
static Uint16 vga_keymap[NUM_VGAKEYMAPS][NR_KEYS];
static SDLKey keymap[128];
static Uint16 keymap_temp[128]; /* only used at startup */
static SDL_keysym *TranslateKey(int scancode, SDL_keysym *keysym);

//DKS - new stuff for USB keyboard
static int GP2X_SetRawKeyboardMode(_THIS);
static void GP2X_SetPreviousKeyboardMode(_THIS);
static int GP2X_InRawKeyboardMode(_THIS);

//DKS - pulled from 1.2.9's newer USB keyboard support I added
int GP2X_InRawKeyboardMode(_THIS)
{
	return((tty_fd >= 0) && (this->hidden->saved_kbd_mode >= 0));
}

//DKS - pulled this from virgin 1.2.9 SDL fbcon 
int GP2X_SetRawKeyboardMode(_THIS)
{
	struct termios keyboard_termios;

	/* Set medium-raw keyboard mode */
	if ( (tty_fd >= 0) && !GP2X_InRawKeyboardMode(this) ) {

//DKS - don't need this on GP2X
//		/* Switch to the correct virtual terminal */
//		if ( current_vt > 0 ) {
//			struct vt_stat vtstate;
//
//			if ( ioctl(keyboard_fd, VT_GETSTATE, &vtstate) == 0 ) {
//				saved_vt = vtstate.v_active;
//			}
//			if ( ioctl(keyboard_fd, VT_ACTIVATE, current_vt) == 0 ) {
//				ioctl(keyboard_fd, VT_WAITACTIVE, current_vt);
//			}
//		}

		/* Set the terminal input mode */
		if ( tcgetattr(tty_fd, &(this->hidden->saved_kbd_termios)) < 0 ) {
			SDL_SetError("Unable to get terminal attributes");
			if ( tty_fd > 0 ) {
				close(tty_fd);
			}
			tty_fd = -1;
			return(-1);
		}
		if ( ioctl(tty_fd, KDGKBMODE, &(this->hidden->saved_kbd_mode)) < 0 ) {
			SDL_SetError("Unable to get current keyboard mode");
			if ( tty_fd > 0 ) {
				close(tty_fd);
			}
			tty_fd = -1;
			return(-1);
		}
		keyboard_termios = this->hidden->saved_kbd_termios;
		keyboard_termios.c_lflag &= ~(ICANON | ECHO | ISIG);
		keyboard_termios.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
		keyboard_termios.c_cc[VMIN] = 0;
		keyboard_termios.c_cc[VTIME] = 0;
		if (tcsetattr(tty_fd, TCSAFLUSH, &keyboard_termios) < 0) {
			GP2X_CloseKeyboard(this);
			SDL_SetError("Unable to set terminal attributes");
			return(-1);
		}
		/* This will fail if we aren't root or this isn't our tty */
		if ( ioctl(tty_fd, KDSKBMODE, K_MEDIUMRAW) < 0 ) {
			GP2X_CloseKeyboard(this);
			SDL_SetError("Unable to set keyboard in raw mode");
			return(-1);
		}
		if ( ioctl(tty_fd, KDSETMODE, KD_GRAPHICS) < 0 ) {
			GP2X_CloseKeyboard(this);
			SDL_SetError("Unable to set keyboard in graphics mode");
			return(-1);
		}
	}
	return(tty_fd);

}

//DKS - pulled from GPH's code
void GP2X_SetPreviousKeyboardMode(_THIS)
{
	if(!GP2X_InRawKeyboardMode(this))
		return;

	if (tty_fd >= 0) {
		ioctl(tty_fd, KDSETMODE, KD_TEXT);
		ioctl(tty_fd, KDSKBMODE, this->hidden->saved_kbd_mode);
		tcsetattr(tty_fd, TCSAFLUSH, &(this->hidden->saved_kbd_termios));
	}

	this->hidden->saved_kbd_mode = -1;
}

/* Ugh, we have to duplicate the kernel's keysym mapping code...
   Oh, it's not so bad. :-)

   FIXME: Add keyboard LED handling code
*/
static void GP2X_vgainitkeymaps(int fd)
{
  struct kbentry entry;
  int map, i;

  /* Don't do anything if we are passed a closed keyboard */
  if (fd < 0) {
    return;
  }

  /* Load all the keysym mappings */
  for (map=0; map<NUM_VGAKEYMAPS; ++map) {
    memset(vga_keymap[map], 0, NR_KEYS*sizeof(Uint16));

    for (i = 0; i < NR_KEYS; ++i) {
      entry.kb_table = map;
      entry.kb_index = i;
      if (ioctl(fd, KDGKBENT, &entry) == 0) {
	/* fill keytemp. This replaces SDL_fbkeys.h */
	if ((map == 0) && (i<128)) {
	  keymap_temp[i] = entry.kb_value;
	}
	/* The "Enter" key is a special case */
	if (entry.kb_value == K_ENTER) {
	  entry.kb_value = K(KT_ASCII,13);
	}
	/* Handle numpad specially as well */
	if (KTYP(entry.kb_value) == KT_PAD) {
	  switch (entry.kb_value) {
	  case K_P0:
	  case K_P1:
	  case K_P2:
	  case K_P3:
	  case K_P4:
	  case K_P5:
	  case K_P6:
	  case K_P7:
	  case K_P8:
	  case K_P9:
	    vga_keymap[map][i]=entry.kb_value;
	    vga_keymap[map][i]+= '0';
	    break;
	  case K_PPLUS:
	    vga_keymap[map][i]=K(KT_ASCII,'+');
	    break;
	  case K_PMINUS:
	    vga_keymap[map][i]=K(KT_ASCII,'-');
	    break;
	  case K_PSTAR:
	    vga_keymap[map][i]=K(KT_ASCII,'*');
	    break;
	  case K_PSLASH:
	    vga_keymap[map][i]=K(KT_ASCII,'/');
	    break;
	  case K_PENTER:
	    vga_keymap[map][i]=K(KT_ASCII,'\r');
	    break;
	  case K_PCOMMA:
	    vga_keymap[map][i]=K(KT_ASCII,',');
	    break;
	  case K_PDOT:
	    vga_keymap[map][i]=K(KT_ASCII,'.');
	    break;
	  default:
	    break;
	  }
	}
	/* Do the normal key translation */
	if ((KTYP(entry.kb_value) == KT_LATIN) ||
	    (KTYP(entry.kb_value) == KT_ASCII) ||
	    (KTYP(entry.kb_value) == KT_LETTER)) {
	  vga_keymap[map][i] = entry.kb_value;
	}
      }
    }
  }
}

//DKS - not used anymore
//int GP2X_InGraphicsMode(_THIS)
//{
//  return ((keyboard_fd >= 0) && (saved_kbd_mode >= 0));
//}

//DKS - not used anymore
//int GP2X_EnterGraphicsMode(_THIS)
//{
//  struct termios keyboard_termios;
//
//  /* Set medium-raw keyboard mode */
//  if ((keyboard_fd >= 0) && !GP2X_InGraphicsMode(this)) {
//
//    /* Set the terminal input mode */
//    if (tcgetattr(keyboard_fd, &saved_kbd_termios) < 0) {
//      SDL_SetError("Unable to get terminal attributes");
//      if (keyboard_fd > 0) {
//	close(keyboard_fd);
//      }
//      keyboard_fd = -1;
//      return(-1);
//    }
//    keyboard_termios = saved_kbd_termios;
//
//    keyboard_termios.c_lflag = 0;
//    keyboard_termios.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
//
//    keyboard_termios.c_cc[VTIME]    = 0;   // 문자 사이의 timer를 disable
//    keyboard_termios.c_cc[VMIN]     = 1;   // 최소 5 문자 받을 때까진 blocking
//
//    tcflush(keyboard_fd, TCIFLUSH);
//
//    if (tcsetattr(keyboard_fd, TCSAFLUSH, &keyboard_termios) < 0) {
//      GP2X_CloseKeyboard(this);
//      SDL_SetError("Unable to set terminal attributes");
//      return(-1);
//    }
//
//
//  }
//  return(keyboard_fd);
//}

//DKS - not used anymore
//void GP2X_LeaveGraphicsMode(_THIS)
//{
//  tcsetattr(keyboard_fd, TCSAFLUSH, &saved_kbd_termios);
//  saved_kbd_mode = -1;
//}

//DKS - changed to newer function from after I completed the USB keyboard support in 1.2.9
//int GP2X_OpenKeyboard(_THIS)
//{
//  /* Open only if not already opened */
//  if ( keyboard_fd < 0 ) {
//    static const char * const tty0[] = { "/dev/tty0", "/dev/vc/0", NULL };
//    static const char * const vcs[] = { "/dev/vc/%d", "/dev/tty%d", NULL };
//    int i, tty0_fd;
//
//    /* Try to query for a free virtual terminal */
//    tty0_fd = -1;
//    for (i = 0; tty0[i] && (tty0_fd < 0); ++i) {
//      tty0_fd = open(tty0[i], O_WRONLY, 0);
//    }
//
//    if ( keyboard_fd < 0 ) {
//      /* Last resort, maybe our tty is a usable VT */
//      current_vt = 0;
//      keyboard_fd = open("/dev/tty", O_RDWR);
//    }
//#ifdef DEBUG_KEYBOARD
//    fprintf(stderr, "Current VT: %d\n", current_vt);
//#endif
//    saved_kbd_mode = -1;
//
//  }
//  return(keyboard_fd);
//}
//DKS - pulled from GPH's code
int GP2X_OpenKeyboard(_THIS)
{
	//DKS - it should be noted tty_fd is really an alias for keyboard_fd, and keyboard_fd
	// is an int buried in SDL_gp2xvideo.c

	/* Open only if not already opened */
 	if ( tty_fd < 0 ) {
		static const char * const tty0[] = { "/dev/tty0", "/dev/vc/0", NULL };
		int i;

		/* Try to query for a free virtual terminal */
		tty_fd = -1;
		for ( i=0; tty0[i] && (tty_fd < 0); ++i ) {
			tty_fd = open(tty0[i],  O_RDWR | O_NDELAY, 0);
		}

		if(tty_fd >= 0) {
			/* Set up keymap */
			GP2X_vgainitkeymaps(tty_fd);
		}

 		this->hidden->saved_kbd_mode = -1;
		
		GP2X_SetRawKeyboardMode(this);
 	}

 	return(tty_fd);
}

//DKS - changed to newer function from after I completed the USB keyboard support in 1.2.9
//void GP2X_CloseKeyboard(_THIS)
//{
//  if (keyboard_fd >= 0) {
//    GP2X_LeaveGraphicsMode(this);
//    if (keyboard_fd > 0) {
//      close(keyboard_fd);
//    }
//  }
//  keyboard_fd = -1;
//}
void GP2X_CloseKeyboard(_THIS)
{
	if ( tty_fd >= 0 ) {
		GP2X_SetPreviousKeyboardMode(this);
		if ( tty_fd >= 0 ) close(tty_fd);
	}
	tty_fd = -1;
}


//DKS - changed to newer function from after I completed the USB keyboard support in 1.2.9
//static void handle_keyboard(_THIS)
//{
//  unsigned char keybuf[BUFSIZ];
//  int i, nread;
//  int pressed;
//  int scancode;
//  SDL_keysym keysym;
//
//  nread = read(keyboard_fd, keybuf, BUFSIZ);
//
//  // ghcstop add: 041206  확장키의 경우는 처리를 하지 않는다.
//  if (nread >= 3) return;
//
//  for (i = 0; i<nread; ++i) {
//    scancode = keybuf[i] & 0x7F;
//    if (keybuf[i] & 0x80) {
//      pressed = SDL_RELEASED;
//    } else {
//      pressed = SDL_PRESSED;
//    }
//    TranslateKey(scancode, &keysym);
//
//    /* Handle Alt-FN for vt switch */
//    switch (keysym.sym) {
//    case SDLK_F1:
//    case SDLK_F2:
//    case SDLK_F3:
//    case SDLK_F4:
//    case SDLK_F5:
//    case SDLK_F6:
//    case SDLK_F7:
//    case SDLK_F8:
//    case SDLK_F9:
//    case SDLK_F10:
//    case SDLK_F11:
//    case SDLK_F12:
//      if (SDL_GetModState() & KMOD_ALT) {
//	if (pressed) {
//	  switch_vt(this, (keysym.sym-SDLK_F1)+1);
//	}
//	break;
//      }
//      /* Fall through to normal processing */
//    default:
//      posted += SDL_PrivateKeyboard(pressed, &keysym);
//      break;
//    }
//  }
//}
//DKS - pulled from virgin SDL fbcon/  and modified
static void handle_keyboard(_THIS)
{
	unsigned char keybuf[BUFSIZ];
	int i, nread;
	int pressed;
	int scancode;
	SDL_keysym keysym;

	nread = read(tty_fd, keybuf, BUFSIZ);
	for ( i=0; i<nread; ++i ) {
		scancode = keybuf[i] & 0x7F;
		if ( keybuf[i] & 0x80 ) {
			pressed = SDL_RELEASED;
		} else {
			pressed = SDL_PRESSED;
		}
		TranslateKey(scancode, &keysym);
//DKS - on GP2X, no virtual terminals
//		/* Handle Alt-FN for vt switch */
//		switch (keysym.sym) {
//		    case SDLK_F1:
//		    case SDLK_F2:
//		    case SDLK_F3:
//		    case SDLK_F4:
//		    case SDLK_F5:
//		    case SDLK_F6:
//		    case SDLK_F7:
//		    case SDLK_F8:
//		    case SDLK_F9:
//		    case SDLK_F10:
//		    case SDLK_F11:
//		    case SDLK_F12:
//			if ( SDL_GetModState() & KMOD_ALT ) {
//				if ( pressed ) {
//					switch_vt(this, (keysym.sym-SDLK_F1)+1);
//				}
//				break;
//			}
			/* Fall through to normal processing */
//		    default:
			posted += SDL_PrivateKeyboard(pressed, &keysym);
//			break;
//		}
	}
}

//DKS - reenabled this function that was commented out.. it is the same as the one I 
// had pulled from fbcon/ when finishing USB keyboard stuff in 1.2.9
void GP2X_InitOSKeymap(_THIS)
{
  int i;

  /* Initialize the Linux key translation table */

  /* First get the ascii keys and others not well handled */
  for (i=0; i<SDL_TABLESIZE(keymap); ++i) {
    switch(i) {
      /* These aren't handled by the x86 kernel keymapping (?) */
    case SCANCODE_PRINTSCREEN:
      keymap[i] = SDLK_PRINT;
      break;
    case SCANCODE_BREAK:
      keymap[i] = SDLK_BREAK;
      break;
    case SCANCODE_BREAK_ALTERNATIVE:
      keymap[i] = SDLK_PAUSE;
      break;
    case SCANCODE_LEFTSHIFT:
      keymap[i] = SDLK_LSHIFT;
      break;
    case SCANCODE_RIGHTSHIFT:
      keymap[i] = SDLK_RSHIFT;
      break;
    case SCANCODE_LEFTCONTROL:
      keymap[i] = SDLK_LCTRL;
      break;
    case SCANCODE_RIGHTCONTROL:
      keymap[i] = SDLK_RCTRL;
      break;
    case SCANCODE_RIGHTWIN:
      keymap[i] = SDLK_RSUPER;
      break;
    case SCANCODE_LEFTWIN:
      keymap[i] = SDLK_LSUPER;
      break;
    case 127:
      keymap[i] = SDLK_MENU;
      break;
      /* this should take care of all standard ascii keys */
    default:
      keymap[i] = KVAL(vga_keymap[0][i]);
      break;
    }
  }
  for (i=0; i<SDL_TABLESIZE(keymap); ++i) {
    switch(keymap_temp[i]) {
    case K_F1:  keymap[i] = SDLK_F1;  break;
    case K_F2:  keymap[i] = SDLK_F2;  break;
    case K_F3:  keymap[i] = SDLK_F3;  break;
    case K_F4:  keymap[i] = SDLK_F4;  break;
    case K_F5:  keymap[i] = SDLK_F5;  break;
    case K_F6:  keymap[i] = SDLK_F6;  break;
    case K_F7:  keymap[i] = SDLK_F7;  break;
    case K_F8:  keymap[i] = SDLK_F8;  break;
    case K_F9:  keymap[i] = SDLK_F9;  break;
    case K_F10: keymap[i] = SDLK_F10; break;
    case K_F11: keymap[i] = SDLK_F11; break;
    case K_F12: keymap[i] = SDLK_F12; break;

    case K_DOWN:  keymap[i] = SDLK_DOWN;  break;
    case K_LEFT:  keymap[i] = SDLK_LEFT;  break;
    case K_RIGHT: keymap[i] = SDLK_RIGHT; break;
    case K_UP:    keymap[i] = SDLK_UP;    break;

    case K_P0:     keymap[i] = SDLK_KP0; break;
    case K_P1:     keymap[i] = SDLK_KP1; break;
    case K_P2:     keymap[i] = SDLK_KP2; break;
    case K_P3:     keymap[i] = SDLK_KP3; break;
    case K_P4:     keymap[i] = SDLK_KP4; break;
    case K_P5:     keymap[i] = SDLK_KP5; break;
    case K_P6:     keymap[i] = SDLK_KP6; break;
    case K_P7:     keymap[i] = SDLK_KP7; break;
    case K_P8:     keymap[i] = SDLK_KP8; break;
    case K_P9:     keymap[i] = SDLK_KP9; break;
    case K_PPLUS:  keymap[i] = SDLK_KP_PLUS; break;
    case K_PMINUS: keymap[i] = SDLK_KP_MINUS; break;
    case K_PSTAR:  keymap[i] = SDLK_KP_MULTIPLY; break;
    case K_PSLASH: keymap[i] = SDLK_KP_DIVIDE; break;
    case K_PENTER: keymap[i] = SDLK_KP_ENTER; break;
    case K_PDOT:   keymap[i] = SDLK_KP_PERIOD; break;

    case K_SHIFT:  if ( keymap[i] != SDLK_RSHIFT )
	keymap[i] = SDLK_LSHIFT;
      break;
    case K_SHIFTL: keymap[i] = SDLK_LSHIFT; break;
    case K_SHIFTR: keymap[i] = SDLK_RSHIFT; break;
    case K_CTRL:  if ( keymap[i] != SDLK_RCTRL )
	keymap[i] = SDLK_LCTRL;
      break;
    case K_CTRLL:  keymap[i] = SDLK_LCTRL;  break;
    case K_CTRLR:  keymap[i] = SDLK_RCTRL;  break;
    case K_ALT:    keymap[i] = SDLK_LALT;   break;
    case K_ALTGR:  keymap[i] = SDLK_RALT;   break;

    case K_INSERT: keymap[i] = SDLK_INSERT;   break;
    case K_REMOVE: keymap[i] = SDLK_DELETE;   break;
    case K_PGUP:   keymap[i] = SDLK_PAGEUP;   break;
    case K_PGDN:   keymap[i] = SDLK_PAGEDOWN; break;
    case K_FIND:   keymap[i] = SDLK_HOME;     break;
    case K_SELECT: keymap[i] = SDLK_END;      break;

    case K_NUM:  keymap[i] = SDLK_NUMLOCK;   break;
    case K_CAPS: keymap[i] = SDLK_CAPSLOCK;  break;

    case K_F13:   keymap[i] = SDLK_PRINT;     break;
    case K_HOLD:  keymap[i] = SDLK_SCROLLOCK; break;
    case K_PAUSE: keymap[i] = SDLK_PAUSE;     break;

    case 127: keymap[i] = SDLK_BACKSPACE; break;

    default: break;
    }
  }
}
//DKS - commenting this out, it is oooold
//void GP2X_InitOSKeymap(_THIS)
//{
//  int i;
//
//  /* Initialize the Linux key translation table */
//
//  /* First get the ascii keys and others not well handled */
//  for (i=0; i<SDL_TABLESIZE(keymap); ++i)  // 128개 까정
//    {
//      keymap[i] = 0; // all key map clear ==> 몽땅 SDLK_UNKNOWN로 일단 세팅
//    }
//
//  keymap[0] =  SDLK_UNKNOWN	;
//  keymap[3] =  SDLK_CTRL_C	;
//  keymap[8] =  SDLK_BACKSPACE	;
//  keymap[9] =  SDLK_TAB		;
//  keymap[12] = SDLK_CLEAR		;
//  keymap[13] = SDLK_RETURN	;
//  keymap[19] = SDLK_PAUSE		;
//  keymap[27] = SDLK_ESCAPE    ;
//
//
//  for (i=32; i<=64; ++i)  // ' ' 에서부터 '@'까지의 32개
//    {
//      keymap[i] = i; // all key map clear ==> 몽땅 SDLK_UNKNOWN로 일단 세팅
//    }
//
//  for (i=91; i<=122; ++i)  // '[' 에서부터 'z'까지의 32개
//    {
//      keymap[i] = i; // all key map clear ==> 몽땅 SDLK_UNKNOWN로 일단 세팅
//    }
//
//  keymap[127] = SDLK_DELETE    ;
//
//
//}
//

//DKS - ostensibly the same as what I used for 1.2.9's USB keyboard support, but I had 
// left the unicode stuff uncommented so I will bring the 1.2.9 version back here as it's
// already been tested.
//static SDL_keysym *TranslateKey(int scancode, SDL_keysym *keysym)
//{
//  /* Set the keysym information */
//  keysym->scancode = scancode;
//  keysym->sym = keymap[scancode];
//  keysym->mod = KMOD_NONE;
//
//  /* If UNICODE is on, get the UNICODE value for the key */
//  keysym->unicode = 0;
//#if 0 // 041206: ghcstop delete 왜냐? 아예 keymap을 내맘대로 serial console용으로 변화시켰기 때문
//  if ( SDL_TranslateUNICODE ) {
//    int map;
//    SDLMod modstate;
//
//    modstate = SDL_GetModState();
//    map = 0;
//    if ( modstate & KMOD_SHIFT ) {
//      map |= (1<<KG_SHIFT);
//    }
//    if ( modstate & KMOD_CTRL ) {
//      map |= (1<<KG_CTRL);
//    }
//    if ( modstate & KMOD_ALT ) {
//      map |= (1<<KG_ALT);
//    }
//    if ( modstate & KMOD_MODE ) {
//      map |= (1<<KG_ALTGR);
//    }
//    if ( KTYP(vga_keymap[map][scancode]) == KT_LETTER ) {
//      if ( modstate & KMOD_CAPS ) {
//	map ^= (1<<KG_SHIFT);
//      }
//    }
//    if ( KTYP(vga_keymap[map][scancode]) == KT_PAD ) {
//      if ( modstate & KMOD_NUM ) {
//	keysym->unicode=KVAL(vga_keymap[map][scancode]);
//      }
//    } else {
//      keysym->unicode = KVAL(vga_keymap[map][scancode]);
//    }
//  }
//#endif
//  return(keysym);
//}
//DKS - pulled from virgin 1.2.9 
static SDL_keysym *TranslateKey(int scancode, SDL_keysym *keysym)
{
	/* Set the keysym information */
	keysym->scancode = scancode;
	keysym->sym = keymap[scancode];
	keysym->mod = KMOD_NONE;

	/* If UNICODE is on, get the UNICODE value for the key */
	keysym->unicode = 0;
	if ( SDL_TranslateUNICODE ) {
		int map;
		SDLMod modstate;

		modstate = SDL_GetModState();
		map = 0;
		if ( modstate & KMOD_SHIFT ) {
			map |= (1<<KG_SHIFT);
		}
		if ( modstate & KMOD_CTRL ) {
			map |= (1<<KG_CTRL);
		}
		if ( modstate & KMOD_ALT ) {
			map |= (1<<KG_ALT);
		}
		if ( modstate & KMOD_MODE ) {
			map |= (1<<KG_ALTGR);
		}
		if ( KTYP(vga_keymap[map][scancode]) == KT_LETTER ) {
			if ( modstate & KMOD_CAPS ) {
				map ^= (1<<KG_SHIFT);
			}
		}
		if ( KTYP(vga_keymap[map][scancode]) == KT_PAD ) {
			if ( modstate & KMOD_NUM ) {
				keysym->unicode=KVAL(vga_keymap[map][scancode]);
			}
		} else {
			keysym->unicode = KVAL(vga_keymap[map][scancode]);
		}
	}
	return(keysym);
}

//DKS - updated to newer code pulled from 1.2.9
//void GP2X_PumpEvents(_THIS)
//{
//  fd_set fdset;
//  int max_fd;
//  static struct timeval zero;
//
//  do {
//    posted = 0;
//
//    FD_ZERO(&fdset);
//    max_fd = 0;
//    /*** TODO
//    if (this->hidden->keyboard_fd >= 0) {
//
//      FD_SET(this->hidden->keyboard_fd, &fdset);
//      if (max_fd < this->hidden->keyboard_fd) {
//	max_fd = this->hidden->keyboard_fd;
//      }
//    }
//    ***/
//    if (this->hidden->mouse_fd >= 0) {
//      FD_SET(this->hidden->mouse_fd, &fdset);
//      if (max_fd < this->hidden->mouse_fd) {
//	max_fd = this->hidden->mouse_fd;
//      }
//    }
//    if (select(max_fd+1, &fdset, NULL, NULL, &zero) > 0) {
//      /*** TODO
//      if (this->hidden->keyboard_fd >= 0) {
//	if (FD_ISSET(this->hidden->keyboard_fd, &fdset)) {
//	  handle_keyboard(this);
//	}
//      }
//      ***/
//      if (this->hidden->mouse_fd >= 0) {
//	if (FD_ISSET(this->hidden->mouse_fd, &fdset)) {
//	  handle_mouse(this);
//	}
//      }
//    }
//  } while (posted);
//}
void GP2X_PumpEvents(_THIS)
{
  fd_set fdset;
  int max_fd;
  static struct timeval zero;

  do {
    posted = 0;

    FD_ZERO(&fdset);
    max_fd = 0;
    if (this->hidden->keyboard_fd >= 0) {
      FD_SET(this->hidden->keyboard_fd, &fdset);
      if (max_fd < this->hidden->keyboard_fd) {
	max_fd = this->hidden->keyboard_fd;
      }
    }
    if (this->hidden->mouse_fd >= 0) {
      FD_SET(this->hidden->mouse_fd, &fdset);
      if (max_fd < this->hidden->mouse_fd) {
	max_fd = this->hidden->mouse_fd;
      }
    }
    if (select(max_fd+1, &fdset, NULL, NULL, &zero) > 0) {
      if (this->hidden->keyboard_fd >= 0) {
	if (FD_ISSET(this->hidden->keyboard_fd, &fdset)) {
	  handle_keyboard(this);
	}
      }
      if (this->hidden->mouse_fd >= 0) {
	if (FD_ISSET(this->hidden->mouse_fd, &fdset)) {
	  handle_mouse(this);
	}
      }
    }
  } while (posted);
}


/* end of SDL_gp2xevents.c ... */
