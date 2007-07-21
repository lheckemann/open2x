/*
 * gp2x_yv12toyuy2.c
 *
 * Copyright (C) 2007	John Willis <djwillis@open2x.org>
 *
 * Supports conversion from YV12 > YUY2 to match the GP2X hardware.
 *
 * This file is part of the GP2X (Open2x) fork of mpeg2dec
 *
 * mpeg2dec-gp2x is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec-gp2x is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Color Conversion Utility Functions
 *
 * Overview: xine's video output modules only accept YUV images from
 * video decoder modules. A video decoder can either send a planar (YV12)
 * image or a packed (YUY2) image to a video output module. However, many
 * older video codecs are RGB-based. Either each pixel is an index
 * to an RGB value in a palette table, or each pixel is encoded with
 * red, green, and blue values. In the latter case, typically either
 * 15, 16, 24, or 32 bits are used to represent a single pixel.
 * The facilities in this file are designed to ease the pain of converting
 * RGB -> YUV.
 *
 * If you want to use these facilities in your decoder, include the
 * xineutils.h header file. Then declare a yuv_planes_t structure. This
 * structure represents 3 non-subsampled YUV planes. "Non-subsampled"
 * means that there is a Y, U, and V sample for each pixel in the RGB
 * image, whereas YUV formats are usually subsampled so that the U and
 * V samples correspond to more than 1 pixel in the output image. When
 * you need to convert RGB values to Y, U, and V, values, use the
 * COMPUTE_Y(r, g, b), COMPUTE_U(r, g, b), COMPUTE_V(r, g, b) macros found
 * in xineutils.h
 *
 * The yuv_planes_t structure has 2 other fields: row_width and row_count
 * which are equivalent to the frame width and height, respectively.
 *
 * When an image has been fully decoded into the yuv_planes_t structure,
 * call yuv444_to_yuy2() with the structure and the final (pre-allocated)
 * YUY2 buffer. xine will have already chosen the best conversion
 * function to use based on the CPU type. The YUY2 buffer will then be
 * ready to pass to the video output module.
 *
 * If your decoder is rendering an image based on an RGB palette, a good
 * strategy is to maintain a YUV palette rather than an RGB palette and
 * render the image directly in YUV.
 *
 * Some utility macros that you may find useful in your decoder are
 * UNPACK_RGB15, UNPACK_RGB16, UNPACK_BGR15, and UNPACK_BGR16. All are
 * located in xineutils.h. All of them take a packed pixel, either in
 * RGB or BGR format depending on the macro, and unpack them into the
 * component red, green, and blue bytes. If a CPU has special instructions
 * to facilitate these operations (such as the PPC AltiVec pixel-unpacking
 * instructions), these macros will automatically map to those special
 * instructions.
 *
 * $Id: color.c,v 1.1 2005/04/04 22:38:34 dsalt-guest Exp $
 */

#include "xine_internal.h"

/*
 * In search of the perfect colorspace conversion formulae...
 * These are the conversion equations that xine currently uses:
 *
 *      Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
 *      U  = -0.16874 * R - 0.33126 * G + 0.50000 * B + 128
 *      V  =  0.50000 * R - 0.41869 * G - 0.08131 * B + 128
 *
 * Feel free to experiment with different coefficients by altering the
 * next 9 defines.
 */

#if 1

#define Y_R (SCALEFACTOR *  0.29900)
#define Y_G (SCALEFACTOR *  0.58700)
#define Y_B (SCALEFACTOR *  0.11400)

#define U_R (SCALEFACTOR * -0.16874)
#define U_G (SCALEFACTOR * -0.33126)
#define U_B (SCALEFACTOR *  0.50000)

#define V_R (SCALEFACTOR *  0.50000)
#define V_G (SCALEFACTOR * -0.41869)
#define V_B (SCALEFACTOR * -0.08131)

#else

/*
 * Here is another promising set of coefficients. If you use these, you
 * must also add 16 to the Y calculation in the COMPUTE_Y macro found
 * in xineutils.h.
 */

#define Y_R (SCALEFACTOR *  0.257)
#define Y_G (SCALEFACTOR *  0.504)
#define Y_B (SCALEFACTOR *  0.098)

#define U_R (SCALEFACTOR * -0.148)
#define U_G (SCALEFACTOR * -0.291)
#define U_B (SCALEFACTOR *  0.439)

#define V_R (SCALEFACTOR *  0.439)
#define V_G (SCALEFACTOR * -0.368)
#define V_B (SCALEFACTOR * -0.071)

#endif

/*
 * Precalculate all of the YUV tables since it requires fewer than
 * 10 kilobytes to store them.
 */
int y_r_table[256];
int y_g_table[256];
int y_b_table[256];

int u_r_table[256];
int u_g_table[256];
int u_b_table[256];

int v_r_table[256];
int v_g_table[256];
int v_b_table[256];

void (*yv12_to_yuy2)
  (unsigned char *y_src, int y_src_pitch,
   unsigned char *u_src, int u_src_pitch,
   unsigned char *v_src, int v_src_pitch,
   unsigned char *yuy2_map, int yuy2_pitch,
   int width, int height, int progressive);

/*
 * init_yuv_planes
 *
 * This function initializes a yuv_planes_t structure based on the width
 * and height passed to it. The width must be divisible by 2.
 */
void init_yuv_planes(yuv_planes_t *yuv_planes, int width, int height) {

  int plane_size;

  yuv_planes->row_width = width;
  yuv_planes->row_count = height;
  plane_size = yuv_planes->row_width * yuv_planes->row_count;

  yuv_planes->y = xine_xmalloc(plane_size);
  yuv_planes->u = xine_xmalloc(plane_size);
  yuv_planes->v = xine_xmalloc(plane_size);
}

/*
 * free_yuv_planes
 *
 * This frees the memory used by the YUV planes.
 */
void free_yuv_planes(yuv_planes_t *yuv_planes) {
  free(yuv_planes->y);
  free(yuv_planes->u);
  free(yuv_planes->v);
}

static void hscale_chroma_line (unsigned char *dst, unsigned char *src,
  int width) {

  unsigned int n1, n2;
  int       x;

  n1       = *src;
  *(dst++) = n1;

  for (x=0; x < (width - 1); x++) {
    n2       = *(++src);
    *(dst++) = (3*n1 + n2 + 2) >> 2;
    *(dst++) = (n1 + 3*n2 + 2) >> 2;
    n1       = n2;
  }

  *dst = n1;
}

static void vscale_chroma_line (unsigned char *dst, int pitch,
  unsigned char *src1, unsigned char *src2, int width) {

  unsigned int t1, t2;
  unsigned int n1, n2, n3, n4;
  unsigned int *dst1, *dst2;
  int       x;

  dst1 = (unsigned int *) dst;
  dst2 = (unsigned int *) (dst + pitch);

  /* process blocks of 4 pixels */
  for (x=0; x < (width / 4); x++) {
    n1  = *(((unsigned int *) src1)++);
    n2  = *(((unsigned int *) src2)++);
    n3  = (n1 & 0xFF00FF00) >> 8;
    n4  = (n2 & 0xFF00FF00) >> 8;
    n1 &= 0x00FF00FF;
    n2 &= 0x00FF00FF;

    t1 = (2*n1 + 2*n2 + 0x20002);
    t2 = (n1 - n2);
    n1 = (t1 + t2);
    n2 = (t1 - t2);
    t1 = (2*n3 + 2*n4 + 0x20002);
    t2 = (n3 - n4);
    n3 = (t1 + t2);
    n4 = (t1 - t2);

    *(dst1++) = ((n1 >> 2) & 0x00FF00FF) | ((n3 << 6) & 0xFF00FF00);
    *(dst2++) = ((n2 >> 2) & 0x00FF00FF) | ((n4 << 6) & 0xFF00FF00);
  }

  /* process remaining pixels */
  for (x=(width & ~0x3); x < width; x++) {
    n1 = src1[x];
    n2 = src2[x];

    dst[x]       = (3*n1 + n2 + 2) >> 2;
    dst[x+pitch] = (n1 + 3*n2 + 2) >> 2;
  }
}

static void upsample_c_plane_c(unsigned char *src, int src_width,
  int src_height, unsigned char *dest,
  unsigned int src_pitch, unsigned int dest_pitch) {

  unsigned char *cr1;
  unsigned char *cr2;
  unsigned char *tmp;
  int y;

  cr1 = &dest[dest_pitch * (src_height * 2 - 2)];
  cr2 = &dest[dest_pitch * (src_height * 2 - 3)];

  /* horizontally upscale first line */
  hscale_chroma_line (cr1, src, src_width);
  src += src_pitch;

  /* store first line */
  memcpy (dest, cr1, src_width * 2);
  dest += dest_pitch;

  for (y = 0; y < (src_height - 1); y++) {

    hscale_chroma_line (cr2, src, src_width);
    src += src_pitch;

    /* interpolate and store two lines */
    vscale_chroma_line (dest, dest_pitch, cr1, cr2, src_width * 2);
    dest += 2 * dest_pitch;

    /* swap buffers */
    tmp = cr2;
    cr2 = cr1;
    cr1 = tmp;
  }

  /* horizontally upscale and store last line */
  src -= src_pitch;
  hscale_chroma_line (dest, src, src_width);
}

/*
 * yuv9_to_yv12_c
 *
 */
static void yuv9_to_yv12_c
  (unsigned char *y_src, int y_src_pitch, unsigned char *y_dest, int y_dest_pitch,
   unsigned char *u_src, int u_src_pitch, unsigned char *u_dest, int u_dest_pitch,
   unsigned char *v_src, int v_src_pitch, unsigned char *v_dest, int v_dest_pitch,
   int width, int height) {

  int y;

  /* Y plane */
  for (y=0; y < height; y++) {
    xine_fast_memcpy (y_dest, y_src, width);
    y_src += y_src_pitch;
    y_dest += y_dest_pitch;
  }

  /* U plane */
  upsample_c_plane_c(u_src, width / 4, height / 4, u_dest,
    u_src_pitch, u_dest_pitch);

  /* V plane */
  upsample_c_plane_c(v_src, width / 4, height / 4, v_dest,
    v_src_pitch, v_dest_pitch);

}

/*****************************************************************************
 * I420_YUY2: planar YUV 4:2:0 to packed YUYV 4:2:2
 * original conversion routine from Videolan project
 * changed to support interlaced frames and use simple mean interpolation [MF]
 *****************************************************************************/
static void yv12_to_yuy2_c
  (unsigned char *y_src, int y_src_pitch,
   unsigned char *u_src, int u_src_pitch,
   unsigned char *v_src, int v_src_pitch,
   unsigned char *yuy2_map, int yuy2_pitch,
   int width, int height, int progressive) {

    uint8_t *p_line1, *p_line2 = yuy2_map;
    uint8_t *p_y1, *p_y2 = y_src;
    uint8_t *p_u = u_src;
    uint8_t *p_v = v_src;
    uint8_t *p_u2 = u_src + u_src_pitch;
    uint8_t *p_v2 = v_src + v_src_pitch;

    int i_x, i_y;

    const int i_source_margin = y_src_pitch - width;
    const int i_source_u_margin = u_src_pitch - width/2;
    const int i_source_v_margin = v_src_pitch - width/2;
    const int i_dest_margin = yuy2_pitch - width*2;


    if( progressive ) {

      for( i_y = height / 2 ; i_y-- ; )
      {
          p_line1 = p_line2;
          p_line2 += yuy2_pitch;

          p_y1 = p_y2;
          p_y2 += y_src_pitch;

          for( i_x = width / 8 ; i_x-- ; )
          {
              C_YUV420_YUYV( );
              C_YUV420_YUYV( );
              C_YUV420_YUYV( );
              C_YUV420_YUYV( );
          }

          p_y2 += i_source_margin;
          p_u += i_source_u_margin;
          p_v += i_source_v_margin;
          if( i_y > 1 ) {
            p_u2 += i_source_u_margin;
            p_v2 += i_source_v_margin;
          } else {
            p_u2 = p_u;
            p_v2 = p_v;
          }
          p_line2 += i_dest_margin;
      }

    } else {

      p_u2 = u_src + 2*u_src_pitch;
      p_v2 = v_src + 2*v_src_pitch;
      for( i_y = height / 4 ; i_y-- ; )
      {
          p_line1 = p_line2;
          p_line2 += 2 * yuy2_pitch;

          p_y1 = p_y2;
          p_y2 += 2 * y_src_pitch;

          for( i_x = width / 8 ; i_x-- ; )
          {
              C_YUV420_YUYV( );
              C_YUV420_YUYV( );
              C_YUV420_YUYV( );
              C_YUV420_YUYV( );
          }

          p_y2 += i_source_margin + y_src_pitch;
          p_u += i_source_u_margin + u_src_pitch;
          p_v += i_source_v_margin + v_src_pitch;
          if( i_y > 1 ) {
            p_u2 += i_source_u_margin + u_src_pitch;
            p_v2 += i_source_v_margin + v_src_pitch;
          } else {
            p_u2 = p_u;
            p_v2 = p_v;
          }
          p_line2 += i_dest_margin + yuy2_pitch;
      }

      p_line2 = yuy2_map + yuy2_pitch;
      p_y2 = y_src + y_src_pitch;
      p_u = u_src + u_src_pitch;
      p_v = v_src + v_src_pitch;
      p_u2 = u_src + 3*u_src_pitch;
      p_v2 = v_src + 3*v_src_pitch;

      for( i_y = height / 4 ; i_y-- ; )
      {
          p_line1 = p_line2;
          p_line2 += 2 * yuy2_pitch;

          p_y1 = p_y2;
          p_y2 += 2 * y_src_pitch;

          for( i_x = width / 8 ; i_x-- ; )
          {
              C_YUV420_YUYV( );
              C_YUV420_YUYV( );
              C_YUV420_YUYV( );
              C_YUV420_YUYV( );
          }

          p_y2 += i_source_margin + y_src_pitch;
          p_u += i_source_u_margin + u_src_pitch;
          p_v += i_source_v_margin + v_src_pitch;
          if( i_y > 1 ) {
            p_u2 += i_source_u_margin + u_src_pitch;
            p_v2 += i_source_v_margin + v_src_pitch;
          } else {
            p_u2 = p_u;
            p_v2 = p_v;
          }
          p_line2 += i_dest_margin + yuy2_pitch;
      }

    }
}


#define C_YUYV_YUV420( )                                          \
    *p_y1++ = *p_line1++; *p_y2++ = *p_line2++;                   \
    *p_u++ = (*p_line1++ + *p_line2++)>>1;                        \
    *p_y1++ = *p_line1++; *p_y2++ = *p_line2++;                   \
    *p_v++ = (*p_line1++ + *p_line2++)>>1;

/*
 * init_yuv_conversion
 *
 * This function precalculates all of the tables used for converting RGB
 * values to YUV values. This function also decides which conversion
 * functions to use.
 */
void init_yuv_conversion(void) {

  int i;

  /* initialize the RGB -> YUV tables */
  for (i = 0; i < 256; i++) {

    y_r_table[i] = Y_R * i;
    y_g_table[i] = Y_G * i;
    y_b_table[i] = Y_B * i;

    u_r_table[i] = U_R * i;
    u_g_table[i] = U_G * i;
    u_b_table[i] = U_B * i;

    v_r_table[i] = V_R * i;
    v_g_table[i] = V_G * i;
    v_b_table[i] = V_B * i;
  }

  /* determine best YUV444 -> YUY2 converter to use */
  if (xine_mm_accel() & MM_ACCEL_X86_MMX)
    yuv444_to_yuy2 = yuv444_to_yuy2_mmx;
  else
    yuv444_to_yuy2 = yuv444_to_yuy2_c;

  /* determine best YV12 -> YUY2 converter to use */
  if (xine_mm_accel() & MM_ACCEL_X86_MMXEXT)
    yv12_to_yuy2 = yv12_to_yuy2_mmxext;
  else
    yv12_to_yuy2 = yv12_to_yuy2_c;

  /* determine best YV12 -> YUY2 converter to use */
  if (xine_mm_accel() & MM_ACCEL_X86_MMXEXT)
    yuy2_to_yv12 = yuy2_to_yv12_mmxext;
  else
    yuy2_to_yv12 = yuy2_to_yv12_c;


  /* determine best YUV9 -> YV12 converter to use (only the portable C
   * version is available so far) */
  yuv9_to_yv12 = yuv9_to_yv12_c;

  /* determine best YUV411 -> YV12 converter to use (only the portable C
   * version is available so far) */
  yuv411_to_yv12 = yuv411_to_yv12_c;

}
