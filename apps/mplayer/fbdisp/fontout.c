/*              
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Created. Jan, 2001
 *
 */
#include "fontdisp.h"

void gfx_hangul_disp(PSD psd, uint16 x, uint16 y, uint8 page, uint8 code)
{
   uint16 *start;
   int i;

   if( page > 0xAC ) page -= 3;

   start = psd->hfont->hanbitmap + ( (page-0xA1)*94 + (code-0xA1) )*psd->hfont->height;

   for(i=0; i<psd->hfont->height; i++) // height
   {
      put_short(start[i], psd->hfont->width);
      printf("\n");
   }

}

void put_short(unsigned short ulFont, uint16 width)
{
   int i;
   unsigned short ulMask, ulCheck;

   ulMask = 0x8000;
   for(i=0; i<width; i++)
   {
      ulCheck = ulFont & ulMask;
      if( ulCheck !=0 )
         printf("O");
      else
         printf(".");
      ulMask >>= 1;
   }
}


void gfx_han_text( PSD psd, uint16 x, uint16 y, char *text )
{
   uint8 data1, data2;
   int i;

   for(i=0; i<(int)strlen(text); i++)
   {
      data1 = *(text+i);
      if( data1 >= 0xA1 )
      {
         i++;
         data2 = *(text+i);

         gfx_hangul_disp(dc, x, y, data1, data2);
      }
   }
}


void gfx_han_putc( PSD psd, uint16 x, uint16 y, char *text )
{
   uint8 data1, data2;

   data1 = *text;
   if( data1 >= 0xA1 )
   {
      data2 = *(text+1);
      gfx_hangul_disp(dc, x, y, data1, data2);
   }
}

void gfx_printf(PSD psd, uint16 x, uint16 y, char *format, ...)
{
   va_list argptr;
   char str[200];

   va_start(argptr,format);
   vsprintf(str,format,argptr);
   gfx_mixed_text(dc, x, y, str);
   va_end(argptr);
}

void gfx_mixed_text(PSD psd, uint16 x, uint16 y, char *text)
{
   uint8 data1, data2;
   int i;

   for(i=0; i<(int)strlen(text); i++)
   {
      data1 = *(text+i);
      if( data1 >= 0xA1 )
      {
         i++;
         data2 = *(text+i);

         gfx_hangul_disp(dc, x, y, data1, data2);
      }
      else
        gfx_eng_putc(dc, x, y, text+i);
   }
}


void gfx_eng_putc( PSD psd, uint16 x, uint16 y, char *text )
{
   register int offset, bitcount, width;
   register uint32 *dp, *save_dp, font;
   int save_width;
   int height;

   offset = psd->efont->offsets[text[0]-psd->efont->startchar];
   width = psd->efont->widths[text[0]-psd->efont->startchar];
   height = psd->efont->height;

   dp = ((uint32 *)&psd->efont->bitmaps[0])+(offset>>5);

   offset = 32-(offset&0x1f);

   save_dp = dp;
   save_width = width;

   while(height--)
   {
      bitcount = offset;
      width = save_width;
      dp = save_dp;
      font = *dp++;

      while(width>0)
      {
         if( bitcount<=0 )
         {
            bitcount = 32;
            font = *dp++;
         }
         font = NFBIT_SHIFTSET(font, bitcount);

         if( bitcount > width) bitcount = width;

         while(bitcount--)
         {
            if( NFBIT_TESTBIT(font) ) printf("O");
            else                      printf(".");
            font = NFBIT_LEFTSHIFT(font);
            width --;
         }
      }
      save_dp += (psd->efont->stride); // plus amount of stride
      printf("\n");
   } // while

}

void gfx_eng_text( PSD psd, uint16 x, uint16 y, char *text )
{
   register uint32 *gp;
   register uint32 font;
   register int offset;
   register int width;
   uint32 *bitmap   = (uint32 *)&psd->efont->bitmaps[0];
   uint16 height    = psd->efont->height;
   uint8  *cp;

   while(height--)
   {
      for( cp = (uint8 *)text; (offset = *cp)!=0; cp++)
      {
         offset -= psd->efont->startchar;
         width  = psd->efont->widths[offset];
         offset = psd->efont->offsets[offset];
         gp = bitmap+(offset>>5);
         offset = 32-(offset&0x1f); // bitcount = 32-(offset&0x1f)
         font = *gp++;

         while(width>0)
         {
            if( offset<=0 )
            {
               offset = 32;
               font = *gp++;
            }
            font = NFBIT_SHIFTSET(font, offset);

            if( offset > width) offset = width;

            while(offset--)
            {
               if( NFBIT_TESTBIT(font) ) printf("O");
               else                      printf(".");
               font = NFBIT_LEFTSHIFT(font);
               width--;
            }
         }
      }// for(text)
      bitmap += (psd->efont->stride);
      printf("\n");
   }// while(height--)

}

int gfx_htext_width(PSD psd, char *text)
{
   int i, text_length_pixel=0;
   uint8 data1;

   for(i=0; i<(int)strlen(text); i++)
   {
      data1 = *(text+i);
      if( data1 >= 0xA1 )
      {
         i++;
         text_length_pixel += psd->hfont->width;
      }
   }

   return text_length_pixel;
}

int gfx_echar_width(PSD psd, char ch)
{
   int char_position =  ch - psd->efont->startchar;

   if( char_position < 0 ) return -1;

   return psd->efont->widths[char_position];
}


int gfx_etext_width(PSD psd, char *text)
{
   int j;
   int char_position, text_length_pixel = 0;

   for(j=0; j<(int)strlen(text); j++)
   {
      char_position = text[j]-psd->efont->startchar;
      text_length_pixel += psd->efont->widths[char_position];
   }
   return text_length_pixel;
}

int gfx_mtext_width(PSD psd, char *text)
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
         text_length_pixel += psd->hfont->width;
      }
      else
      {
         char_position = text[i]-psd->efont->startchar;
         text_length_pixel += psd->efont->widths[char_position];
      }
   }

   return text_length_pixel;
}

