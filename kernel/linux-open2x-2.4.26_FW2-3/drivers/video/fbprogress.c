/*
 *  Copyright (C) 2000 Cajus Pollmeier
 *
 *  Support for higher color depths added by
 *  Jeroen Asselman <jeroen@asselman.com>

 *	Support for GP2X/MMSP2 added 2007
 *
 *	John Willis <djwillis@open2x.org>
 *
 *
 *  Support for displaying a progress bar and
 *  status message while booting using 8/16/24/32 bits/pixel.
 *
 *  It is accessible via /proc/progress like this:
 *
 *  echo "45 starting init" > /proc/progress
 *
 *  This sets the progress bar to 45% and shows the message
 *  "starting init".
 *
 *  Position definitions:
 *  PROGRESS_BAR_X      306  (upper left
 *  PROGRESS_BAR_Y      310   corner pixel)
 *  PROGRESS_CORNER_X   513  (lower right    (or use PROGRESS_BAR_WIDTH  207)
 *  PROGRESS_CORNER_Y   324   corner pixel)  (or use PROGRESS_BAR_HEIGHT 14)
 *  STRIPE_BAR          1    (optional, defaults to 0) (0 = bar is solid
 *                            color, 1= bar is repeated stripes of color)
 *  PROGRESS_VERTICALLY 0    (optional, defaults to false)
 *  REVERSE_DIRECTION   0    (optional, defaults to false)
 *  TEXT_X              305
 *  TEXT_Y              294
 *  ICON_X              290
 *  ICON_Y              311
 *
 *  Text definitions:
 *  MAX_TEXT_LEN        25
 *  MESSAGE_LENGTH      80
 *  LPP_TEXT_BLOCK_HEIGHT 4
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/tty.h>
#include <linux/console.h>
#include <linux/string.h>
#include <linux/kd.h>
#include <linux/malloc.h>
#include <linux/fb.h>
#include <linux/vt_kern.h>
#include <linux/selection.h>
#include <linux/smp.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/version.h>

#include <asm/irq.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#include <video/fbcon.h>
#include <linux/boot_font.h>

#include "fbprogress.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
  #define LINUX22
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
  #undef LINUX22
#endif

//senquack
#define LPP_BPP ((p->var.bits_per_pixel+7)/8)
//senquack - changing it to this did nothing to fix progress bug:
//#define LPP_BPP 2

#define LPP_CHAR_HEIGHT 11
#define LPP_CHAR_WIDTH 8

/*
 * Compile time computations based on theme.
 */

/* Establish default choices. */
#ifndef PROGRESS_VERTICALLY
#   define PROGRESS_VERTICALLY 0
#endif

#ifndef REVERSE_DIRECTION
#   define REVERSE_DIRECTION 0
#endif

#ifndef STRIPE_BAR
#   define STRIPE_BAR 0
#endif

/* Support human-friendly "diagonal corner" defines in definitions file.
 * (Exploit compiler's constant folding.) */
#ifndef PROGRESS_BAR_WIDTH
#   define PROGRESS_BAR_WIDTH (PROGRESS_CORNER_X - PROGRESS_BAR_X + 1)
#endif

#ifndef PROGRESS_BAR_HEIGHT
#   define PROGRESS_BAR_HEIGHT (PROGRESS_CORNER_Y - PROGRESS_BAR_Y + 1)
#endif

/* The size of the icons are limited by the minimum dimention of the
 * progress bar. */
#if PROGRESS_BAR_HEIGHT < PROGRESS_BAR_WIDTH
#   define ICON_EDGE_LENGTH PROGRESS_BAR_HEIGHT
#else
#   define ICON_EDGE_LENGTH PROGRESS_BAR_WIDTH
#endif

#if PROGRESS_VERTICALLY
    /* Progress bar is oriented vertically. */
    /* (HEIGHT and WIDTH are vertical and horzontal, LENGTH and THICKNESS
     *  give longways and shortways dimensions. */
#   define PB_LENGTH PROGRESS_BAR_HEIGHT
#   define PB_THICKNESS PROGRESS_BAR_WIDTH
    /* The Y dimention represents progess */
#   define SIG_DIM(X, Y) Y
    /* Don't apply a progress offset to the X dimention */
#   define APPLY_X_OFFSET(OFFSET) 0
    /* Apply progress offset to Y dimension */
#   define APPLY_Y_OFFSET(OFFSET) OFFSET
#else
    /* Progress bar is oriented horizontally. */
#   define PB_LENGTH PROGRESS_BAR_WIDTH
#   define PB_THICKNESS PROGRESS_BAR_HEIGHT
    /* The X dimention represents progess */
#   define SIG_DIM(X, Y) X
    /* Apply progress offset to X dimension */
#   define APPLY_X_OFFSET(OFFSET) OFFSET
    /* Don't apply a progress offset to the Y dimention */
#   define APPLY_Y_OFFSET(OFFSET) 0
#endif

/* Is the pixel in the forground of the progress bar? */
#if REVERSE_DIRECTION
    /* Exchange forgound for background in bar.  (Elsewhere the bar
     * is flipped about the 50% line.) */
#   define PIXEL_FG(X, Y, EDGE_COORD) (SIG_DIM(X, Y) > EDGE_COORD)
#else
#   define PIXEL_FG(X, Y, EDGE_COORD) (SIG_DIM(X,Y) < EDGE_COORD)
#endif

int fbcon_set_progress(struct file *file, const char *buffer,
		       unsigned long count, void *data);

void fbcon_progress(unsigned int progress, char *text);

static int module_permission(struct inode *inode, int op);

static ssize_t module_input(struct file *file, const char *buf,
		            size_t length, loff_t *offset);

static ssize_t module_output(struct file *file, char *buf, size_t len,
		             loff_t *offset);

/* globals */
/* we don't know how many bpp before hand, it shouldn't be biger than 4 */
#define LPP_MAX_BPP 4
#if STRIPE_BAR
    unsigned char bg_color[LPP_MAX_BPP*PB_THICKNESS] = {0};
    unsigned char fg_color[LPP_MAX_BPP*PB_THICKNESS] = {0};
#else
    unsigned char bg_color[LPP_MAX_BPP] = {0};
    unsigned char fg_color[LPP_MAX_BPP] = {0};
#endif
unsigned char bg_text[LPP_MAX_BPP] = {0};
unsigned char fg_text[LPP_MAX_BPP] = {0};
unsigned char warning[ICON_EDGE_LENGTH*ICON_EDGE_LENGTH*LPP_MAX_BPP];
unsigned char failure[ICON_EDGE_LENGTH*ICON_EDGE_LENGTH*LPP_MAX_BPP];
int last_progress;
int ignore_proc;

static struct file_operations file_fbcon_progress_proc_entry =
{
    NULL,
    module_output,  /* "read" from the file */
    module_input,   /* "write" to the file */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL, /* module_open, */
    NULL,
    NULL, /*module_close, */
};

static struct inode_operations  inode_fbcon_progress_proc_entry =
{
    &file_fbcon_progress_proc_entry,
    NULL, /* create */
    NULL, /* lookup */
    NULL, /* link */
    NULL, /* unlink */
    NULL, /* symlink */
    NULL, /* mkdir */
    NULL, /* rmdir */
    NULL, /* mknod */
    NULL, /* rename */
    NULL, /* readlink */
    NULL, /* follow_link */
    NULL, /* readpage */
    NULL, /* writepage */
    NULL, /* bmap */
    NULL, /* truncate */
    module_permission /* check for permissions */
};

#ifdef LINUX22
/* Directory entry */
static struct proc_dir_entry fbcon_progress_proc_entry =
{
    0,
    8, "progress",
    S_IFREG | S_IRUGO | S_IWUSR,
    1,
    0, 0,
    80,
    &inode_fbcon_progress_proc_entry,
    NULL
};
#else
static struct proc_dir_entry *fbprogress_proc_entry;
#endif

static unsigned int fbcon_atoi(char *str)
{
        unsigned int    val;
        int             c;
        char            *sp;

        val = 0;
        sp = str;

        for (; (*sp != 0); sp++) {
                c = *sp - '0';
                if ((c < 0) || (c >= 10)) {
                        break;
                }
                val = (val * 10) + c;
        }
	return(val);
}

static int module_permission(struct inode *inode, int op)
{
  /* We allow everybody to read from our module, but
   * only root (uid 0) may write to it */
  if (op == 4 || (op == 2 && current->euid == 0))
    return 0;

  /* If it's anything else, access is denied */
  return -EACCES;
}


void fbcon_draw( unsigned char *pic)
{
   int y;
	//   senquack - fix for progress bar disappearing after init is called.. changing 
	//   	[fg_console] to [0] did nothing to help with progress bar in the end..
   struct display *p = &fb_display[fg_console];
//   struct display *p = &fb_display[0];
   int line = p->next_line;
   unsigned char *fb = p->screen_base + ICON_X*LPP_BPP + ICON_Y * line;

   for( y= 0; y<ICON_EDGE_LENGTH; y++ )
     memcpy((fb+y*line),
	    &pic[y*ICON_EDGE_LENGTH],
	    ICON_EDGE_LENGTH*LPP_BPP);
}


static ssize_t module_input( struct file *file, const char *buf, size_t length, loff_t *offset)
{
  int i, j= 0;
  int space_found= 0;
  int no_message= 0;
  int get_rest= 0;
  static char buffer[MAX_TEXT_LEN];
  static char Message[MESSAGE_LENGTH];
  unsigned int value;

  for(i=0; i<MESSAGE_LENGTH-1 && i<length; i++){
    if(!space_found || get_rest){
      get_user(Message[i], buf+i);
      if( i==0 && Message[0] == 'w' ){
        fbcon_draw(warning);
        get_rest= 1;
	no_message= 1;
      }
      if( i==0 && Message[0] == 'f' ){
        fbcon_draw(failure);
        get_rest= 1;
	no_message= 1;
      }
    }
    else{
      get_user(buffer[j++], buf+i);
      if( j > MAX_TEXT_LEN ) get_rest= 1;
    }
    if( Message[i] == ' ' ){
      space_found= 1;
      Message[i]= '\0';
    }
  }

  if( no_message ) return i;

  buffer[j]=   0;
  Message[i]=  0;

  value= fbcon_atoi(Message);

  if(value > 100 )
    ignore_proc= 1;
  
  if( !ignore_proc ) fbcon_progress(value, buffer);

  return i;
}


static ssize_t module_output( struct file *file, char *buf, size_t len, loff_t *offset)
{
  static int finished = 0;
  int i;
  char message[MESSAGE_LENGTH+30];

  if (finished) {
    finished = 0;
    return 0;
  }

  sprintf(message, "%d", last_progress);

  for(i=0; i<len && message[i]; i++)
    put_user(message[i], buf+i);

  finished = 1;

  return i;
}


void fbcon_render_char( int offset, int x, int y, unsigned char fgcol[], unsigned char bgcol[] )
{
//    struct display *p = &fb_display[fg_console]; /* draw to vt in foreground */
    struct display *p = &fb_display[0]; /* draw to vt in foreground */
    int line = p->next_line;
    int depth=LPP_BPP;
    unsigned char *fb = p->screen_base + x + y * line;
    int i, j;
    unsigned char bits;

    for( i= 0; i<11; i++ ){
      bits= (offset < 69) ? font[offset*11+i] : 0;
      for( j= 0; j<LPP_CHAR_WIDTH; j++ ){
	if( bits & (1 << j) ){
          memcpy((fb+j*depth), fgcol, depth);
	} else {
	  memcpy((fb+j*depth), bgcol, depth);
	}
      }
      fb+= line;
    }
}


void fbcon_write( char *str, int x, int y, int len, unsigned char fgcol[], unsigned char bgcol[])
{
  char *idx= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789():;-+. ";
  char z;
  int offset, k, x_pos= x, sw;
  
  //senquack - moved this up here so LPP_BPP macro will work

	//   senquack - fix for progress bar disappearing after init is called.. changing 
	//   	[fg_console] to [0] did nothing to help with progress bar in the end..
  struct display *p = &fb_display[fg_console];
//  struct display *p = &fb_display[0];

  //senquack - fixing character offsets
  x_pos *= LPP_BPP;

#ifdef LPP_TEXT_BLOCK_HEIGHT
  //  senquack
//  struct display *p = &fb_display[fg_console];
  int line = p->next_line;
  char *tmp = (p->screen_base+x*LPP_BPP + y * line);

  for ( k = 0; k < (LPP_CHAR_HEIGHT * (LPP_TEXT_BLOCK_HEIGHT-1));k++) {
    memcpy(tmp, (tmp+LPP_CHAR_HEIGHT*line),MAX_TEXT_LEN*LPP_CHAR_WIDTH*LPP_BPP);
    tmp+=line;
  }

  y+=11*(LPP_TEXT_BLOCK_HEIGHT-1);
#endif

  sw= 0;

  for( k= 0; k<len; k++ ){
    if( !str[k] ) sw= 1;
    z= sw?' ':str[k];

    for( offset= 0; offset < 70; offset++){
      if( idx[offset] == z ) break;
    }

    fbcon_render_char( offset, x_pos, y, fgcol, bgcol );
	 //senquack - fix
//    x_pos+= LPP_CHAR_WIDTH;
    x_pos+= LPP_CHAR_WIDTH * LPP_BPP;
  }
}


void fbcon_register_progress(void)
{
#ifdef LINUX22
    proc_register(&proc_root, &fbcon_progress_proc_entry);
    printk("FBCON: Trying LINUX22\n");
#else
    fbprogress_proc_entry = create_proc_entry("progress",
                            S_IFREG | S_IRUGO | S_IWUSR,&proc_root);
    if(fbprogress_proc_entry == NULL)
    {
          printk("FBCON: unable to create proc entry\n");
          return;
    }
    printk("FBCON: created proc entry\n");

    fbprogress_proc_entry->proc_iops = &inode_fbcon_progress_proc_entry;
    fbprogress_proc_entry->write_proc = &module_input;
    fbprogress_proc_entry->read_proc = &module_output;
#endif
}


void fbcon_progress( unsigned int progress, char *text )
{
    /* Redraw entire progress bar. */

	//   senquack - fix for progress bar disappearing after init is called
//    struct display *p = &fb_display[fg_console]; /* draw to vt in foreground */
    struct display *p = &fb_display[0]; /* draw to vt in foreground */
    int depth = p->var.bits_per_pixel;
    int line = p->next_line;                     /* _bytes_/screen line */
    unsigned char *fb = p->screen_base;
    unsigned int i, w, l;

    last_progress= progress;

    printk("FBCON: fbcon_progress called\n");
    if( (depth != 8) && (depth != 15) && (depth != 16) && (depth != 24) && (depth != 32) )
         return;

    w= (progress>100?100:progress);   /* force to percentage boundries */
#   if REVERSE_DIRECTION
        w= 100 - w;                   /* flip about the axis at 50% */
#   endif
    w= (w*PB_LENGTH) / 100;              /* scale pixel coord to length of bar */

    depth = (depth+7)/8;
    fb += PROGRESS_BAR_X * depth + PROGRESS_BAR_Y * line;

    for( l= 0; l < PROGRESS_BAR_HEIGHT; l++ ){
        for( i= 0; i < PROGRESS_BAR_WIDTH; i++ ){
#           if STRIPE_BAR
                memcpy(fb,
                       ( PIXEL_FG(i,l, w)
                         ? &fg_color[i*depth]
                         : &bg_color[i*depth] ),
                       depth);
#           else
                memcpy(fb,
                       ( PIXEL_FG(i,l, w)
                         ? fg_color
                         : bg_color ),
                       depth);
#           endif
            fb+=depth;
	}
        fb+= line - PROGRESS_BAR_WIDTH * depth;
    }
    fbcon_write( text, TEXT_X, TEXT_Y,  MAX_TEXT_LEN, fg_text, bg_text );
}


void fbcon_progress_setup(unsigned char *fb, unsigned int line){
   /* line = number of bytes/screen line, _not_ the number of pixels. */
	//
	//   senquack - fix for progress bar disappearing after init is called
//   struct display *p = &fb_display[fg_console]; /* draw to vt in foreground */
   struct display *p = &fb_display[0]; /* draw to vt in foreground */

   int depth = LPP_BPP;
   int y;

#  if STRIPE_BAR
#      if PROGRESS_VERTICALLY
          /* Stripe is layed out in the direction the fb runs, horizontally. */
          memcpy(&fg_color,
                 (fb + (PROGRESS_BAR_X)*depth  + line * PROGRESS_BAR_Y),
                 depth * PB_THICKNESS);
          memcpy(&bg_color,
                 (fb + (PROGRESS_BAR_X)*depth + line * (PROGRESS_BAR_Y + 1)),
	         depth * PB_THICKNESS);
#      else
           /* Stripe is vertical, must copy a pixel at a time. */
	   for( y= 0; y<PB_THICKNESS; y++){
                memcpy(&fg_color[y*depth],
                       (fb + (PROGRESS_BAR_X)*depth
                        + line * (PROGRESS_BAR_Y + y)),
                       depth);
                memcpy(&bg_color[y*depth],
                       (fb + (PROGRESS_BAR_X + 1)*depth
                        + line * (PROGRESS_BAR_Y + y)),
                       depth);
	   }
#      endif
#  else
       /* fg and bg colors are defined by a single pixel */
       memcpy(&fg_color,
              (fb + (PROGRESS_BAR_X)*depth + line * PROGRESS_BAR_Y),
              depth);
       memcpy(&bg_color,
              (fb + (PROGRESS_BAR_X + APPLY_X_OFFSET(1))*depth
                  + line * (PROGRESS_BAR_Y + APPLY_Y_OFFSET(1))),
              depth);
#  endif
   memcpy(&fg_text,
          (fb + (PROGRESS_BAR_X + APPLY_X_OFFSET(2))*depth
              + line * (PROGRESS_BAR_Y + APPLY_Y_OFFSET(2))),
          depth);
   memcpy(&bg_text,
          (fb + (PROGRESS_BAR_X + APPLY_X_OFFSET(3))*depth
              + line * (PROGRESS_BAR_Y + APPLY_Y_OFFSET(3))),
          depth);

   for( y= 0; y<ICON_EDGE_LENGTH; y++ ){
     memcpy(&warning[y*ICON_EDGE_LENGTH],
            (fb + (PROGRESS_BAR_X + APPLY_X_OFFSET(4))*depth
             +line * (PROGRESS_BAR_Y + y + APPLY_Y_OFFSET(4))),
            ICON_EDGE_LENGTH*depth);
     memcpy(&failure[y*ICON_EDGE_LENGTH],
            (fb + (PROGRESS_BAR_X + APPLY_X_OFFSET(4 + ICON_EDGE_LENGTH))*depth
             + line
               * (PROGRESS_BAR_Y + y + APPLY_Y_OFFSET(4 + ICON_EDGE_LENGTH))),
            ICON_EDGE_LENGTH*depth);
   }

   ignore_proc= 0;
   fbcon_progress(0, "booting...");
}
