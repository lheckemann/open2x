/*
 * drivers/video/mmsp2fb.c
 *
 * Copyright (C) 2003-2004 MIZI Research, Inc.
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 * Framebuffer Device Driver Source for Magiceyes MMSP2
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


#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <video/fbcon.h>
#include <video/fbcon-mfb.h>
#include <video/fbcon-cfb4.h>
#include <video/fbcon-cfb8.h>
#include <video/fbcon-cfb16.h>

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef int	S32;
typedef unsigned int CBOOL;

/*
 * debugging?
 */
#define DEBUG
#undef DEBUG

#include "mmsp2fb_800.h"


/*
 * IMHO this looks wrong.  In 8BPP, length should be 8.
 */
static struct mmsp2fb_rgb rgb_8 = {
	red:	{ offset: 0,  length: 4, },
	green:	{ offset: 0,  length: 4, },
	blue:	{ offset: 0,  length: 4, },
	transp:	{ offset: 0,  length: 0, },
};

static struct mmsp2fb_rgb def_rgb_16 = {
	red:	{ offset: 11, length: 5, },
	green:	{ offset: 5,  length: 6, },
	blue:	{ offset: 0,  length: 5, },
	transp:	{ offset: 0,  length: 0, },
};

static struct mmsp2fb_mach_info mmsp2lcd_info __initdata =
{
	pixclock:	73030,	bpp:		BPPS/*16*/,
	xres:		WIDTH/*640*/,	yres:		HEIGHT/*480*/,

	hsync_len:	32,	vsync_len:	19,
	left_margin:	120,	upper_margin:	33,
	right_margin:	17,	lower_margin:	12,

	sync:			0,
};

static struct mmsp2fb_mach_info * __init
mmsp2fb_get_machine_info(struct mmsp2fb_info *fbi)
{
	struct mmsp2fb_mach_info *inf = NULL;

	inf = &mmsp2lcd_info;

	return inf;
}

static int mmsp2fb_activate_var(struct fb_var_screeninfo *var, struct mmsp2fb_info *);

/*
 * Get the VAR structure pointer for the specified console
 */
static inline struct fb_var_screeninfo *get_con_var(struct fb_info *info, int con)
{
	struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
	return (con == fbi->currcon || con == -1) ? &fbi->fb.var : &fb_display[con].var;
}

/*
 * Get the DISPLAY structure pointer for the specified console
 */
static inline struct display *get_con_display(struct fb_info *info, int con)
{
	struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
	return (con < 0) ? fbi->fb.disp : &fb_display[con];
}

/*
 * Get the CMAP pointer for the specified console
 */
static inline struct fb_cmap *get_con_cmap(struct fb_info *info, int con)
{
	struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
	return (con == fbi->currcon || con == -1) ? &fbi->fb.cmap : &fb_display[con].cmap;
}

static inline u_int
chan_to_field(u_int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

/*
 * Convert bits-per-pixel to a hardware palette PBS value.
 */
static inline u_int
palette_pbs(struct fb_var_screeninfo *var)
{
	int ret = 0;
	switch (var->bits_per_pixel) {
#ifdef FBCON_HAS_CFB4
	case 4:  ret = 0 << 12;	break;
#endif
#ifdef FBCON_HAS_CFB8
	case 8:  ret = 1 << 12; break;
#endif
#ifdef FBCON_HAS_CFB16
	case 16: ret = 2 << 12; break;
#endif
	}
	return ret;
}

static int
mmsp2fb_setpalettereg(u_int regno, u_int red, u_int green, u_int blue,
		       u_int trans, struct fb_info *info)
{
	struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
	u_int val, ret = 1;

	if (regno < fbi->palette_size) {
		val = ((red >> 4) & 0xf00);
		val |= ((green >> 8) & 0x0f0);
		val |= ((blue >> 12) & 0x00f);

		if (regno == 0)
			val |= palette_pbs(&fbi->fb.var);

		fbi->palette_cpu[regno] = val;
		ret = 0;
	}
	return ret;
}

static int
mmsp2fb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
		   u_int trans, struct fb_info *info)
{
	struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
	struct display *disp = get_con_display(info, fbi->currcon);
	u_int val;
	int ret = 1;

	/*
	 * If inverse mode was selected, invert all the colours
	 * rather than the register number.  The register number
	 * is what you poke into the framebuffer to produce the
	 * colour you requested.
	 */
	if (disp->inverse) {
		red   = 0xffff - red;
		green = 0xffff - green;
		blue  = 0xffff - blue;
	}

	/*
	 * If greyscale is true, then we convert the RGB value
	 * to greyscale no mater what visual we are using.
	 */
	if (fbi->fb.var.grayscale)
		red = green = blue = (19595 * red + 38470 * green +
					7471 * blue) >> 16;

	switch (fbi->fb.disp->visual) {
	case FB_VISUAL_TRUECOLOR:
		/*
		 * 12 or 16-bit True Colour.  We encode the RGB value
		 * according to the RGB bitfield information.
		 */
		if (regno < 16) {
			u16 *pal = fbi->fb.pseudo_palette;

			val  = chan_to_field(red, &fbi->fb.var.red);
			val |= chan_to_field(green, &fbi->fb.var.green);
			val |= chan_to_field(blue, &fbi->fb.var.blue);

			pal[regno] = val;
			ret = 0;
		}
		break;

	case FB_VISUAL_STATIC_PSEUDOCOLOR:
	case FB_VISUAL_PSEUDOCOLOR:
		ret = mmsp2fb_setpalettereg(regno, red, green, blue, trans, info);
		break;
	}

	return ret;
}

/*
 *  mmsp2fb_display_dma_period()
 *    Calculate the minimum period (in picoseconds) between two DMA
 *    requests for the LCD controller.
 */
static unsigned int
mmsp2fb_display_dma_period(struct fb_var_screeninfo *var)
{
	unsigned int mem_bits_per_pixel;

	mem_bits_per_pixel = var->bits_per_pixel;
	if (mem_bits_per_pixel == 12)
		mem_bits_per_pixel = 16;

	/*
	 * Period = pixclock * bits_per_byte * bytes_per_transfer
	 *		/ memory_bits_per_pixel;
	 */
	return var->pixclock * 8 * 16 / mem_bits_per_pixel;
}

/*
 *  mmsp2fb_decode_var():
 *    Get the video params out of 'var'. If a value doesn't fit, round it up,
 *    if it's too big, return -EINVAL.
 *
 *    Suggestion: Round up in the following order: bits_per_pixel, xres,
 *    yres, xres_virtual, yres_virtual, xoffset, yoffset, grayscale,
 *    bitfields, horizontal timing, vertical timing.
 */
static int
mmsp2fb_validate_var(struct fb_var_screeninfo *var,
		      struct mmsp2fb_info *fbi)
{
	int ret = -EINVAL;

	if (var->xres < MIN_XRES)
		var->xres = MIN_XRES;
	if (var->yres < MIN_YRES)
		var->yres = MIN_YRES;
	if (var->xres > fbi->max_xres)
		var->xres = fbi->max_xres;
	if (var->yres > fbi->max_yres)
		var->yres = fbi->max_yres;
	var->xres_virtual =
	    var->xres_virtual < var->xres ? var->xres : var->xres_virtual;
	var->yres_virtual =
	    var->yres_virtual < var->yres ? var->yres : var->yres_virtual;

	DPRINTK("var->bits_per_pixel=%d\n", var->bits_per_pixel);
	switch (var->bits_per_pixel) {
#ifdef FBCON_HAS_CFB4
	case 4:		ret = 0; break;
#endif
#ifdef FBCON_HAS_CFB8
	case 8:		ret = 0; break;
#endif
#ifdef FBCON_HAS_CFB16
	case 16:	ret = 0; break;
#endif
	default:
		break;
	}


	return ret;
}

static inline void mmsp2fb_set_truecolor(u_int is_true_color)
{
	DPRINTK("true_color = %d\n", is_true_color);
}

static void
mmsp2fb_hw_set_var(struct fb_var_screeninfo *var, struct mmsp2fb_info *fbi)
{
	u_long palette_mem_size;

	fbi->palette_size = var->bits_per_pixel == 8 ? 256 : 16;

	palette_mem_size = fbi->palette_size * sizeof(u16);

	DPRINTK("palette_mem_size = 0x%08lx\n", (u_long) palette_mem_size);

	fbi->palette_cpu = (u16 *)(fbi->map_cpu + PAGE_SIZE - palette_mem_size);
	fbi->palette_dma = fbi->map_dma + PAGE_SIZE - palette_mem_size;
	fb_set_cmap(&fbi->fb.cmap, 1, mmsp2fb_setcolreg, &fbi->fb);

	/* Set board control register to handle new color depth */
	mmsp2fb_set_truecolor(var->bits_per_pixel >= 16);

	mmsp2fb_activate_var(var, fbi);
	fbi->palette_cpu[0] = (fbi->palette_cpu[0] &
					 0xcfff) | palette_pbs(var);
}

/*
 * mmsp2fb_set_var():
 *	Set the user defined part of the display for the specified console
 */
static int
mmsp2fb_set_var(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{
	struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
	struct fb_var_screeninfo *dvar = get_con_var(&fbi->fb, con);
	struct display *display = get_con_display(&fbi->fb, con);
	int err, chgvar = 0, rgbidx;

	DPRINTK("set_var\n");

	/*
	 * Decode var contents into a par structure, adjusting any
	 * out of range values.
	 */
	err = mmsp2fb_validate_var(var, fbi);
	if (err)
		return err;

	if (var->activate & FB_ACTIVATE_TEST)
		return 0;

	if ((var->activate & FB_ACTIVATE_MASK) != FB_ACTIVATE_NOW)
		return -EINVAL;

	if (dvar->xres != var->xres)
		chgvar = 1;
	if (dvar->yres != var->yres)
		chgvar = 1;
	if (dvar->xres_virtual != var->xres_virtual)
		chgvar = 1;
	if (dvar->yres_virtual != var->yres_virtual)
		chgvar = 1;
	if (dvar->bits_per_pixel != var->bits_per_pixel)
		chgvar = 1;
	if (con < 0)
		chgvar = 0;

	switch (var->bits_per_pixel) {
#ifdef FBCON_HAS_CFB4
	case 4:
		if (fbi->cmap_static)
			display->visual	= FB_VISUAL_STATIC_PSEUDOCOLOR;
		else
			display->visual	= FB_VISUAL_PSEUDOCOLOR;
		display->line_length	= var->xres / 2;
		display->dispsw		= &fbcon_cfb4;
		rgbidx			= RGB_8;
		break;
#endif
#ifdef FBCON_HAS_CFB8
	case 8:
		if (fbi->cmap_static)
			display->visual	= FB_VISUAL_STATIC_PSEUDOCOLOR;
		else
			display->visual	= FB_VISUAL_PSEUDOCOLOR;
		display->line_length	= var->xres;
		display->dispsw		= &fbcon_cfb8;
		rgbidx			= RGB_8;
		break;
#endif
#ifdef FBCON_HAS_CFB16
	case 16:
		display->visual		= FB_VISUAL_TRUECOLOR;
		display->line_length	= var->xres * 2;
		display->dispsw		= &fbcon_cfb16;
		display->dispsw_data	= fbi->fb.pseudo_palette;
		rgbidx			= RGB_16;
		break;
#endif
	default:
		rgbidx = 0;
		display->dispsw = &fbcon_dummy;
		break;
	}

	display->screen_base	= fbi->screen_cpu;
	display->next_line	= display->line_length;
	display->type		= fbi->fb.fix.type;
	display->type_aux	= fbi->fb.fix.type_aux;
	display->ypanstep	= fbi->fb.fix.ypanstep;
	display->ywrapstep	= fbi->fb.fix.ywrapstep;
	display->can_soft_blank	= 0;
	display->inverse	= fbi->cmap_inverse;

	*dvar			= *var;
	dvar->activate		&= ~FB_ACTIVATE_ALL;

	/*
	 * Copy the RGB parameters for this display
	 * from the machine specific parameters.
	 */
	dvar->red		= fbi->rgb[rgbidx]->red;
	dvar->green		= fbi->rgb[rgbidx]->green;
	dvar->blue		= fbi->rgb[rgbidx]->blue;
	dvar->transp		= fbi->rgb[rgbidx]->transp;

	DPRINTK("RGBT length = %d:%d:%d:%d\n",
		dvar->red.length, dvar->green.length, dvar->blue.length,
		dvar->transp.length);

	DPRINTK("RGBT offset = %d:%d:%d:%d\n",
		dvar->red.offset, dvar->green.offset, dvar->blue.offset,
		dvar->transp.offset);

	/*
	 * Update the old var.  The fbcon drivers still use this.
	 * Once they are using fbi->fb.var, this can be dropped.
	 */
	display->var = *dvar;

	/*
	 * If we are setting all the virtual consoles, also set the
	 * defaults used to create new consoles.
	 */
	if (var->activate & FB_ACTIVATE_ALL)
		fbi->fb.disp->var = *dvar;

	/*
	 * If the console has changed and the console has defined
	 * a changevar function, call that function.
	 */
	if (chgvar && info && fbi->fb.changevar)
		fbi->fb.changevar(con);

	/* If the current console is selected, activate the new var. */
	if (con != fbi->currcon)
		return 0;

	mmsp2fb_hw_set_var(dvar, fbi);

	return 0;
}

static int
__do_set_cmap(struct fb_cmap *cmap, int kspc, int con,
	      struct fb_info *info)
{
	struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
	struct fb_cmap *dcmap = get_con_cmap(info, con);
	int err = 0;

	if (con == -1)
		con = fbi->currcon;

	/* no colormap allocated? (we always have "this" colour map allocated) */
	if (con >= 0)
		err = fb_alloc_cmap(&fb_display[con].cmap, fbi->palette_size, 0);

	if (!err && con == fbi->currcon)
		err = fb_set_cmap(cmap, kspc, mmsp2fb_setcolreg, info);

	if (!err)
		fb_copy_cmap(cmap, dcmap, kspc ? 0 : 1);

	return err;
}

static int
mmsp2fb_set_cmap(struct fb_cmap *cmap, int kspc, int con,
		  struct fb_info *info)
{
	struct display *disp = get_con_display(info, con);

	if (disp->visual == FB_VISUAL_TRUECOLOR ||
	    disp->visual == FB_VISUAL_STATIC_PSEUDOCOLOR)
		return -EINVAL;

	return __do_set_cmap(cmap, kspc, con, info);
}

static int
mmsp2fb_get_fix(struct fb_fix_screeninfo *fix, int con, struct fb_info *info)
{
	struct display *display = get_con_display(info, con);

	*fix = info->fix;

	fix->line_length = display->line_length;
	fix->visual	 = display->visual;
	return 0;
}

static int
mmsp2fb_get_var(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{
	*var = *get_con_var(info, con);
	return 0;
}

static int
mmsp2fb_get_cmap(struct fb_cmap *cmap, int kspc, int con, struct fb_info *info)
{
	struct fb_cmap *dcmap = get_con_cmap(info, con);
	fb_copy_cmap(dcmap, cmap, kspc ? 0 : 2);
	return 0;
}

static struct fb_ops mmsp2fb_ops = {
	owner:		THIS_MODULE,
	fb_get_fix:	mmsp2fb_get_fix,
	fb_get_var:	mmsp2fb_get_var,
	fb_set_var:	mmsp2fb_set_var,
	fb_get_cmap:	mmsp2fb_get_cmap,
	fb_set_cmap:	mmsp2fb_set_cmap,
};

/*
 *  mmsp2fb_switch():       
 *	Change to the specified console.  Palette and video mode
 *      are changed to the console's stored parameters.
 *
 *	Uh oh, this can be called from a tasklet (IRQ)
 */
static int mmsp2fb_switch(int con, struct fb_info *info)
{
	struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
	struct display *disp;
	struct fb_cmap *cmap;

	DPRINTK("con=%d info->modename=%s\n", con, fbi->fb.modename);

	if (con == fbi->currcon)
		return 0;

	if (fbi->currcon >= 0) {
		disp = fb_display + fbi->currcon;

		/*
		 * Save the old colormap and video mode.
		 */
		disp->var = fbi->fb.var;

		if (disp->cmap.len)
			fb_copy_cmap(&fbi->fb.cmap, &disp->cmap, 0);
	}

	fbi->currcon = con;
	disp = fb_display + con;

	/*
	 * Make sure that our colourmap contains 256 entries.
	 */
	fb_alloc_cmap(&fbi->fb.cmap, 256, 0);

	if (disp->cmap.len)
		cmap = &disp->cmap;
	else
		cmap = fb_default_cmap(1 << disp->var.bits_per_pixel);

	fb_copy_cmap(cmap, &fbi->fb.cmap, 0);

	fbi->fb.var = disp->var;
	fbi->fb.var.activate = FB_ACTIVATE_NOW;

	mmsp2fb_set_var(&fbi->fb.var, con, info);
	return 0;
}

/*
 * mmsp2fb_blank():
 *	Blank the display by setting all palette values to zero.  Note, the 
 * 	12 and 16 bpp modes don't really use the palette, so this will not
 *      blank the display in all modes.  
 */
static void mmsp2fb_blank(int blank, struct fb_info *info)
{
	struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;

	DPRINTK("mmsp2fb_blank: blank=%d info->modename=%s\n", blank,
		fbi->fb.modename);
}

static int mmsp2fb_updatevar(int con, struct fb_info *info)
{
	DPRINTK("entered\n");
	return 0;
}



/*
 * mmsp2fb_activate_var():
 *	Configures LCD Controller based on entries in var parameter.  Settings are      
 *	only written to the controller if changes were made.  
 */
static int mmsp2fb_activate_var(struct fb_var_screeninfo *var, struct mmsp2fb_info *fbi)
{
	return 0;
}

static void mmsp2fb_setup_gpio(struct mmsp2fb_info *fbi)
{
}

static void mmsp2fb_enable_controller(void)
{
	set_gpio_ctrl(GPIO_B0, GPIOMD_OUT, GPIOPU_NOSET); // LCD on/off
	write_gpio_bit(GPIO_B0, 1);  // LCD on
}

static void mmsp2fb_disable_controller(void)
{
	set_gpio_ctrl(GPIO_B0, GPIOMD_OUT, GPIOPU_NOSET); // LCD on/off
	write_gpio_bit(GPIO_B0, 0);  // LCD off
}

/*
 *  mmsp2fb_handle_irq: Handle 'LCD DONE' interrupts.
 */
static void mmsp2fb_handle_irq(int irq, void *dev_id, struct pt_regs *regs)
{
}


/*
 * mmsp2fb_map_video_memory():
 *      Allocates the DRAM memory for the frame buffer.  This buffer is  
 *	remapped into a non-cached, non-buffered, memory region to  
 *      allow palette and pixel writes to occur without flushing the 
 *      cache.  Once this area is remapped, all virtual memory
 *      access to the video memory should occur at the new region.
 */
static int __init mmsp2fb_map_video_memory(struct mmsp2fb_info *fbi)
{
	u_long palette_mem_size;

	/*
	 * We reserve one page for the palette, plus the size
	 * of the framebuffer.
	 */
	fbi->map_size = PAGE_ALIGN(fbi->fb.fix.smem_len + PAGE_SIZE);
	fbi->map_cpu = consistent_alloc(GFP_KERNEL, fbi->map_size,
					&fbi->map_dma);

	if (fbi->map_cpu) {
		fbi->screen_cpu = fbi->map_cpu + PAGE_SIZE;
		fbi->screen_dma = fbi->map_dma + PAGE_SIZE;
		fbi->fb.fix.smem_start = fbi->screen_dma;

		fbi->palette_size = fbi->fb.var.bits_per_pixel == 8 ? 256 : 16;

		palette_mem_size = fbi->palette_size * sizeof(u16);

		DPRINTK("palette_mem_size = 0x%08lx\n", (u_long) palette_mem_size);

		fbi->palette_cpu = (u16 *)(fbi->map_cpu + PAGE_SIZE - palette_mem_size);
		fbi->palette_dma = fbi->map_dma + PAGE_SIZE - palette_mem_size;
	}

	return fbi->map_cpu ? 0 : -ENOMEM;
}

/* Fake monspecs to fill in fbinfo structure */
static struct fb_monspecs monspecs __initdata = {
	30000, 70000, 50, 65, 0	/* Generic */
};


static struct mmsp2fb_info * __init mmsp2fb_init_fbinfo(void)
{
	struct mmsp2fb_mach_info *inf;
	struct mmsp2fb_info *fbi;

	fbi = kmalloc(sizeof(struct mmsp2fb_info) + sizeof(struct display) +
		sizeof(u16) * 16, GFP_KERNEL);
	if (!fbi)
		return NULL;

	memset(fbi, 0, sizeof(struct mmsp2fb_info) + sizeof(struct display));

	fbi->currcon		= -1;

	strcpy(fbi->fb.fix.id, MMSP2_NAME);

	fbi->fb.fix.type	= FB_TYPE_PACKED_PIXELS;
	fbi->fb.fix.type_aux	= 0;
	fbi->fb.fix.xpanstep	= 0;
	fbi->fb.fix.ypanstep	= 0;
	fbi->fb.fix.ywrapstep	= 0;
	fbi->fb.fix.accel	= FB_ACCEL_NONE;

	fbi->fb.var.nonstd	= 0;
	fbi->fb.var.activate	= FB_ACTIVATE_NOW;
	fbi->fb.var.height	= -1;
	fbi->fb.var.width	= -1;
	fbi->fb.var.accel_flags	= 0;
	fbi->fb.var.vmode	= FB_VMODE_NONINTERLACED;

	strcpy(fbi->fb.modename, MMSP2_NAME);
	strcpy(fbi->fb.fontname, "Acorn8x8");

	fbi->fb.fbops		= &mmsp2fb_ops;
	fbi->fb.changevar	= NULL;
	fbi->fb.switch_con	= mmsp2fb_switch;
	fbi->fb.updatevar	= mmsp2fb_updatevar;
	fbi->fb.blank		= mmsp2fb_blank;
	fbi->fb.flags		= FBINFO_FLAG_DEFAULT;
	fbi->fb.node		= -1;
	fbi->fb.monspecs	= monspecs;
	fbi->fb.disp		= (struct display *)(fbi + 1);
	fbi->fb.pseudo_palette	= (void *)(fbi->fb.disp + 1);

	fbi->rgb[RGB_8]		= &rgb_8;
	fbi->rgb[RGB_16]	= &def_rgb_16;

	inf = mmsp2fb_get_machine_info(fbi);

	fbi->max_xres			= inf->xres;
	fbi->fb.var.xres		= inf->xres;
	fbi->fb.var.xres_virtual	= inf->xres;
	fbi->max_yres			= inf->yres;
	fbi->fb.var.yres		= inf->yres;
	fbi->fb.var.yres_virtual	= inf->yres;
	fbi->max_bpp			= inf->bpp;
	fbi->fb.var.bits_per_pixel	= inf->bpp;
	fbi->fb.var.pixclock		= inf->pixclock;
	fbi->fb.var.hsync_len		= inf->hsync_len;
	fbi->fb.var.left_margin		= inf->left_margin;
	fbi->fb.var.right_margin	= inf->right_margin;
	fbi->fb.var.vsync_len		= inf->vsync_len;
	fbi->fb.var.upper_margin	= inf->upper_margin;
	fbi->fb.var.lower_margin	= inf->lower_margin;
	fbi->fb.var.sync		= inf->sync;
	fbi->fb.var.grayscale		= inf->cmap_greyscale;
	fbi->cmap_inverse		= inf->cmap_inverse;
	fbi->cmap_static		= inf->cmap_static;
	fbi->fb.fix.smem_len		= fbi->max_xres * fbi->max_yres *
					  fbi->max_bpp / 8;

	return fbi;
}

static void mmsp2_lcd_init(unsigned long mmsp2_fbmem_addr)
{
	mmsp2fb_disable_controller();

	/*
	 * setup VD[7:0]
	 *   GPIOB : 16pin (no bug)
	 */ 
	GPIOBALTFNHI = 0xaaaa; // VD[7:0]

	/*
	 * setup GPIO alt.function related with LCD controller
	 */
	set_gpio_ctrl(GPIO_B7, GPIOMD_ALT1, GPIOPU_NOSET); // CLKH
	set_gpio_ctrl(GPIO_B6, GPIOMD_ALT1, GPIOPU_NOSET); // DE
	set_gpio_ctrl(GPIO_B5, GPIOMD_ALT1, GPIOPU_NOSET); // HSYNC
	set_gpio_ctrl(GPIO_B4, GPIOMD_ALT1, GPIOPU_NOSET); // VSYNC
//	set_gpio_ctrl(GPIO_B3, GPIOMD_ALT1, GPIOPU_NOSET); // LCD_DE
	set_gpio_ctrl(GPIO_B3, GPIOMD_IN, GPIOPU_NOSET);
	set_gpio_ctrl(GPIO_B2, GPIOMD_IN, GPIOPU_NOSET);
	set_gpio_ctrl(GPIO_B1, GPIOMD_IN, GPIOPU_NOSET);

	/* configure clock souce of LCD controller */
	/* FIXME : hard coded : 200MHz / 8 = 25MHz*/
	DISPCSET = (0x1 << 14) | (12 << 7) | (0); // clk = (FPLL / (7+1))
//	DISPCSET = (0x1 << 14) | (7 << 8) | (1<<7); // clk = (FPLL / (7+1))
//	IMAGCSET = (0x1 << 14) | (7 << 8); // clk = (FPLL / (7+1))

	/* setup VD[17:8]
	 *  GPIOA : 16pin (no bug)
	 */
	GPIOAALTFNLOW = 0xaaaa;  // VD[15:8]
	GPIOAALTFNHI &= ~(0x000f);
	GPIOAALTFNHI |= 0x000a; // VD[17:16]

	/* back light - PWM_SetCh1 (0, 1, 127, 256); */
	set_gpio_ctrl(GPIO_L12, GPIOMD_IN, GPIOPU_NOSET); // disable first
	PWMPRES01 &= 0xff00;
	PWMPRES01 |= (0 << 15) | (1 << 8);
	PWMDUTY1 = 250;
	PWMPERIOD1 = 256;
	set_gpio_ctrl(GPIO_L12, GPIOMD_ALT1, GPIOPU_NOSET); // PWM Ch#1 output

	/* Video Post Processor MLC setup start */
	/* Uses RGB layer region1 only */
	MLC_OVLAY_CNTR = 0;
	MLC_OVLAY_CNTR |= (1<<12);

	MLC_STL_CNTL = 0;
	MLC_STL_CNTL |= (2<<9) | (1<<1) | (1<<0);

	MLC_STL_MIXMUX = 0;
	MLC_STL_MIXMUX |= (0<<0);

	MLC_STL_ALPHAL = 0x0fff;
	MLC_STL_ALPHAH = 0xff;

	MLC_STL1_STX = 0;
	MLC_STL1_ENDX = WIDTH-1;
	MLC_STL1_STY = 0;
	MLC_STL1_ENDY = HEIGHT-1;

	MLC_STL_CKEY_GR = 0xffff;
	MLC_STL_CKEY_R = 0xff;

	MLC_STL_HSC = 1024;
	MLC_STL_VSCL = (WIDTH*BPPS/8) & 0xFFFF;
	MLC_STL_VSCH = (WIDTH*BPPS/8) >> 16;
	MLC_STL_HW = WIDTH*BPPS/8;

	MLC_STL_OADRL = (U16)(mmsp2_fbmem_addr & 0xffff);
	MLC_STL_OADRH = (U16)(mmsp2_fbmem_addr >> 16);
	MLC_STL_EADRL = (U16)(mmsp2_fbmem_addr & 0xffff);
	MLC_STL_EADRH = (U16)(mmsp2_fbmem_addr >> 16);

	MLC_OVLAY_CNTR |= (1<<2);
	/* Video Post Processor MLC setup end */
	
	/* Display Controller setup start */
	DPC_CLKCNTL = 0;
	DPC_CLKCNTL |= (2<<3);
	DPC_CNTL = 0;
	DPC_CNTL |= (2<<1); // 6:6:6(2), 5:6:5(1) ?

	DPC_X_MAX = WIDTH - 1;
	DPC_Y_MAX = HEIGHT - 1;
#ifdef CONFIG_640x480_LCD
	DPC_HS_WIDTH = (96-1)<<0;
	DPC_HS_END = (24/2-1)<<0;
	DPC_HS_STR = ((40/2)+(40%1)-1)<<0;
	DPC_DE = (40-1)<<4;

	DPC_V_SYNC = (10<<8) | ((2-1)<<0);
	DPC_V_END = (33+2+10-1)<<0;
#elif defined(CONFIG_800x600_LCD)
	DPC_HS_WIDTH = (128-1)<<0;
	DPC_HS_END = (216/2-1)<<0;
	DPC_HS_STR = ((40/2)+(40%1)-1)<<0;
	DPC_DE = (40-1)<<4;

	DPC_V_SYNC = (10<<8) | ((4-1)<<0);
	DPC_V_END = (33+4+10-1)<<0;
#endif

	DPC_DE |= (0<<14) | (0<<12) | (0<<0);

	DPC_FPIPOL1 = 0;
	DPC_FPIPOL2 = 0;
	DPC_FPIPOL3 = 0;
			
	DPC_C_SYNC = 0;
	//DPC_Y_BLANK = 0;
	//DPC_C_BLANK = 0;
	//DPC_YP_CSYNC = 0;
	DPC_YN_CSYNC = 16;
	DPC_CP_CSYNC = 748;
	//DPC_CN_CSYNC = 0;
	
	DPC_FPIATV1 = 0xffff;
	DPC_FPIATV2 = 0xffff;
	DPC_FPIATV3 = 0xffff;
	DPC_CNTL |= (1<<0);
	/* Display Controller setup end */
	
	DPRINTK("MLC_OVLAY_CNTR=0x%x\n", MLC_OVLAY_CNTR);
	DPRINTK("MLC_STL_CNTL=0x%x\n", MLC_STL_CNTL);
	DPRINTK("MLC_STL_MIXMUX=0x%x\n", MLC_STL_MIXMUX);
	DPRINTK("MLC_STL_ALPHAL=0x%x\n", MLC_STL_ALPHAL);
	DPRINTK("MLC_STL_ALPHAH=0x%x\n", MLC_STL_ALPHAH);
	DPRINTK("MLC_STL1_STX=%d\n", MLC_STL1_STX);
	DPRINTK("MLC_STL1_ENDX=%d\n", MLC_STL1_ENDX);
	DPRINTK("MLC_STL1_STY=%d\n", MLC_STL1_STY);
	DPRINTK("MLC_STL1_ENDY=%d\n", MLC_STL1_ENDY);
	DPRINTK("MLC_STL_CKEY_GR=0x%x\n", MLC_STL_CKEY_GR);
	DPRINTK("MLC_STL_CKEY_R=0x%x\n", MLC_STL_CKEY_R);
	DPRINTK("DPC_CNTL=0x%x\n", DPC_CNTL);
	DPRINTK("DPC_FPICNTL=0x%x\n", DPC_FPICNTL);
	DPRINTK("DPC_FPIPOL1=0x%x\n", DPC_FPIPOL1);
	DPRINTK("DPC_FPIPOL2=0x%x\n", DPC_FPIPOL2);
	DPRINTK("DPC_FPIPOL3=0x%x\n", DPC_FPIPOL3);
	DPRINTK("DPC_FPIATV1=0x%x\n", DPC_FPIATV1);
	DPRINTK("DPC_FPIATV2=0x%x\n", DPC_FPIATV2);
	DPRINTK("DPC_FPIATV3=0x%x\n", DPC_FPIATV3);
	DPRINTK("DPC_X_MAX=%d\n", DPC_X_MAX);
	DPRINTK("DPC_Y_MAX=%d\n", DPC_Y_MAX);
	DPRINTK("DPC_DE=0x%x\n", DPC_DE);
	DPRINTK("DPC_INTR=0x%x\n", DPC_INTR);
	DPRINTK("DPC_CLKCNTL=0x%x\n", DPC_CLKCNTL);
	DPRINTK("DPC_TVBNK=0x%x\n", DPC_TVBNK);
	DPRINTK("DPC_PS=0x%x\n", DPC_PS);
	DPRINTK("DPC_FG=0x%x\n", DPC_FG);
	DPRINTK("DPC_LP=0x%x\n", DPC_LP);
	DPRINTK("DPC_CLKVH=0x%x\n", DPC_CLKVH);
	DPRINTK("DPC_CLKVL=0x%x\n", DPC_CLKVL);
	DPRINTK("DPC_POL=0x%x\n", DPC_POL);
	DPRINTK("DPC_CISSYNC=%d\n", DPC_CISSYNC);
	DPRINTK("DPC_MID_SYNC=%d\n", DPC_MID_SYNC);
	DPRINTK("DPC_C_SYNC=%d\n", DPC_C_SYNC);
	DPRINTK("DPC_Y_BLANK=%d\n", DPC_Y_BLANK);
	DPRINTK("DPC_C_BLANK=%d\n", DPC_C_BLANK);
	DPRINTK("DPC_YP_CSYNC=%d\n", DPC_YP_CSYNC);
	DPRINTK("DPC_YN_CSYNC=%d\n", DPC_YN_CSYNC);
	DPRINTK("DPC_CP_CSYNC=%d\n", DPC_CP_CSYNC);
	DPRINTK("DPC_CN_CSYNC=%d\n", DPC_CN_CSYNC);
	
	mmsp2fb_enable_controller();
}

int __init mmsp2fb_init(void)
{
	struct mmsp2fb_info *fbi;
	int ret;

	fbi = mmsp2fb_init_fbinfo();
	ret = -ENOMEM;
	if (!fbi)
		goto failed;

	/* Initialize video memory */
	ret = mmsp2fb_map_video_memory(fbi);
	if (ret)
		goto failed;

	memset((void *)fbi->screen_cpu, 0, fbi->fb.fix.smem_len);

	mmsp2_lcd_init(fbi->screen_dma);
	mmsp2fb_set_var(&fbi->fb.var, -1, &fbi->fb);

	ret = register_framebuffer(&fbi->fb);
	if (ret < 0)
		goto failed;

	/* This driver cannot be unloaded at the moment */
	MOD_INC_USE_COUNT;

	return 0;

failed:
	if (fbi)
		kfree(fbi);
	return ret;
}

int __init mmsp2fb_setup(char *options)
{
	return 0;
}

MODULE_DESCRIPTION("MMSP2 framebuffer driver");
MODULE_LICENSE("GPL");
