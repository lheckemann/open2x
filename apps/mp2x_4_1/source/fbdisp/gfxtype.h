/*
 * Copyright (c) 1999, 2000 Greg Haerr <greg@censoft.com>
 *
 * Engine-level Screen, Mouse and Keyboard device driver API's and types
 * 
 * Contents of this file are not for general export
 *
 *  - godori <ghcstop>, www.aesop-embedded.org
 *    => Modified. 2002. 02. 11
 */


#ifndef _GFXTYPE_H
#define _GFXTYPE_H



typedef char              int8;
typedef short             int16;
typedef int               int32;
typedef unsigned char     uint8;
typedef unsigned short    uint16;
typedef unsigned int      uint32;


typedef int             MWCOORD;     // device coordinates 
typedef int             MWBOOL;      // boolean value 
typedef unsigned char   MWUCHAR;     // unsigned char 
typedef unsigned long   MWCOLORVAL;  // device-independent color value 


typedef unsigned char  *ADDR8;
typedef unsigned short *ADDR16;
typedef unsigned long  *ADDR32;


#define MWPF_TRUECOLOR0888 3   // pixel is packed 32 bits 8/8/8 truecolor
#define MWPF_TRUECOLOR888  4   // pixel is packed 24 bits 8/8/8 truecolor
#define MWPF_TRUECOLOR565  5   // pixel is packed 16 bits 5/6/5 truecolor
#define MWPF_TRUECOLOR332  6   // pixel is packed 8 bits 3/3/2 truecolor





#define MWPIXEL_FORMAT   MWPF_TRUECOLOR565
#endif
