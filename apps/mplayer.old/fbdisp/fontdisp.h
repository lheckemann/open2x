/*              
 *  All right reserved.
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Created. 2001.02.11
 */

#ifndef __FONTDISP_H
#define __FONTDISP_H

#include <stdio.h>
#include <stdlib.h>
//#include <mem.h>
#include <string.h>
#include <stdarg.h>

#include "gfxtype.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef struct {
    char    *hanname;
    uint16  *hanbitmap;
    uint16  numchars;
    uint16  width;
    uint16  height;
} Hanfont;

typedef struct{
    uint16 *bitmap;         
    uint8  *widths;         
    uint8   imagebytes;     
    uint8   number_of_char; 
    uint8   firstchar;      
    uint8   lastchar;
    uint8   startchar;
    uint8   height;
} Engfont;




#endif

