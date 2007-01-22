/*---------------------------------------------------------------------------

    png2linuxlogo.c

    Convert an appropriate PNG (80x80, palette, 224 or fewer colors) to
    linux_logo.h format for Linux bootup image on frame-buffer console.
    (Drop-in replacement for /usr/src/linux/include/asm/linux_logo.h on
    most systems--note that "asm" is a symlink to an architecture-specific
    subdirectory, and it doesn't get created until "make [menu]config" is
    run.)

    Limitations:
     - won't quantize grayscale or RGB images to 223 or fewer colors
     - won't accept any image other than 80x80 pixels
     - ignores transparency
     - only "full-color" image customized; emits standard 16-color & bw logos

    Compilation:

      gcc -Wall -O -I../libpng -I../zlib -o png2linuxlogo \
        png2linuxlogo.c -L../libpng -lpng -L../zlib -lz -lm

    or

      gcc -Wall -O -I../libpng -I../zlib -o png2linuxlogo-static \
        png2linuxlogo.c ../libpng/libpng.a ../zlib/libz.a -lm

    (Edit libpng and zlib locations as necessary.)

    Sample usage (assuming "make [menu]config" already run in kernel dir):

      pngtopnm linuxlogo-80x80-rgb.png \
        | ppmquant -fs 223 \
        | pnmtopng \
        | png2linuxlogo \
        > linux_logo.h.new

      mv /usr/src/linux/include/asm/linux_logo.h \
        /usr/src/linux/include/asm/linux_logo.h.old

      cp linux_logo.h.new /usr/src/linux/include/asm/linux_logo.h

      cd /usr/src/linux && make bzImage

  ---------------------------------------------------------------------------

      Copyright (c) 1998-2002 Greg Roelofs.  All rights reserved.

      This software is provided "as is," without warranty of any kind,
      express or implied.  In no event shall the author or contributors
      be held liable for any damages arising in any way from the use of
      this software.

      Permission is granted to anyone to use this software for any purpose,
      including commercial applications, and to alter it and redistribute
      it freely, subject to the following restrictions:

      1. Redistributions of source code must retain the above copyright
         notice, disclaimer, and this list of conditions.
      2. Redistributions in binary form must reproduce the above copyright
         notice, disclaimer, and this list of conditions in the documenta-
         tion and/or other materials provided with the distribution.
      3. All advertising materials mentioning features or use of this
         software must display the following acknowledgment:

            This product includes software developed by Greg Roelofs
            and contributors for the book, "PNG: The Definitive Guide,"
            published by O'Reilly and Associates.

  ---------------------------------------------------------------------------*/

#define VERSION  "1.02 of 9 June 2002"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>	/* isatty() */
#include "png.h"	/* libpng header file:  includes zlib.h and setjmp.h */

#define TRUE 1
#define FALSE 0

typedef struct _jmpbuf_wrapper {
  jmp_buf jmpbuf;
} jmpbuf_wrapper;

static jmpbuf_wrapper p2llPngJmpbufStruct;

static int  png2linuxlogo (FILE *infile, FILE *outfile);
static void p2llPngErrorHandler (png_structp png_ptr, png_const_charp msg);



int main(int argc, char *argv[])
{
    FILE *infile, *outfile;
    int /* c, */ error;

    if (argc > 1) {
        if ((infile = fopen(argv[1], "rb")) == (FILE *)NULL) {
            fprintf(stderr,
              "png2linuxlogo error:  cannot open %s for reading\n", argv[1]);
            return 5;
        }
    } else {
        if (isatty(0) /* || ((c = getc(stdin)) == EOF) */ ) {
            fprintf(stderr, "png2linuxlogo version %s, by Greg Roelofs.\n",
              VERSION);
            fprintf(stderr, "   Compiled with libpng %s; using libpng %s.\n",
              PNG_LIBPNG_VER_STRING, png_libpng_ver);
            fprintf(stderr, "   Compiled with zlib %s; using zlib %s.\n\n",
              ZLIB_VERSION, zlib_version);
            fprintf(stderr, "usage:  png2linuxlogo 80x80-palette-image.png > "
              "linux_logo.h.new\n");
            fprintf(stderr, "   or:  ... | png2linuxlogo > linux_logo.h.new\n");
            fprintf(stderr, "\n   Then edit the \"<arch-FIXME>\" line "
              "appropriately (e.g., \"ia32\" on standard\n");
            fprintf(stderr, "   x86 systems), copy the file to "
              "/usr/src/linux/include/asm/linux_logo.h,\n");
            fprintf(stderr, "   and recompile the kernel (e.g., \"cd "
              "/usr/src/linux; make bzImage\").  Note\n");
            fprintf(stderr, "   that /usr/src/linux/include/asm is "
              "a symlink to an architecture-specific\n");
            fprintf(stderr, "   subdirectory; the link gets created "
              "by \"make config\" or \"make menuconfig\",\n");
            fprintf(stderr, "   so either do that first or fill in "
              "the correct path by hand.\n\n");
            fprintf(stderr, "   (If you don't know how to configure "
              "and compile a Linux kernel, you probably\n");
            fprintf(stderr, "   shouldn't bother with "
              "this program. ;-) )\n\n");
            return 0;
        }
        /* ungetc(c, stdin); */

        /* setmode/fdopen code borrowed from Info-ZIP's funzip.c (Mark Adler) */
        /* [HAVE_SETMODE is same macro used by NetPBM 9.x for Cygwin, etc.] */

#ifdef HAVE_SETMODE   /* DOS, FLEXOS, Human68k, NetWare, OS/2, Win32 */
# if (defined(__HIGHC__) && !defined(FLEXOS))
        setmode(stdin, _BINARY);
# else
        setmode(0, O_BINARY);   /* some buggy C libs require BOTH setmode() */
# endif                         /*  call AND fdopen() in binary mode :-( */
#endif

#ifdef RISCOS
        infile = stdin;
#else
        if ((infile = fdopen(0, "rb")) == (FILE *)NULL) {
            fprintf(stderr, "png2linuxlogo error:  cannot find stdin\n");
            return 5;
        }
#endif
    }

    /* output always goes to stdout (text mode, not binary) */

#ifdef RISCOS
    outfile = stdout;
#else
    if ((outfile = fdopen(1, "w")) == (FILE *)NULL) {
        fprintf(stderr, "png2linuxlogo error:  cannot write to stdout\n");
        return 5;
    }
#endif

    error = png2linuxlogo(infile, outfile);

    fclose(infile);
    fclose(outfile);

    return error;
}



/* This function is based on the Chapter 13 demo code in "PNG: The
 * Definitive Guide" (http://www.libpng.org/pub/png/pngbook.html).
 *
 * Return codes:
 *   0 = okey dokey
 *   1 = infile is not a PNG image
 *   2 = PNG image is wrong type or wrong size or has too many colors
 *   3 = libpng error of some sort (longjmp)
 *   4 = insufficient memory
 */

static int png2linuxlogo(FILE *infile, FILE *outfile)
{
    static int add_bogus_entry = FALSE;
    static png_byte sig[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height, rowbytes;
    int bit_depth, color_type, num_palette;
    png_colorp palette;
    png_bytep p, image_data = NULL;
    png_bytepp row_pointers = NULL;
    int i;
    time_t numtime;
    char ascnumtime[32];


    /* first do a quick check that the file really is a PNG image; could
     * have used slightly more general png_sig_cmp() function instead */
    i = fread(sig, 1, 8, infile);
    if (i < 8 || !png_check_sig(sig, 8)) {
        fprintf(stderr, "png2linuxlogo error: not a PNG image\n");
        fprintf(stderr,
          "  (signature bytes = %02x %02x %02x %02x %02x %02x %02x %02x)\n",
          sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7]);
        return 1;   /* bad signature */
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      &p2llPngJmpbufStruct, p2llPngErrorHandler, NULL);
    if (png_ptr == NULL) {
        fprintf(stderr,
          "png2linuxlogo error: can't allocate libpng main struct\n");
        return 4;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fprintf(stderr,
          "png2linuxlogo error: can't allocate libpng info struct\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return 4;
    }

    /* setjmp() must be called in every non-callback function that calls a
     * PNG-reading libpng function */
    if (setjmp(p2llPngJmpbufStruct.jmpbuf)) {
        fprintf(stderr,
          "png2linuxlogo error: setjmp returns error condition\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 3;
    }

    png_init_io(png_ptr, infile);
    png_set_sig_bytes(png_ptr, 8);  /* we already read the 8 signature bytes */

    png_read_info(png_ptr, info_ptr);  /* read all PNG info up to image data */

    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
      NULL, NULL, NULL);

    /* GRR 20010922:  should allow 1- to 8-bit grayscale, too (but would have
     *  to count colors in 8-bit case) */
    if (color_type != PNG_COLOR_TYPE_PALETTE) {
        fprintf(stderr, "png2linuxlogo error: only palette PNGs supported\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 2;
    }

    if (width != 320 || height != 240) {
        fprintf(stderr,
          "png2linuxlogo error: image is %ldx%ld; must be exactly 320x240\n",
          width, height);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 2;
    }

    if (bit_depth < 8)
        png_set_packing(png_ptr);   /* expand to 1 byte per pixel */

    png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

    if (num_palette > 223) {
        fprintf(stderr,
          "png2linuxlogo error: too many colors (%d); must be less than 224\n",
          num_palette);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 2;
    }

    if (num_palette == 214)
        add_bogus_entry = TRUE;   /* 214 is special:  triggers stock version */

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
         fprintf(stderr, "png2linuxlogo warning: transparency not supported\n");
    }

    png_read_update_info(png_ptr, info_ptr);

    /* allocate space for the PNG image data */
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    if ((image_data = (png_bytep)malloc(rowbytes*height)) == NULL) {
        fprintf(stderr, "png2linuxlogo error: can't allocate image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 4;
    }
    if ((row_pointers = (png_bytepp)malloc(height*sizeof(png_bytep))) == NULL) {
        fprintf(stderr, "png2linuxlogo error: can't allocate row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free(image_data);
        return 4;
    }

    /* set the individual row_pointers to point at the correct offsets */
    for (i = 0;  i < height;  ++i)
        row_pointers[i] = image_data + i*rowbytes;

    png_read_image(png_ptr, row_pointers);   /* read whole image... */
    png_read_end(png_ptr, NULL);             /* ...done! */

    numtime = time((time_t *)NULL);
    strftime(ascnumtime, 32, "%Y/%m/%d %H:%M:%S", localtime(&numtime));

    fprintf(outfile, "\
/* $Id: linux_logo.h,v 1.5 %s png2linuxlogo Exp $\n\
 * include/asm/linux_logo.h: This is a color Linux logo\n\
 *                           to be displayed on boot.\n\
 *\n\
 * Copyright (C) 1996 Larry Ewing (lewing@isc.tamu.edu)\n\
 * Copyright (C) 1998 Jakub Jelinek (jj@sunsite.mff.cuni.cz)\n\
 *\n\
 * You can put anything here, subject to these restrictions:\n\
 *  - LINUX_LOGO_COLORS has to be strictly less than 224;\n\
 *  - image size has to be exactly 80x80;\n\
 *  - index values have to start from 0x20\n\
 *    (i.e., RGB(linux_logo_red[0],\n\
 *               linux_logo_green[0],\n\
 *               linux_logo_blue[0]) is color index 0x20);\n\
 *  - BW image has to be 80x80 as well, with most-significant\n\
 *    bit on the left;\n\
 *  - serial_console ASCII image can be any size,\n\
 *    but it should contain %%s to display the version.\n\
 *\n\
 * This version was autogenerated by png2linuxlogo %s\n\
 * (http://www.libpng.org/pub/png/apps/png2linuxlogo.html),\n\
 * Copyright (C) 2001-2002 Greg Roelofs (newt@pobox.com).\n\
 *\n\
 * Replace /usr/src/linux/include/asm-<your_arch>/linux_logo.h with\n\
 * this (include/asm will point at include/asm-<your_arch> after you\n\
 * do \"make config\" or \"make menuconfig\"), edit the linux_logo_banner\n\
 * line below (\"FIXME\") to match the original, recompile the kernel, and\n\
 * enjoy your new boot logo.  (Unless your display supports 16 or fewer\n\
 * colors, in which case you're out of luck...sorry.)\n\
 */\n\
\n\
#include <linux/init.h>\n\
#include <linux/version.h>\n\
\n\
#define linux_logo_banner \"Linux/<arch-FIXME> version \" UTS_RELEASE\n\
\n\
#define __HAVE_ARCH_LINUX_LOGO		/* needed for 2.4.18, at least */\n\
\n\
#define LINUX_LOGO_COLORS %d\n\
\n\
#ifdef INCLUDE_LINUX_LOGO_DATA\n\
\n\
unsigned char linux_logo_red[] __initdata = {",
      ascnumtime, VERSION, add_bogus_entry? num_palette+1 : num_palette);

    for (i = 0;  i < num_palette;  ++i) {
        if (i > 0)
            fprintf(outfile, ",");
        if ((i & 7) == 0)
            fprintf(outfile, "\n ");
        fprintf(outfile, " 0x%02X", palette[i].red);
    }
    if (add_bogus_entry)
        fprintf(outfile, ", 0x00");

    fprintf(outfile, "\n\
};\n\
\n\
unsigned char linux_logo_green[] __initdata = {");

    for (i = 0;  i < num_palette;  ++i) {
        if (i > 0)
            fprintf(outfile, ",");
        if ((i & 7) == 0)
            fprintf(outfile, "\n ");
        fprintf(outfile, " 0x%02X", palette[i].green);
    }
    if (add_bogus_entry)
        fprintf(outfile, ", 0x00");

    fprintf(outfile, "\n\
};\n\
\n\
unsigned char linux_logo_blue[] __initdata = {");

    for (i = 0;  i < num_palette;  ++i) {
        if (i > 0)
            fprintf(outfile, ",");
        if ((i & 7) == 0)
            fprintf(outfile, "\n ");
        fprintf(outfile, " 0x%02X", palette[i].blue);
    }
    if (add_bogus_entry)
        fprintf(outfile, ", 0x00");

    fprintf(outfile, "\n\
};\n\
\n\
unsigned char linux_logo[] __initdata = {");

    p = image_data;
    for (i = 0;  i < 320*240;  ++i) {
        if (i > 0)
            fprintf(outfile, ",");
        if ((i & 7) == 0)
            fprintf(outfile, "\n ");
        fprintf(outfile, " 0x%02X", (*p++) + 0x20);
    }

    fprintf(outfile, "\n\
};\n\
\n\
#define INCLUDE_LINUX_LOGOBW\n\
#define INCLUDE_LINUX_LOGO16\n\
#include <linux/linux_logo.h>\n\
\n\
#else\n\
\n\
/* prototypes only */\n\
extern unsigned char linux_logo_red[];\n\
extern unsigned char linux_logo_green[];\n\
extern unsigned char linux_logo_blue[];\n\
extern unsigned char linux_logo[];\n\
extern unsigned char linux_logo_bw[];\n\
extern unsigned char linux_logo16_red[];\n\
extern unsigned char linux_logo16_green[];\n\
extern unsigned char linux_logo16_blue[];\n\
extern unsigned char linux_logo16[];\n\
\n\
#endif\n");

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    free(image_data);
    free(row_pointers);

    return 0;
}



static void p2llPngErrorHandler(png_structp png_ptr, png_const_charp msg)
{
  jmpbuf_wrapper  *jmpbuf_ptr;

  /* This function, aside from the extra step of retrieving the "error
   * pointer" (below) and the fact that it exists within the application
   * rather than within libpng, is essentially identical to libpng's
   * default error handler.  The second point is critical:  since both
   * setjmp() and longjmp() are called from the same code, they are
   * guaranteed to have compatible notions of how big a jmp_buf is,
   * regardless of whether _BSD_SOURCE or anything else has (or has not)
   * been defined. */

  fprintf(stderr, "png2linuxlogo:  fatal libpng error: %s\n", msg);
  fflush(stderr);

  jmpbuf_ptr = png_get_error_ptr(png_ptr);
  if (jmpbuf_ptr == NULL) {         /* we are completely hosed now */
    fprintf(stderr, "png2linuxlogo:  EXTREMELY fatal error: jmpbuf"
      " unrecoverable; terminating.\n");
    fflush(stderr);
    exit(99);
  }

  longjmp(jmpbuf_ptr->jmpbuf, 1);
}
