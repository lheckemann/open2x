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
 "@(#) $Id: SDL_gp2xvideo.h,v 1.4 2004/01/04 16:49:24 slouken Exp $";
#endif

#ifndef _SDL_gp2xvideo_h
#define _SDL_gp2xvideo_h

#include "SDL_mouse.h"
#include "SDL_sysvideo.h"
#include "SDL_mutex.h"

#include "mmsp2_regs.h"

// Hidden "this" pointer for the video functions
#define _THIS	SDL_VideoDevice *this

// Allocate 8MB for the frame buffer (seems to be how gph have it)
#define GP2X_VIDEO_MEM_SIZE 5*1024*1024

// Number of native modes supported
#define SDL_NUMMODES 8


////
// Internal structure for allocating video memory
typedef struct video_bucket {
  struct video_bucket *prev, *next;
  char *base;
  unsigned int size;
  short used;
  short dirty;
} video_bucket;

////
// Private display data

typedef struct SDL_PrivateVideoData {
  int memory_fd;
  int x_offset, y_offset, ptr_offset;
  int w, h, pitch;
  int phys_width, phys_height, phys_pitch, phys_ilace;
  int scale_x, scale_y;
  int xscale, yscale;
  SDL_mutex *hw_lock;
  unsigned short fastioclk, grpclk;
  unsigned short src_foreground, src_background;
  char *vmem;
  int buffer_showing;
  char *buffer_addr[2];
  unsigned short volatile *io;
  unsigned int volatile *fio;
  video_bucket video_mem;
  char *surface_mem;
  int memory_left;
  int memory_max;
  SDL_Rect *SDL_modelist[SDL_NUMMODES+1];
  unsigned short stl_cntl, stl_mixmux, stl_alphal, stl_alphah;
  unsigned short stl_hsc, stl_vscl, stl_vsch, stl_hw;
  unsigned short stl_oadrl, stl_oadrh, stl_eadrl, stl_eadrh;
} SDL_PrivateVideoData;

extern VideoBootStrap GP2X_bootstrap;

////
// utility functions
////

////
// convert virtual address to physical
static inline unsigned int GP2X_Phys(_THIS, char *virt)
{
  return (unsigned int)((long)virt - (long)(this->hidden->vmem) + 0x3101000);
}

////
// convert virtual address to physical (lower word)
static inline unsigned short GP2X_PhysL(_THIS, char *virt)
{
  return (unsigned short)(((long)virt - (long)(this->hidden->vmem) + 0x3101000) & 0xffff);
}

////
// convert virtual address to phyical (upper word)
static inline unsigned short GP2X_PhysH(_THIS, char *virt)
{
  return (unsigned short)(((long)virt - (long)(this->hidden->vmem) + 0x3101000) >> 16);
}

////
// mark surface has been used in HW accel
static inline void GP2X_AddBusySurface(SDL_Surface *surface)
{
  ((video_bucket *)surface->hwdata)->dirty = 1;
}

////
// test if surface has been used in HW accel
static inline int GP2X_IsSurfaceBusy(SDL_Surface *surface)
{
  return ((video_bucket *)surface->hwdata)->dirty;
}

////
// wait for blitter to finish with all busy surfaces
static inline void GP2X_WaitBusySurfaces(_THIS)
{
  video_bucket *bucket;

  for (bucket = &this->hidden->video_mem; bucket; bucket = bucket->next)
    bucket->dirty = 0;
  do {} while (this->hidden->fio[MESGSTATUS] & MESG_BUSY);
}

//#define GP2X_DEBUG 1
#endif // _SDL_gp2xvideo_h
