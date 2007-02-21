/*
 *  - godori <ghcstop>, www.aesop-embedded.org
 *
 *    => Created. Jan, 2001
 */

#include "gfxdev.h"


#ifndef OUTLINE_BITMAP
void gfx_hangul_disp(PSD psd, uint16 x, uint16 y, uint8 page, uint8 code)
{
   uint16 *start;
   int i, j;
   uint16 pixel_image, localx = x;
   uint16 us_mask = 0x8000, us_check;
   uint16 fgcolor = psd->fg;
   uint16 bgcolor = psd->bg;

   if( page > 0xC8 )
   {
        page = 0xA3;
        code = 0xBF;
   }

   if( page > 0xAC ) page -= 3;


   start = psd->hfont->hanbitmap + ( (page-0xA1)*94 + (code-0xA1) )*psd->hfont->height;


   for(i=0; i<psd->hfont->height; i++)
   {
      pixel_image = start[i];
      for( j=0; j<psd->hfont->width; j++)
      {
         us_check = pixel_image & us_mask;
         if( us_check )
            gfx_draw_pixel(psd, localx+j, y, fgcolor);
         #ifdef USE_DISP_BG_COLOR
         else
            gfx_draw_pixel(psd, localx+j, y, bgcolor);
         #endif

         us_mask >>= 1;
      }
      us_mask = 0x8000;
      y++;
      localx = x;
   }

}

#else

unsigned int orig[32+2];
unsigned int out[32+2];
unsigned int tmp[32+2];
static uint32 tmpv;

void disp_hfont(PSD psd, uint16 x, uint16 y, uint32 *data, uint16 color)
{
   int i, j;
   uint16 localx = x;
   uint16 localy = y;

   uint32 mask = 0x80000000, check, pixel_image;


   for(i=0; i<psd->hfont->height+2; i++)
   {
      pixel_image = data[i];
      for( j=0; j<psd->hfont->width+2; j++)
      {
         check = pixel_image & mask;
         if( check )
            gfx_draw_pixel(psd, localx+j, localy, color);
         #ifdef USE_DISP_BG_COLOR
         else
            gfx_draw_pixel(psd, localx+j, localy, WHITE);
         #endif

         mask >>= 1;
      }
      mask = 0x80000000;
      localy++;
      localx = x;
   }

}

void gfx_hangul_disp(PSD psd, uint16 x, uint16 y, uint8 page, uint8 code)
{
    uint16 *start;
    int i, j;
    uint16 localx = x;
    uint16 localy = y;
    uint32 mask = 0x80000000, check, pixel_image;



    if( page > 0xC8 )
    {

        page = 0xA3;
        code = 0xBF;
    }

    if( page > 0xAC ) page -= 3;



    start = psd->hfont->hanbitmap + ( (page-0xA1)*94 + (code-0xA1) )*psd->hfont->height;







    for(i=0; i<psd->hfont->height; i++)
    {
        tmpv = (uint32)start[i];




        orig[i+1] = tmpv<<15;
    }
    orig[0]=0;
    orig[i+1]=0;




    for(i=0; i<psd->hfont->height+2; i++)
    {
        if( i <= psd->hfont->height-2 )
        {
            tmp[i]  = orig[i+1]<<1;
        }
        else
            tmp[i]  = 0;

        out[i] = tmp[i]|orig[i];
    }




    for(i=0; i<psd->hfont->height+2; i++)
    {
        if( i <= psd->hfont->height-2 )
        {
            tmp[i]  = orig[i+1]>>1;
        }
        else
            tmp[i]  = 0;

        out[i] |= tmp[i];
    }




    for(i=0; i<psd->hfont->height+2; i++)
    {
        if( i == 0 )
        {
            tmp[i]  = 0;
        }
        else
            tmp[i]  = orig[i-1]<<1;

        out[i]  |= tmp[i];
    }




    for(i=0; i<psd->hfont->height+2; i++)
    {
        if( i == 0 )
        {
            tmp[i]  = 0;
        }
        else
            tmp[i]  = orig[i-1]>>1;

        out[i]  |= tmp[i];
    }




    for(i=0; i<psd->hfont->height+2; i++)
    {
        tmp[i]  = orig[i]<<1;
        out[i] |= tmp[i];
    }




    for(i=0; i<psd->hfont->height+2; i++)
    {
        tmp[i]  = orig[i]>>1;
        out[i] |= tmp[i];
    }





    for(i=0; i<psd->hfont->height+2; i++)
    {
        if( i <= psd->hfont->height-2 )
        {
            tmp[i]  = orig[i+1];
        }
        else
            tmp[i]  = 0;

        out[i] |= tmp[i];
    }





    for(i=0; i<psd->hfont->height+2; i++)
    {
        if( i == 0 )
        {
            tmp[i]  = 0;
        }
        else
            tmp[i]  = orig[i-1];

        out[i]  |= tmp[i];
    }




#if 1
   for(i=0; i<psd->hfont->height+2; i++)
   {
      pixel_image = out[i];
      for( j=0; j<psd->hfont->width+2; j++)
      {
         check = pixel_image & mask;
         if( check )
            gfx_draw_pixel(psd, localx+j, localy, psd->fg);
         #ifdef USE_DISP_BG_COLOR
         else
            gfx_draw_pixel(psd, localx+j, localy, psd->bg);
         #endif

         mask >>= 1;
      }
      mask = 0x80000000;
      localy++;
      localx = x;
   }





   mask = 0x80000000;
   localx = x;
   localy = y;

   for(i=0; i<psd->hfont->height+2; i++)
   {
      pixel_image = orig[i];
      for( j=0; j<psd->hfont->width+2; j++)
      {
         check = pixel_image & mask;
         if( check )
            gfx_draw_pixel(psd, localx+j, localy, psd->fcolor);
         #ifdef USE_DISP_BG_COLOR
         else
            gfx_draw_pixel(psd, localx+j, localy, psd->bg);
         #endif

         mask >>= 1;
      }
      mask = 0x80000000;
      localy++;
      localx = x;
   }
#endif


}
#endif






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

         gfx_hangul_disp(psd, x, y, data1, data2);
         #ifdef OUTLINE_BITMAP
         x += (psd->hfont->width+2);
         #else
         x += psd->hfont->width;
         #endif
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

      gfx_hangul_disp(psd, x, y, data1, data2);
   }
}


void gfx_printf(PSD psd, uint16 x, uint16 y, char *format, ...)
{
   va_list argptr;
   char str[200];

   va_start(argptr,format);
   vsprintf(str,format,argptr);
   gfx_draw_mtext(psd, x, y, str);
   va_end(argptr);
}

void gfx_mixed_text(PSD psd, uint16 x, uint16 y, char *text)
{
   uint8 data1, data2;
   uint8 cho, jung, jong;
   int i;

   for(i=0; i<(int)strlen(text); i++)
   {
      data1 = *(text+i);
      if( data1 >= 0xA1 )
      {
         i++;
         data2 = *(text+i);


         gfx_hangul_disp(psd, x, y, data1, data2);
         #ifdef OUTLINE_BITMAP
         x += (psd->hfont->width+2);
         #else
         x += psd->hfont->width;
         #endif
      }
      else
      {
         gfx_draw_eputc(psd, x, y, text+i);
         #ifdef OUTLINE_BITMAP
         x += (psd->efont->widths[text[i]-psd->efont->startchar]+2);
         #else
         x += psd->efont->widths[text[i]-psd->efont->startchar];
         #endif
      }
   }
}




#ifndef OUTLINE_BITMAP

void gfx_eng_text( PSD psd, uint16 x, uint16 y, char *text )
{
   int i, j, k;
   int width=0;
   int char_position;
   uint16 output = 0x0000;
   uint16 mask = 0x8000, localx = x;

   for(i=0; i<psd->efont->height; i++)
   {
      for(j=0; j<(int)strlen(text); j++)
      {
         char_position = text[j]-psd->efont->startchar;
         width = psd->efont->widths[char_position];
         for(k=0; k<width; k++)
         {
            output = (uint16)( psd->efont->bitmap[char_position*psd->efont->height+i] & mask);
            if(output != 0 )
               gfx_draw_pixel(psd, localx++, y, psd->fg);
            #ifdef USE_DISP_BG_COLOR
            else
               gfx_draw_pixel(psd, localx++, y, psd->bg);
            #else
            else                      localx++;
            #endif
            mask >>= 1;
         }
         mask = 0x8000;
      }
      localx = x;
      y++;
   }
}

void gfx_eng_putc( PSD psd, uint16 x, uint16 y, char *text )
{
   int i, j;
   int width=0;
   int char_position;
   uint16 output = 0x0000;
   uint16 mask = 0x8000, localx = x;

   for(i=0; i<psd->efont->height; i++)
   {
      char_position = text[0]-psd->efont->startchar;
      width = psd->efont->widths[char_position];
      for(j=0; j<width; j++)
      {
         output = (uint16)( psd->efont->bitmap[char_position*psd->efont->height+i] & mask);
         if(output != 0 )
            gfx_draw_pixel(psd, localx++, y, psd->fg);
         #ifdef USE_DISP_BG_COLOR
         else
            gfx_draw_pixel(psd, localx++, y, psd->bg);
         #else
         else                      localx++;
         #endif

         mask >>= 1;
      }
      mask = 0x8000;
      localx = x;
      y++;
   }
}
#else

void gfx_eng_text( PSD psd, uint16 x, uint16 y, char *text )
{
   uint8 data1, data2;
   int i;

   for(i=0; i<(int)strlen(text); i++)
   {
      gfx_draw_eputc(psd, x, y, text+i);
      #ifdef OUTLINE_BITMAP
      x += (psd->efont->widths[text[i]-psd->efont->startchar]+2);
      #else
      x += psd->efont->widths[text[i]-psd->efont->startchar];
      #endif
   }
}



uint32 eorig[32+2];
uint32 eout[32+2];
uint32 etmp[32+2];


void disp_efont(PSD psd, int width, uint16 x, uint16 y, uint32 *data, uint16 color)
{
    int i, j;
    uint32 output = 0x00000000;
    uint32 mask = 0x80000000;
    uint16 localx = x;
    uint16 localy = y;

   for(i=0; i<psd->efont->height+2; i++)
   {
      for(j=0; j<width+2; j++)
      {
         output = ( eorig[i] & mask);
         if( output )
            gfx_draw_pixel(psd, localx++, localy, color);
         #ifdef USE_DISP_BG_COLOR
         else
            gfx_draw_pixel(psd, localx++, localy, white);
         #else
         else                      localx++;
         #endif

         mask >>= 1;
      }
      mask = 0x80000000;
      localx = x;
      localy++;
   }
}


void gfx_eng_putc( PSD psd, uint16 x, uint16 y, char *text )
{
    int i, j;
    int width;
    int char_position;
    uint32 output = 0x00000000;
    uint32 mask = 0x80000000;
    uint16 localx = x;
    uint16 localy = y;

    char_position = text[0]-psd->efont->startchar;
    width = psd->efont->widths[char_position];


    for(i=0; i<psd->efont->height; i++)
    {
        tmpv = (uint32)psd->efont->bitmap[char_position*psd->efont->height+i];
        eorig[i+1] = (tmpv<<15);



    }


    eorig[0]=0;
    eorig[i+1]=0;





    for(i=0; i<psd->efont->height+2; i++)
    {
        if( i <= psd->efont->height-2 )
        {
            etmp[i]  = eorig[i+1]<<1;
        }
        else
            etmp[i]  = 0;

        eout[i] = etmp[i]|eorig[i];
    }




    for(i=0; i<psd->efont->height+2; i++)
    {
        if( i <= psd->efont->height-2 )
        {
            etmp[i]  = eorig[i+1]>>1;
        }
        else
            etmp[i]  = 0;

        eout[i] |= etmp[i];
    }




    for(i=0; i<psd->efont->height+2; i++)
    {
        if( i == 0 )
        {
            etmp[i]  = 0;
        }
        else
            etmp[i]  = eorig[i-1]<<1;

        eout[i]  |= etmp[i];
    }




    for(i=0; i<psd->efont->height+2; i++)
    {
        if( i == 0 )
        {
            etmp[i]  = 0;
        }
        else
            etmp[i]  = eorig[i-1]>>1;

        eout[i]  |= etmp[i];
    }




    for(i=0; i<psd->efont->height+2; i++)
    {
        etmp[i]  = eorig[i]<<1;
        eout[i] |= etmp[i];
    }




    for(i=0; i<psd->efont->height+2; i++)
    {
        etmp[i]  = eorig[i]>>1;
        eout[i] |= etmp[i];
    }





    for(i=0; i<psd->efont->height+2; i++)
    {
        if( i <= psd->efont->height-2 )
        {
            etmp[i]  = eorig[i+1];
        }
        else
            etmp[i]  = 0;

        eout[i] |= etmp[i];
    }







    for(i=0; i<psd->efont->height+2; i++)
    {
        if( i == 0 )
        {
            etmp[i]  = 0;
        }
        else
            etmp[i]  = eorig[i-1];

        eout[i]  |= etmp[i];
    }


   for(i=0; i<psd->efont->height+2; i++)
   {
      for(j=0; j<width+2; j++)
      {
         output = ( eout[i] & mask);
         if( output )
            gfx_draw_pixel(psd, localx++, localy, psd->fg);
         #ifdef USE_DISP_BG_COLOR
         else
            gfx_draw_pixel(psd, localx++, localy, psd->bg);
         #else
         else                      localx++;
         #endif

         mask >>= 1;
      }
      mask = 0x80000000;
      localx = x;
      localy++;
   }



   mask = 0x80000000;
   localx = x;
   localy = y;

   for(i=0; i<psd->efont->height+2; i++)
   {
      for(j=0; j<width+2; j++)
      {
         output = ( eorig[i] & mask);
         if( output )
            gfx_draw_pixel(psd, localx++, localy, psd->fcolor);
         #ifdef USE_DISP_BG_COLOR
         else
            gfx_draw_pixel(psd, localx++, localy, psd->bg);
         #else
         else                      localx++;
         #endif

         mask >>= 1;
      }
      mask = 0x80000000;
      localx = x;
      localy++;
   }



}

#endif
