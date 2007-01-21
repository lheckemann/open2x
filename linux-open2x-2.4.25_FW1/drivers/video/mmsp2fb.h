/*
 * linux/drivers/video/mp2520f.h
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 *  Copyright (C) 1999 Eric A. Thomas
 *   Based on acornfb.c Copyright (C) Russell King.
 *
 */

#include <asm/arch/proto_dpc.h>
#include <asm/arch/proto_mlc.h>
#include <asm/arch/proto_pwrman.h>


/*
 * include/linux/fb.h
 *
 * struct fb_bitfield {
 *      __u32 offset;                    beginning of bitfield
 *      __u32 length;                    length of bitfield
 *      __u32 msb_right;                 != 0 : Most significant bit is
 *                                       right
 * };
 * These are the bitfields for each
 * display depth that we support.
 */
struct mmsp2fb_rgb {
        struct fb_bitfield      red;
        struct fb_bitfield      green;
        struct fb_bitfield      blue;
        struct fb_bitfield      transp;
};

/*
 * This structure describes the machine which we are running on.
 */
struct mmsp2fb_mach_info {
        u_long          pixclock;

        u_short         xres;
        u_short         yres;

        u_char          bpp;
        u_char          hsync_len;
        u_char          left_margin;
        u_char          right_margin;

        u_char          vsync_len;
        u_char          upper_margin;
        u_char          lower_margin;
        u_char          sync;

        u_int           cmap_greyscale:1,
                        cmap_inverse:1,
                        cmap_static:1,
                        unused:29;
};

#define RGB_8   (0)
#define RGB_16  (1)
#define RGB_24  (3)
#define NR_RGB  3



/*
 * include/linux/fb.h
struct fb_info {
   char modename[40];                   // default video mode
   kdev_t node;
   int flags;
   int open;                            // Has this been open already ?
#define FBINFO_FLAG_MODULE      1       // Low-level driver is a module
   struct fb_var_screeninfo var;        // Current var
   struct fb_fix_screeninfo fix;        // Current fix
   struct fb_monspecs monspecs;         // Current Monitor specs
   struct fb_cmap cmap;                 // Current cmap
   struct fb_ops *fbops;
   char *screen_base;                   // Virtual address
   struct display *disp;                // initial display variable
   struct vc_data *display_fg;          // Console visible on this display
   char fontname[40];                   // default font name
   devfs_handle_t devfs_handle;         // Devfs handle for new name
   devfs_handle_t devfs_lhandle;        // Devfs handle for compat. symlink
   int (*changevar)(int);               // tell console var has changed
   int (*switch_con)(int, struct fb_info*);
                                        // tell fb to switch consoles
   int (*updatevar)(int, struct fb_info*);
                                        // tell fb to update the vars
   void (*blank)(int, struct fb_info*); // tell fb to (un)blank the screen
                                        // arg = 0: unblank
                                        // arg > 0: VESA level (arg-1)
   void *pseudo_palette;                // Fake palette of 16 colors and
                                           the cursor's color for non
                                           palette mode
   // From here on everything is device dependent
   void *par;
};
*/

struct mmsp2fb_info {
        struct fb_info          fb;
        signed int              currcon;
        u_char                  planeidx;

        struct mmsp2fb_rgb    *rgb[NR_RGB];

        u_int                   max_bpp;
        u_int                   max_xres;
        u_int                   max_yres;

        /*
         * These are the addresses we mapped
         * the framebuffer memory region to.
         */
        u_char *                map_cpu;  // dma virtual address handler
        dma_addr_t              map_dma;  // dma physical address handler
        u_int                   map_size;

        u_char *                screen_cpu;
        dma_addr_t              screen_dma;
        u16 *                   palette_cpu;
        dma_addr_t              palette_dma;
        u_int                   palette_size;

        u_int                   cmap_inverse:1,
                                cmap_static:1,
                                unused:30;

#ifdef CONFIG_PM
        struct pm_dev           *pm;
#endif
#ifdef CONFIG_CPU_FREQ
        struct notifier_block   clockchg;
#endif
};

#define __type_entry(ptr,type,member) ((type *)((char *)(ptr)-offsetof(type,member)))
#define TO_INF(ptr,member)      __type_entry(ptr,struct mmsp2fb_info,member)
#define S3C2410_PALETTE_MODE_VAL(bpp)    (((bpp) & 0x018) << 9)

// RGB Plane
#define MMSP2_RGB0_NAME            "mmsp2_RGB0"
#define MMSP2_RGB1_NAME            "mmsp2_RGB1"
#define MMSP2_RGB2_NAME            "mmsp2_RGB2"
#define MMSP2_RGB3_NAME            "mmsp2_RGB3"
#define MMSP2_RGB4_NAME            "mmsp2_RGB4"
// OSD plane
#define MMSP2_OSD_NAME             "mmsp2_OSD"
// YCbCr plane
#define MMSP2_VIDA_NAME            "mmsp2_videoA"
#define MMSP2_VIDB_NAME            "mmsp2_videoB"



// RGB Plane
#define MMSP2_RGB0_NUM            0
#define MMSP2_RGB1_NUM            1
#define MMSP2_RGB2_NUM            2
#define MMSP2_RGB3_NUM            3
#define MMSP2_RGB4_NUM            4
// OSD plane
#define MMSP2_OSD_NUM             5
// YCbCr plane
#define MMSP2_VIDA_NUM
#define MMSP2_VIDB_NUM            6



/*
 * Minimum X and Y resolutions
 */
#define MIN_XRES        64
#define MIN_YRES        64

typedef struct Ioctl_Msg_Hdr_t
{
    u_short message;
    u_short plane;   // plane idx(number)
    u_short msgleng;
} Msghdr;

typedef struct Ioctl_Dummy_t
{
    u_short message;
    u_short msgleng;
    u_char  msgdata[32];
} Msgdummy, *PMsgdummy;


#define MSG(msg) (((Msghdr *)msg)->message)
#define PLANE(msg) (((Msghdr *)msg)->plane)
#define LEN(msg) (((Msghdr *)msg)->msgleng)



// return msg;
#define MSGOK  1000
#define MSGERR 1001


#define MMSP2_FB_RGB_ON             1
#define MMSP2_FB_RGB_OFF            0

#define MMSP2_FB_GET_RECT_INFO      2
#define MMSP2_FB_SET_RECT_INFO      3

// 미스콜이아 추가
#define MMSP2_FB_RGB_COLOR_KEY		4

#define MMSP2_FB0_SET_LCD_TIMING	7
#define MMSP2_FB0_GET_LCD_TIMING	8
	
#define MMSP2_FB0_TV_LCD_CHECK		10
#define MMSP2_FB0_GET_TV_HANDLE		11
#define MMSP2_FB0_SET_TV_HANDLE		12

#define MMSP2_FB0_GET_TV_MODE		13
#define TV_MODE_NTSC				0
#define TV_MODE_PAL					1


#define MMSP2_FB_TV_NTSC_WIDTH		670
#define MMSP2_FB_TV_NTSC_HIGHT		232

#define MMSP2_FB_TV_PAL_WIDTH		670
#define MMSP2_FB_TV_PAL_HIGHT		280

typedef struct Ioctl_GetRect_t
{
    Msghdr mhdr;
    u_short xoff;
    u_short yoff;
    u_short w;
    u_short h;
} MsgGetRect, *PMsgGetRect;
