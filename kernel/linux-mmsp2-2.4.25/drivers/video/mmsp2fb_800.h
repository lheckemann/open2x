/*
 * drivers/video/mmsp2fb.h
 *
 * Copyright (C) 2003-2004 MIZI Research, Inc.
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 * Framebuffer Device Driver header for Magiceyes MMSP2
 *
 * $Id$
 *
 * Revision History:
 *
 * 2004-06-14 Janghoon Lyu <nandy@mizi.com>
 * - initial version
 *
 * 2004-06-21 Jaeoh Cha <jocha@mizi.com>
 * - 640x480 RGB display working!
 *
 */

#define CONFIG_800x600_LCD // ghcstop, for MDK
//#define CONFIG_640x480_LCD // ghcstop, for pmp2

#ifdef CONFIG_640x480_LCD
#define WIDTH	640
#define HEIGHT	480
#define BPPS	16
#elif defined(CONFIG_800x600_LCD)
#define WIDTH	800
#define HEIGHT	600
#define BPPS	16
#endif

/*
 * These are the bitfields for each
 * display depth that we support.
 */
struct mmsp2fb_rgb {
	struct fb_bitfield	red;
	struct fb_bitfield	green;
	struct fb_bitfield	blue;
	struct fb_bitfield	transp;
};

/*
 * This structure describes the machine which we are running on.
 */
struct mmsp2fb_mach_info {
	u_long		pixclock;

	u_short		xres;
	u_short		yres;

	u_char		bpp;
	u_char		hsync_len;
	u_char		left_margin;
	u_char		right_margin;

	u_char		vsync_len;
	u_char		upper_margin;
	u_char		lower_margin;
	u_char		sync;

	u_int		cmap_greyscale:1,
			cmap_inverse:1,
			cmap_static:1,
			unused:29;
};

#define RGB_8	(0)
#define RGB_16	(1)
#define NR_RGB	2

struct mmsp2fb_info {
	struct fb_info		fb;
	signed int		currcon;

	struct mmsp2fb_rgb	*rgb[NR_RGB];

	u_int			max_bpp;
	u_int			max_xres;
	u_int			max_yres;

	/*
	 * These are the addresses we mapped
	 * the framebuffer memory region to.
	 */
	dma_addr_t		map_dma;
	u_char *		map_cpu;
	u_int			map_size;

	u_char *		screen_cpu;
	dma_addr_t		screen_dma;
	u16 *			palette_cpu;
	dma_addr_t		palette_dma;
	u_int			palette_size;

	dma_addr_t		dbar1;
	dma_addr_t		dbar2;

	u_int			cmap_inverse:1,
				cmap_static:1,
				unused:30;
};

#define MMSP2_NAME	"MMSP2"

/*
 *  Debug macros 
 */
#ifdef DEBUG
#  define DPRINTK(fmt, args...)	printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#  define DPRINTK(fmt, args...)
#endif

/*
 * Minimum X and Y resolutions
 */
#define MIN_XRES	64
#define MIN_YRES	64

