/*
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
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
#include <linux/cpufreq.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/uaccess.h>

#include <video/fbcon.h>
#include <video/fbcon-mfb.h>
#include <video/fbcon-cfb4.h>
#include <video/fbcon-cfb8.h>
#include <video/fbcon-cfb16.h>

#include "mmsp2fb.h"


/*
 * enable this if your panel appears to have broken
 */
#undef CHECK_COMPAT

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif

#ifdef CONFIG_MACH_MMSP2_DTK3
	#define LCD_WIDTH	800		
	#define LCD_HEIGHT	600		
#elif CONFIG_MACH_MMSP2_DTK4
#ifdef CONFIG_MACH_GP2X
	#define LCD_WIDTH	320		
	#define LCD_HEIGHT	240		
#else
	#define LCD_WIDTH	640		
	#define LCD_HEIGHT	480		
#endif
#endif


// 20050128: real framebuffer geometry info
#define PLANE_X_OFFSET 0
#define PLANE_Y_OFFSET 0
#define PLANE_X_WIDTH  LCD_WIDTH
#define PLANE_Y_HEIGHT LCD_HEIGHT

#define BASE_DIV_LCD_CLK	7

//static struct mmsp2fb_info *savefbi;

unsigned int tvHandle=0;
unsigned int dispClkDiv=5;

void (*mmsp2fb_blank_helper)(int blank);
/* EXPORT_SYMBOL(mmsp2fb_blank_helper); */

static int mmsp2fb_ioctl(struct inode *inode, struct file *file, u_int cmd, u_long arg, int con, struct fb_info *info);


static struct mmsp2fb_mach_info inf  __initdata =
{
    pixclock:             0,
    bpp:                  16,
    xres:                 LCD_WIDTH,
    yres:                 LCD_HEIGHT,
    hsync_len:            0,
    vsync_len:            0,
    left_margin:          0,
    upper_margin:         0,
    right_margin:         0,
    lower_margin:         0,
    sync:                 FB_ACTIVATE_NOW , /* value 0 */
};


/* IMHO this looks wrong.  In 8BPP, length should be 8. */
static struct mmsp2fb_rgb rgb_8 = {
        red:    { offset: 0,  length: 4, },
        green:  { offset: 0,  length: 4, },
        blue:   { offset: 0,  length: 4, },
        transp: { offset: 0,  length: 0, },
};

/*
 * S3C2410 
 * 256 Palette Usage (TFT)
 * 256 color palette consist of 256(depth) * 16 bit SPSRAM
 */
static struct mmsp2fb_rgb def_rgb_16 = {
        red:    { offset: 11, length: 5, },
        green:  { offset: 5,  length: 6, },
        blue:   { offset: 0,  length: 5, },
        transp: { offset: 0,  length: 0, },
};

static struct mmsp2fb_rgb def_rgb_24 = {
    red:    {offset: 16, length: 8, },
    green:  {offset: 8,  length: 8, },
    blue:   {offset: 0,  length: 8, },
    transp: {offset: 0,  length: 0, },
};

static void mmsp2fb_lcd_port_init(void)
{
	tDispClkInfo ClkInfo;


	ClkInfo.DISPCLK_SOURCE  = 2;	// UCLK Use PLL Clock. 95846400

#ifdef CONFIG_MACH_GP2X
	ClkInfo.DISPCLK_DIVIDER= 16;   
#else
	ClkInfo.DISPCLK_DIVIDER= 7;
#endif
    
	//ClkInfo.DISPCLK_POL = 0;     	// No Inversion..
	ClkInfo.DISPCLK_POL = 1;     	// No Inv
	PMR_SetDispClk(&ClkInfo);
	DPC_InitHardware(DPC_RGB_666, CFALSE, CFALSE,CFALSE, 0, 0, DPC_USE_PLLCLK);

#ifdef CONFIG_MACH_MMSP2_DTK3
	DPC_UTIL_HVSYNC(DPC_RGB_666, LCD_WIDTH, LCD_HEIGHT, /*5, 5, 16,*/ 128, 216, 40, CTRUE, /*2, 4, 6,*/ 4, 10, 33, CTRUE);
#elif CONFIG_MACH_MMSP2_DTK4

#ifdef CONFIG_MACH_GP2X
	DPC_UTIL_HVSYNC_GPX320240(DPC_RGB_666, LCD_WIDTH,LCD_HEIGHT,30, 4,53, CFALSE,4, 4, 15,CTRUE,CFALSE,CFALSE,CTRUE); 
#endif

#endif


#ifdef CONFIG_MACH_MMSP2_DTK3
	/*
	 * setup VD[23:0]
	 */ 
	GPIOBALTFNHI  = 0xaaaa; // VD[7:0]
	GPIOAALTFNLOW = 0xaaaa;  // VD[15:8]
	GPIOAALTFNHI  = 0xaaaa; // VD[23:16]
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	/*
	 * setup VD[17:0]
	 */ 
	GPIOBALTFNHI  = 0xaaaa; // VD[7:0]
	GPIOAALTFNLOW = 0xaaaa;  // VD[15:8]
	GPIOAALTFNHI &= ~(0x000f);
	GPIOAALTFNHI |= 0x000a;  // VD[17:16]
#endif
	/*
	 * setup GPIO alt.function related with LCD controller
	 */
	set_gpio_ctrl(GPIO_B7, GPIOMD_ALT1, GPIOPU_NOSET); // CLKH
	set_gpio_ctrl(GPIO_B6, GPIOMD_ALT1, GPIOPU_NOSET); // DE
	set_gpio_ctrl(GPIO_B5, GPIOMD_ALT1, GPIOPU_NOSET); // HSYNC
	set_gpio_ctrl(GPIO_B4, GPIOMD_ALT1, GPIOPU_NOSET); // VSYNC
}

/*
 * role : Fill Machine dependant data ,according to  STN or TFT 
 */
static void  __init mmsp2fb_get_machine_info(struct mmsp2fb_info *fbi)
{
	ds_printk("mmsp2fb_get_machine_info\n");
	fbi->max_xres                   = inf.xres;
	fbi->fb.var.xres                = inf.xres;
	fbi->fb.var.xres_virtual        = inf.xres;
	fbi->max_yres                   = inf.yres;
	fbi->fb.var.yres                = inf.yres;
	fbi->fb.var.yres_virtual        = inf.yres;
	fbi->max_bpp                    = inf.bpp;
	fbi->fb.var.bits_per_pixel      = inf.bpp;
	fbi->fb.var.pixclock            = inf.pixclock;
	fbi->fb.var.hsync_len           = inf.hsync_len;
	fbi->fb.var.left_margin         = inf.left_margin;
	fbi->fb.var.right_margin        = inf.right_margin;
	fbi->fb.var.vsync_len           = inf.vsync_len;
	fbi->fb.var.upper_margin        = inf.upper_margin;
	fbi->fb.var.lower_margin        = inf.lower_margin;
	fbi->fb.var.sync                = inf.sync;
	fbi->fb.var.grayscale           = inf.cmap_greyscale;
	fbi->cmap_inverse               = inf.cmap_inverse;
	fbi->cmap_static                = inf.cmap_static;      
}

static void mmsp2fb_lcd_port_init(void );
static int mmsp2fb_activate_var(struct fb_var_screeninfo *var, struct mmsp2fb_info *);
static void set_ctrlr_state(struct mmsp2fb_info *fbi, u_int state);

struct FrameBuffer {
   unsigned short pixel[LCD_HEIGHT][LCD_WIDTH];
};
struct FrameBuffer *FBuf;

#ifdef YOU_WANT_TO_DRAW_TETRAGON
static void lcd_demo(void)
{
        // Test LCD Initialization. by displaying R.G.B and White.
        int i,j;
        for(i=0 ;i<LCD_HEIGHT/2;i++)
        {
                for(j=0;j<LCD_WIDTH;j++)
                {
                  if(j<LCD_WIDTH/2)
                    FBuf->pixel[i][j]= 0xffff;
                  else
                    FBuf->pixel[i][j]= 0xf800;
                }
        }
        for(i=LCD_HEIGHT/2 ;i<LCD_HEIGHT;i++)
        {
                for(j=0;j<LCD_WIDTH;j++)
                {
                if(j<LCD_WIDTH/2)
                    FBuf->pixel[i][j]= 0x07e0;
                else
                    FBuf->pixel[i][j]= 0x001f;
                }
        }
}
#endif


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
 *
 * struct display fb_display[MAX_NR_CONSOLES]; from fbcon.c

 */
static inline struct display *get_con_display(struct fb_info *info, int con)
{
        struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
        return (con < 0) ? fbi->fb.disp : &fb_display[con];
}

/*
 * Get the CMAP pointer for the specified console
 *
 * struct fb_cmap {
 *      __u32 start;                     First entry    
 *      __u32 len;                       Number of entries 
 *      __u16 *red;                      Red values
 *      __u16 *green;
 *      __u16 *blue;
 *      __u16 *transp;                   transparency, can be NULL 
 * };
 *
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
        case 4:  ret = 0 << 12; break;
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


/********************************************************
 * bit mask RGB=565  -> RRRR RGGG GGGB BBBB				*
 *                      1111 1000 0000 0000   0xf800	*
 *                      0000 0111 1110 0000   0x07e0	*
 *                      0000 0000 0001 1111   0x001f	*
 ********************************************************/
static int
mmsp2fb_setpalettereg(u_int regno, u_int red, u_int green, u_int blue,
                       u_int trans, struct fb_info *info)
{
        struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
        u_int val, ret = 1;

        if (regno < fbi->palette_size) {
                val = ((red >> 5) & 0xf800);
                val |= ((green >> 11) & 0x07e0);
                val |= ((blue >> 16) & 0x001f);

                if (regno == 0)
                        val |= palette_pbs(&fbi->fb.var);

                fbi->palette_cpu[regno] = val;

                ret = 0;
        }
        return ret;
}


/* fb_set_cmap(): fbcmap.c

 * struct fb_cmap {
 *      __u32 start;                     First entry
 *      __u32 len;                       Number of entries
 *      __u16 *red;                      Red values
 *      __u16 *green;
 *      __u16 *blue;
 *      __u16 *transp;                   transparency, can be NULL
 * };

 example> aty128fb.c   : aty128_setcolreg()
          cyber2000fb.c: cyber2000fb_setcolreg()

 *  Set a single color register. The values supplied are already
 *  rounded down to the hardware's capabilities (according to the
 *  entries in the var structure). Return != 0 for invalid regno.
 */

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

        switch (var->bits_per_pixel) {
#ifdef FBCON_HAS_CFB4
        case 4:         ret = 0; break;
#endif
#ifdef FBCON_HAS_CFB8
        case 8:         ret = 0; break;
#endif
#ifdef FBCON_HAS_CFB16
        case 16:        ret = 0; break;
#endif
        default:
                break;
        }

        return ret;
}

static inline void mmsp2fb_set_truecolor(u_int is_true_color)
{
        ds_printk("true_color = %d\n", is_true_color);
}

static void
mmsp2fb_hw_set_var(struct fb_var_screeninfo *var, struct mmsp2fb_info *fbi)
{
        u_long palette_mem_size;

        fbi->palette_size =  256;
        palette_mem_size = fbi->palette_size * sizeof(u16);
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
 *      Set the user defined part of the display for the specified console
 */
static int
mmsp2fb_set_var(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{
        struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
        struct fb_var_screeninfo *dvar = get_con_var(&fbi->fb, con);
        struct display *display = get_con_display(&fbi->fb, con);
        int err, chgvar = 0, rgbidx;

        /*
         * Decode var contents into a par structure, adjusting any
         * out of range values.
         */
        err = mmsp2fb_validate_var(var, fbi);
        if (err)        return err;
        if (var->activate & FB_ACTIVATE_TEST)
        {       
                return 0;
        }

        if ((var->activate & FB_ACTIVATE_MASK) != FB_ACTIVATE_NOW)
        {
                return -EINVAL;
        }

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
        case 16:
                display->visual         = FB_VISUAL_TRUECOLOR;
                display->line_length    = var->xres * 2;
                display->dispsw         = &fbcon_cfb16;
                display->dispsw_data    = fbi->fb.pseudo_palette;
                rgbidx                  = RGB_16;
                break;
        default:
                rgbidx = 0;
                display->dispsw = &fbcon_dummy;
                break;
        }

        display->screen_base    = fbi->screen_cpu;
        display->next_line      = display->line_length;
        display->type           = fbi->fb.fix.type;
        display->type_aux       = fbi->fb.fix.type_aux;
        display->ypanstep       = fbi->fb.fix.ypanstep;
        display->ywrapstep      = fbi->fb.fix.ywrapstep;
        display->can_soft_blank = 1;
        display->inverse        = fbi->cmap_inverse;

        *dvar                   = *var;
        dvar->activate          &= ~FB_ACTIVATE_ALL;

        /*
         * Copy the RGB parameters for this display
         * from the machine specific parameters.
         */
        dvar->red               = fbi->rgb[rgbidx]->red;
        dvar->green             = fbi->rgb[rgbidx]->green;
        dvar->blue              = fbi->rgb[rgbidx]->blue;
        dvar->transp            = fbi->rgb[rgbidx]->transp;

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

        if(con != -1 )
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
        fix->visual      = display->visual;
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
        owner:          THIS_MODULE,
        fb_get_fix:     mmsp2fb_get_fix,
        fb_get_var:     mmsp2fb_get_var,
        fb_set_var:     mmsp2fb_set_var,
        fb_get_cmap:    mmsp2fb_get_cmap,
        fb_set_cmap:    mmsp2fb_set_cmap,
        fb_ioctl:	mmsp2fb_ioctl,
};

/*
 *  mmsp2fb_switch():       
 *      Change to the specified console.  Palette and video mode
 *      are changed to the console's stored parameters.
 *
 *      Uh oh, this can be called from a tasklet (IRQ)
 */
static int mmsp2fb_switch(int con, struct fb_info *info)
{
        struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
        struct display *disp;
        struct fb_cmap *cmap;

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
 * Formal definition of the VESA spec:
 *  On
 *      This refers to the state of the display when it is in full operation
 *  Stand-By
 *      This defines an optional operating state of minimal power reduction with
 *      the shortest recovery time
 *  Suspend
 *      This refers to a level of power management in which substantial power
 *      reduction is achieved by the display.  The display can have a longer 
 *      recovery time from this state than from the Stand-by state
 *  Off
 *      This indicates that the display is consuming the lowest level of power
 *      and is non-operational. Recovery from this state may optionally require
 *      the user to manually power on the monitor
 *
 *  Now, the fbdev driver adds an additional state, (blank), where they
 *  turn off the video (maybe by colormap tricks), but don't mess with the
 *  video itself: think of it semantically between on and Stand-By.
 *
 *  So here's what we should do in our fbdev blank routine:
 *
 *      VESA_NO_BLANKING (mode 0)       Video on,  front/back light on
 *      VESA_VSYNC_SUSPEND (mode 1)     Video on,  front/back light off
 *      VESA_HSYNC_SUSPEND (mode 2)     Video on,  front/back light off
 *      VESA_POWERDOWN (mode 3)         Video off, front/back light off
 *
 *  This will match the matrox implementation.
 */

/*
 * mmsp2fb_blank():
 *      Blank the display by setting all palette values to zero.  Note, the
 *      12 and 16 bpp modes don't really use the palette, so this will not
 *      blank the display in all modes.
 *      blank = 0 unblank ;
 *      blank > 0 , VESA level
 */
static void mmsp2fb_blank(int blank, struct fb_info *info)
{
}

static int mmsp2fb_updatevar(int con, struct fb_info *info)
{
        ds_printk("entered\n");
        return 0;
}

/*
 * mmsp2fb_activate_var():
 *      Configures LCD Controller based on entries in var parameter.  Settings are
 *      only written to the controller if changes were made.
 */
static int mmsp2fb_activate_var(struct fb_var_screeninfo *var, struct mmsp2fb_info *fbi)
{
	u_long flags;

#ifdef DS_DEBUG
        if (var->xres < 16        || var->xres > 1024)
                printk(KERN_ERR "%s: invalid xres %d\n",
                        fbi->fb.fix.id, var->xres);
        if (var->hsync_len < 1    || var->hsync_len > 64)
                printk(KERN_ERR "%s: invalid hsync_len %d\n",
                        fbi->fb.fix.id, var->hsync_len);
        if (var->left_margin < 1  || var->left_margin > 255)
                printk(KERN_ERR "%s: invalid left_margin %d\n",
                        fbi->fb.fix.id, var->left_margin);
        if (var->right_margin < 1 || var->right_margin > 255)
                printk(KERN_ERR "%s: invalid right_margin %d\n",
                        fbi->fb.fix.id, var->right_margin);
        if (var->yres < 1         || var->yres > 1024)
                printk(KERN_ERR "%s: invalid yres %d\n",
                        fbi->fb.fix.id, var->yres);
        if (var->vsync_len < 1    || var->vsync_len > 64)
                printk(KERN_ERR "%s: invalid vsync_len %d\n",
                        fbi->fb.fix.id, var->vsync_len);
        if (var->upper_margin < 0 || var->upper_margin > 255)
                printk(KERN_ERR "%s: invalid upper_margin %d\n",
                        fbi->fb.fix.id, var->upper_margin);
        if (var->lower_margin < 0 || var->lower_margin > 255)
                printk(KERN_ERR "%s: invalid lower_margin %d\n",
                        fbi->fb.fix.id, var->lower_margin);
#endif

        /* Update shadow copy atomically */
    	local_irq_save(flags);

		MLC_RGB_SetBPP( MLC_RGB_16BPP );
		MLC_RGB_SetColorKey
		(
			0xff,	// R
			0xff,	// G
			0xff
		);

		MLC_RGB_MixMux
		(
			MLC_RGB_RGN_1,
			MLC_RGB_MIXMUX_PRI,	// Mix or Mux
			0x8 & 0xf		// Alpha Value
		);

		
		if(GetTVCheck())
		{

			if(GetTvMode())
			{

				MLC_RGB_SetScale
				(
					LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH*2, 
					MMSP2_FB_TV_PAL_WIDTH, MMSP2_FB_TV_PAL_HIGHT 
				);

			}
			else	
			{	

				MLC_RGB_SetScale
				(
					LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH*2, 
					MMSP2_FB_TV_NTSC_WIDTH, MMSP2_FB_TV_NTSC_HIGHT 
				);
			}
		}
		else
		{
			MLC_RGB_SetScale
			(
				LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH*2,	// source
				LCD_WIDTH, LCD_HEIGHT				// destination
			);
		
		}

	
		MLC_RGB_SetActivate
		(
			MLC_RGB_RGN_1, MLC_RGB_RGN_ACT
		);
		
		if(GetTVCheck())
		{	
			if(GetTvMode())
			{
	
				MLC_RGB_SetCoord
				(
					MLC_RGB_RGN_1,PLANE_X_OFFSET, MMSP2_FB_TV_PAL_WIDTH,
					PLANE_Y_OFFSET,MMSP2_FB_TV_PAL_HIGHT	
				);

			}
			else
			{
	
				MLC_RGB_SetCoord
				(
					MLC_RGB_RGN_1,PLANE_X_OFFSET, MMSP2_FB_TV_NTSC_WIDTH,
					PLANE_Y_OFFSET,MMSP2_FB_TV_NTSC_HIGHT
				);
			}
		}
		else
		{
		
			MLC_RGB_SetCoord
			(
				MLC_RGB_RGN_1,
				PLANE_X_OFFSET, PLANE_X_OFFSET+PLANE_X_WIDTH,	
				PLANE_Y_OFFSET, PLANE_Y_OFFSET+PLANE_Y_HEIGHT
			);
		}

		MLC_RGB_SetAddress	((unsigned long)fbi->screen_dma, (unsigned long)fbi->screen_dma);

		MLC_RGBOn (MLC_RGB_RGN_1);

	    local_irq_restore(flags);

	    return 0;
}


/*
 * mmsp2fb_map_video_memory():
 *      Allocates the DRAM memory for the frame buffer.  This buffer is
 *      remapped into a non-cached, non-buffered, memory region to
 *      allow palette and pixel writes to occur without flushing the
 *      cache.  Once this area is remapped, all virtual memory
 *      access to the video memory should occur at the new region.
 */
static int __init mmsp2fb_map_video_memory(struct mmsp2fb_info *fbi)
{
        /*
         * We reserve one page for the palette, plus the size
         * of the framebuffer.
         *
         * fbi->map_dma is bus address (physical)
         * fbi->screen_dma = fbi->map_dma + PAGE_SIZE 
         * fbi->map_cpu is virtual address
         * fbi->screen_cpu = fbi->map_cpu + PAGE_SIZE
         *
         * S3C2400 Result
         * map_size is    0x00027000
         * map_cpu  is    0xC2800000
         * smem_start is  0x0CFC1000
         * palette_mem_size     0x20
         */
  
        fbi->map_size = PAGE_ALIGN(fbi->fb.fix.smem_len  + PAGE_SIZE );
        fbi->map_cpu = (u_char *)VA_FB0_BASE;
        fbi->map_dma = (dma_addr_t)PA_FB0_BASE;
        if (fbi->map_cpu)
        {
                fbi->screen_cpu = fbi->map_cpu  + PAGE_SIZE ;
                fbi->screen_dma = fbi->map_dma  + PAGE_SIZE ;
                fbi->fb.fix.smem_start = fbi->screen_dma;
        }

        return fbi->map_cpu ? 0 : -ENOMEM;
}

static struct fb_monspecs monspecs __initdata = {
//  hfreq lower limit, hfreq upper limit, vfreq lower limit, vfreq upper limit
//  hfreq : 30kHz~70kHz, vfreq : 50Hz~75Hz
        30000, 70000, 50, 75, 0 /* Generic */
};


static struct mmsp2fb_info * __init mmsp2fb_init_fbinfo(void)
{
        struct mmsp2fb_info *fbi;

        fbi = kmalloc(sizeof(struct mmsp2fb_info) + sizeof(struct display) +
                      sizeof(u16) * 16, GFP_KERNEL);
        if (!fbi)
                return NULL;

        memset(fbi, 0, sizeof(struct mmsp2fb_info) + sizeof(struct display));

        fbi->currcon            = -1;
        fbi->planeidx           = MMSP2_RGB0_NUM;

        strcpy(fbi->fb.fix.id, MMSP2_RGB0_NAME);
        fbi->fb.node            = -1;    /* What is this ? */

        fbi->fb.fix.type        = FB_TYPE_PACKED_PIXELS;
        fbi->fb.fix.type_aux    = 0;
        fbi->fb.fix.xpanstep    = 0;
        fbi->fb.fix.ypanstep    = 0;
        fbi->fb.fix.ywrapstep   = 0;
        fbi->fb.fix.accel       = FB_ACCEL_NONE; /* No hardware Accelerator */

        fbi->fb.var.nonstd      = 0;
        fbi->fb.var.activate    = FB_ACTIVATE_NOW;
        fbi->fb.var.height      = -1;
        fbi->fb.var.width       = -1;
        fbi->fb.var.accel_flags = 0;
        fbi->fb.var.vmode       = FB_VMODE_NONINTERLACED;

        strcpy(fbi->fb.modename, MMSP2_RGB0_NAME);
        strcpy(fbi->fb.fontname, "Acorn8x8");

        fbi->fb.fbops           = &mmsp2fb_ops;
        fbi->fb.changevar       = NULL;
        fbi->fb.switch_con      = mmsp2fb_switch;
        fbi->fb.updatevar       = mmsp2fb_updatevar;
        fbi->fb.blank           = mmsp2fb_blank;
        fbi->fb.flags           = FBINFO_FLAG_DEFAULT;
        fbi->fb.node            = -1;
        fbi->fb.monspecs        = monspecs;

        fbi->fb.disp            = (struct display *)(fbi + 1); /* golbal display */
        fbi->fb.pseudo_palette  = (void *)(fbi->fb.disp + 1);

        fbi->rgb[RGB_8]         = &rgb_8; 
        fbi->rgb[RGB_16]        = &def_rgb_16;
        fbi->rgb[RGB_24]        = &def_rgb_24;

        mmsp2fb_get_machine_info(fbi);

        fbi->fb.fix.smem_len    =  (fbi->max_xres * fbi->max_yres * fbi->max_bpp) / 8;


        return fbi;
}

#if 1
#define SWAP_BUF1	0x0370
#define SWAP_BUF2	0x0380
#define SWAP_BUF3	0x0390

int rest_buf = 3;
int scaler_writed = 0;

// VSYNC Interrupt routine hhsong 050930 add
static void dpc_vsync_interrupt(void)
{
//	printk("scaler_writed = %d\n",scaler_writed);
//	printk("__REGW(0xC0001802) = 0x%x\n",__REGW(0xC0001802)); // SC_STATUS
//	printk("__REGW(0xC0001838) = 0x%x\n",__REGW(0xC0001838)); // FDC_CNTL
//	printk("__REGW(0xC0001846) = 0x%x\n",__REGW(0xC0001846)); // FDC_STATUS

	if(scaler_writed)
	{
		if(((__REGW(0xC0001802)>>2) & 0x1)) // SC_Done
		{
			if(rest_buf == 1)
			{
				__REGW(0xC00028A2) = __REGW(0xC000181C);
				__REGW(0xC00028A6) = __REGW(0xC000181C);
				__REGW(0xC000181C) = SWAP_BUF1;
				rest_buf = 3;
			}
			else if(rest_buf == 2)
			{
				__REGW(0xC00028A2) = __REGW(0xC000181C);
				__REGW(0xC00028A6) = __REGW(0xC000181C);
				__REGW(0xC000181C) = SWAP_BUF2;
				rest_buf = 1;
			}
			else if(rest_buf == 3)
			{
				__REGW(0xC00028A2) = __REGW(0xC000181C);
				__REGW(0xC00028A6) = __REGW(0xC000181C);
				__REGW(0xC000181C) = SWAP_BUF3;
				rest_buf = 2;
			}
//			printk(" swap __REGW(0xC000181C) = 0x%x\n",__REGW(0xC000181C));
//			printk(" swap __REGW(0xC00028A2) = 0x%x\n",__REGW(0xC00028A2));
			scaler_writed=0;
		}
	}
//	printk("__REGW(0xC0001802) = 0x%x\n",__REGW(0xC0001802)); // SC_STATUS
	__REGW(0xC0002846) = __REGW(0xC0002846) | (0x1<<1); // pending clear

}
#endif


int __init mmsp2fb_init(void)
{
	struct mmsp2fb_info *fbi;
	int ret;

	PMR_Initialize(); 
	DPC_Initialize();
	MLC_Initialize();

	MLC_isTVCheck(CFALSE);
///////////////////// 050930 hhsong add for VSYNC Interrupt ///////////////////
#if 1
	if(request_irq(IRQ_DISP, dpc_vsync_interrupt, 0, "DPC_VSYNC", NULL))
	{
		printk("mmspfb.c : IRQ_DISP request_irq() failed\n");
		free_irq(IRQ_DISP, NULL);
	}
	else
	{
		printk("IRQ_DISP Number = %d \n",IRQ_DISP);
	}
#endif
////////////////////////////////////////////////////////////////////////////////

	mmsp2fb_lcd_port_init();

        fbi = mmsp2fb_init_fbinfo();
        ret = -ENOMEM;
        if (!fbi)
                goto failed;
                
	//savefbi = fbi;

        /* Initialize video memory */
        ret = mmsp2fb_map_video_memory(fbi);
        if (ret)
                goto failed;

        mmsp2fb_set_var(&fbi->fb.var, -1, &fbi->fb);
        ret = register_framebuffer(&fbi->fb);
        if (ret < 0)
                goto failed;
        /*
         * Ok, now enable the LCD controller
         */
        //set_ctrlr_state(fbi, C_ENABLE); 

        DPC_Run();

#ifndef CONFIG_MACH_GP2X
#ifdef CONFIG_MACH_MMSP2_DTK3
	set_gpio_ctrl(GPIO_B0, GPIOMD_OUT, GPIOPU_NOSET);
	write_gpio_bit(GPIO_B0, 1);
#elif CONFIG_MACH_MMSP2_DTK4
	set_gpio_ctrl(GPIO_N0, GPIOMD_OUT, GPIOPU_NOSET);
	write_gpio_bit(GPIO_N0, 1);		// oyh : LCD On
#endif
#else
	/* batt off */
	set_gpio_ctrl(GPIO_H4, GPIOMD_OUT, GPIOPU_NOSET);
	write_gpio_bit(GPIO_H4, 1);

	/* lcd set */
	set_gpio_ctrl(GPIO_H1, GPIOMD_OUT, GPIOPU_NOSET); // LCD_VGH_ONOFF
	set_gpio_ctrl(GPIO_H2, GPIOMD_OUT, GPIOPU_NOSET); // LCD_BACK_ONOFF
	set_gpio_ctrl(GPIO_B3, GPIOMD_OUT, GPIOPU_NOSET); // LCD_RST	
	write_gpio_bit(GPIO_H1,1);		//LCD_VGH_ON
	udelay(100);
	write_gpio_bit(GPIO_H2,1);		//BACK_ON
	udelay(100);
	write_gpio_bit(GPIO_B3,1);		//LCD_RESET HIGH
		
#endif
        /* This driver cannot be unloaded at the moment */
        MOD_INC_USE_COUNT;

	printk("MMSP2 %s framebuffer driver start\n", fbi->fb.fix.id);

        return 0;

failed:
        if (fbi)    kfree(fbi);
        return ret;
}

int __init mmsp2fb_setup(char *options)
{
	ds_printk("mmsp2fb setup\n");
        return 0;
}


static int mmsp2fb_ioctl(struct inode *inode, struct file *file, u_int cmd, u_long arg, int con, struct fb_info *info)
{
	struct mmsp2fb_info *fbi = (struct mmsp2fb_info *)info;
	int err;
	Msgdummy dummymsg, *pdmsg;

	PMsgGetRect      prect;

	pdmsg = &dummymsg;
	err = copy_from_user(pdmsg, (void *)arg, sizeof(Msgdummy));
	if( err ) return -EFAULT;

	switch( MSG(pdmsg) )
	{
		case MMSP2_FB_RGB_ON:
	        	//MLC_RGB_SetColorKey(0xff, 0xff, 0xff);  
			MLC_RGB_MixMux(MLC_RGB_RGN_1, MLC_RGB_MIXMUX_PRI, 0x8 & 0xf);
			
			if(GetTVCheck())
			{
				if(GetTvMode())
				{
				
					MLC_RGB_SetScale
					(
						LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH*2, 
						MMSP2_FB_TV_PAL_WIDTH, MMSP2_FB_TV_PAL_HIGHT 
					);			
				
					MLC_RGB_SetCoord
					(
						MLC_RGB_RGN_1,PLANE_X_OFFSET, MMSP2_FB_TV_PAL_WIDTH,
						PLANE_Y_OFFSET,MMSP2_FB_TV_PAL_HIGHT
					);					
	
				}
				else
				{
					MLC_RGB_SetScale
					(
						LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH*2, 
						MMSP2_FB_TV_NTSC_WIDTH, MMSP2_FB_TV_NTSC_HIGHT 
					);			
				
					MLC_RGB_SetCoord
					(
						MLC_RGB_RGN_1,PLANE_X_OFFSET, MMSP2_FB_TV_NTSC_WIDTH,
						PLANE_Y_OFFSET,MMSP2_FB_TV_NTSC_HIGHT
					);
				}		
			}
			else
			{	
				MLC_RGB_SetScale(LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH*2, LCD_WIDTH, LCD_HEIGHT );
				// startx, endx(=width), starty, endy(=height)
				MLC_RGB_SetCoord(MLC_RGB_RGN_1,	PLANE_X_OFFSET, PLANE_X_OFFSET+PLANE_X_WIDTH, PLANE_Y_OFFSET, PLANE_Y_OFFSET+PLANE_Y_HEIGHT);
			}
			
			MLC_RGB_SetActivate(MLC_RGB_RGN_1, MLC_RGB_RGN_ACT);
			MLC_RGB_SetAddress	((unsigned long)fbi->screen_dma, (unsigned long)fbi->screen_dma);			
			MLC_RGBOn (MLC_RGB_RGN_1);
			MSG(pdmsg) = MSGOK;
			LEN(pdmsg) = 0;
			break;
		case MMSP2_FB_RGB_OFF:
			MLC_RGBOff (MLC_RGB_RGN_1); 
			MLC_RGB_SetActivate(MLC_RGB_RGN_1, MLC_RGB_RGN_DISACT);
			MSG(pdmsg) = MSGOK;
			LEN(pdmsg) = 0;
			break;
		case MMSP2_FB_GET_RECT_INFO:
			prect       = (PMsgGetRect)pdmsg;
			prect->xoff = PLANE_X_OFFSET;
			prect->yoff = PLANE_Y_OFFSET;
			prect->w    = PLANE_X_WIDTH;
			prect->h    = PLANE_Y_HEIGHT;
			MSG(prect)  = MSGOK;
			LEN(prect)  = 0;
			break;
		case MMSP2_FB0_TV_LCD_CHECK:
			if(GetTVCheck()) return 1;
			else return 0;
		case MMSP2_FB0_GET_TV_MODE:
			return GetTvMode();
		case MMSP2_FB0_SET_TV_HANDLE:
			tvHandle=(unsigned int)pdmsg->msgdata[0];
			break;
		case MMSP2_FB0_GET_TV_HANDLE:
			return (int)tvHandle;
		case MMSP2_FB0_SET_LCD_TIMING:
		{
			//PMR_Initialize(); 
			//tDispClkInfo ClkInfo;
			DPC_Stop();
			dispClkDiv=(unsigned int)pdmsg->msgdata[0];
			printk("dispClkDiv:%d\n",dispClkDiv);
			__REGW(0xC0000924)&=(~(0x3f << 8));
			__REGW(0xC0000924)|=(BASE_DIV_LCD_CLK + dispClkDiv) << 8;
			udelay(10);
			//ClkInfo.DISPCLK_DIVIDER=BASE_DIV_LCD_CLK + dispClkDiv;  
			DPC_Run();	
			break;
		}	
		case MMSP2_FB0_GET_LCD_TIMING:
			printk("get disp:%d\n",dispClkDiv);
			MSG(pdmsg) = dispClkDiv;
			return (int)dispClkDiv;
		default:
			err = -EFAULT;
			break;
	}

	if( err != -EFAULT )
	{
		err = copy_to_user((void *)arg, pdmsg, sizeof(Msgdummy));
	}

	if(err) err = -EFAULT;
	else    err = 0;

	return err;
}

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_DESCRIPTION("MMSP2(Magiceyes) framebuffer driver");
