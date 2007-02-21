/*
 *  - godori <ghcstop>, www.aesop-embedded.org
 *
 *    => Created. Jan, 2001
 *
 */
#include "gfxdev.h"

inline int gfx_htext_width(PSD psd, char *text)
{
   int i, text_length_pixel=0;
   uint8 data1;

   for(i=0; i<(int)strlen(text); i++)
   {
      data1 = *(text+i);
      if( data1 >= 0xA1 )
      {
         i++;
         #ifdef OUTLINE_BITMAP
         text_length_pixel += (psd->hfont->width+2);
         #else
         text_length_pixel += psd->hfont->width;
         #endif
      }
   }

   return text_length_pixel;
}

inline int gfx_hchar_width(PSD psd, char *text)
{
    #ifdef OUTLINE_BITMAP
    return (psd->hfont->width+2);
    #else
    return psd->hfont->width;
    #endif
}


inline int gfx_echar_width(PSD psd, char ch)
{
   int char_position =  ch - psd->efont->startchar;

   if( char_position < 0 ) return -1;

   #ifdef OUTLINE_BITMAP
   return (psd->efont->widths[char_position]+2);
   #else
   return psd->efont->widths[char_position];
   #endif
}


inline int gfx_etext_width(PSD psd, char *text)
{
   int j;
   int char_position, text_length_pixel = 0;

   for(j=0; j<(int)strlen(text); j++)
   {
      char_position = text[j]-psd->efont->startchar;
      #ifdef OUTLINE_BITMAP
      text_length_pixel += (psd->efont->widths[char_position]+2);
      #else
      text_length_pixel += psd->efont->widths[char_position];
      #endif
   }
   return text_length_pixel;
}

inline int gfx_mtext_width(PSD psd, char *text)
{
   int i, text_length_pixel=0;
   uint8 data1;
   int char_position;


   for(i=0; i<(int)strlen(text); i++)
   {
      data1 = *(text+i);
      if( data1 >= 0xA1 )
      {
         i++;
         #ifdef OUTLINE_BITMAP
         text_length_pixel += (psd->hfont->width+2);
         #else
         text_length_pixel += psd->hfont->width;
         #endif
      }
      else
      {
         char_position = text[i]-psd->efont->startchar;
         #ifdef OUTLINE_BITMAP
         text_length_pixel += (psd->efont->widths[char_position]+2);
         #else
         text_length_pixel += psd->efont->widths[char_position];
         #endif
      }
   }

   return text_length_pixel;
}


inline int gfx_han_height(PSD psd)
{
    #ifdef OUTLINE_BITMAP
    return (psd->hfont->height+2);
    #else
    return psd->hfont->height;
    #endif
}

inline int gfx_eng_height(PSD psd)
{
    #ifdef OUTLINE_BITMAP
    return (psd->efont->height+2);
    #else
    return psd->efont->height;
    #endif
}
