/***************************************************************************
 *   SDL_rwz.c -- SDL_RWops zlib compression/decompression stream          *
 *     implimentation                                                      *
 *                                                                         *
 *   Copyright (C) 2004 by Tyler Montbriand                                *
 *   tsm@accesscomm.ca                                                     *
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
#include <zlib.h>
#include "SDL.h"
#include "SDL_rwz.h"
#include "SDL_rwlib_internal.h"

#define PKGNAME GZ

RWFUNC_SEEK(PKGNAME);
RWFUNC_READ(PKGNAME);
RWFUNC_WRITE(PKGNAME);
RWFUNC_CLOSE(PKGNAME);

SDL_RWops *SDL_RWFromGZ(const char *file, const char *mode)
{
  SDL_RWops *rw=SDL_AllocRW();
  gzFile f;

  if(rw==NULL) return(NULL);

  f=gzopen(file,mode);
  if(f==NULL)
  {
    SDL_SetError("Could not open/create gz");
    SDL_FreeRW(rw);
    return(NULL);
  }

  RW_INITFUNC(rw,PKGNAME);
  RW_DATAPTR1(rw)=(void *)f;

  return(rw);
}

//static int rw_gz_seek (SDL_RWops *ctx, int offset, int whence)
RWFUNC_SEEK(PKGNAME)
{
  return(gzseek((gzFile)RW_DATAPTR1(ctx),offset,whence));
}

//static int rw_gz_read (SDL_RWops *ctx, void *ptr, int size, int maxnum)
RWFUNC_READ(PKGNAME)
{
  int bytes=gzread((gzFile)RW_DATAPTR1(ctx),ptr,size*maxnum);

  if(bytes%size)
    gzseek((gzFile)RW_DATAPTR1(ctx),-(bytes%size),SEEK_CUR);

  return(bytes/size);
}

//static int rw_gz_write(SDL_RWops *ctx, const void *ptr, int size, int maxnum)
RWFUNC_WRITE(PKGNAME)
{
  int bytes=gzwrite((gzFile)RW_DATAPTR1(ctx),ptr,size*maxnum);

  if(bytes%size)
    gzseek((gzFile)RW_DATAPTR1(ctx),-(bytes%size),SEEK_CUR);

  return(bytes/size);
}

//static int rw_gz_close(SDL_RWops *c)
RWFUNC_CLOSE(PKGNAME)
{
  if(RW_TYPE(ctx)!=RW_TYPENUM(GZ))
  {
    SDL_SetError("rw_gz_close:  SDL_RWops is the wrong type!");
    return(-1);
  }

  gzclose((gzFile)RW_DATAPTR1(ctx));
  ctx->hidden.unknown.data1=NULL;
  SDL_FreeRW(ctx);
  return(0);
}
