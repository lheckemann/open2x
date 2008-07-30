/* PULLED FROM GPH 4.0.0 KERNEL SOURCE FOR OPEN2X F200 COMPATIBILITY- senquack */

/*
 * ac97_plugin_wm97xx.c  --  Touch screen driver for Wolfson WM9705 and WM9712
 *                           AC97 Codecs.
 *
 * Copyright 2003 Wolfson Microelectronics PLC.
 * Author: Liam Girdwood
 *         liam.girdwood@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Notes:
 *
 *  Features:
 *       - supports WM9705, WM9712
 *       - polling mode
 *       - coordinate polling
 *       - adjustable rpu/dpp settings
 *       - adjustable pressure current
 *       - adjustable sample settle delay
 *       - 4 and 5 wire touchscreens (5 wire is WM9712 only)
 *       - pen down detection
 *       - battery monitor
 *       - sample AUX adc's
 *       - power management
 *       - direct AC97 IO from userspace (#define WM97XX_TS_DEBUG)
 *
 *  TODO:
 *       - continuous mode
 *       - adjustable sample rate
 *       - AUX adc in coordinate / continous modes
 *	 - Official device identifier or misc device ?
 *
 *  Revision history
 *    7th May 2003   Initial version.
 *    6th June 2003  Added non module support and AC97 registration.
 *   18th June 2003  Added AUX adc sampling.
 *   23rd June 2003  Did some minimal reformatting, fixed a couple of
 *		     locking bugs and noted a race to fix.
 *   24th June 2003  Added power management and fixed race condition.
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/pm.h>
#include <linux/wm97xx.h>
#include <asm/uaccess.h>        		/* get_user,copy_to_user */
#include <asm/io.h>

#define TS_NAME "wm97xx"
#define TS_MINOR 16
#define WM_TS_VERSION "1.0"
#define AC97_NUM_REG 64

static DECLARE_WAIT_QUEUE_HEAD(wq);
/* #define DECT_TOUCH_INTERRUPT	1  */
/* #define DECT_TOUCH_EVENT		1 */
static struct tq_struct touch_detect_task;

/*
 * Machine specific set up.
 *
 * This is for targets that can support a PEN down interrupt and/or
 * streaming back touch data in an AC97 slot (not slot 1). The
 * streaming touch data is read back via the targets AC97 FIFO's
*/

/*
 * Debug
 */

#define PFX TS_NAME
#define WM97XX_TS_DEBUG 0

#ifdef WM97XX_TS_DEBUG
#define dbg(format, arg...) printk(KERN_DEBUG PFX ": " format "\n" , ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif
#define err(format, arg...) printk(KERN_ERR PFX ": " format "\n" , ## arg)
#define info(format, arg...) printk(KERN_INFO PFX ": " format "\n" , ## arg)
#define warn(format, arg...) printk(KERN_WARNING PFX ": " format "\n" , ## arg)

/*
 * Module parameters
 */


/*
 * Set the codec sample mode.
 *
 * The WM9712 can sample touchscreen data in 3 different operating
 * modes. i.e. polling, coordinate and continous.
 *
 * Polling:-     The driver polls the codec and issues 3 seperate commands
 *               over the AC97 link to read X,Y and pressure.
 *
 * Coordinate: - The driver polls the codec and only issues 1 command over
 *               the AC97 link to read X,Y and pressure. This mode has
 *               strict timing requirements and may drop samples if
 *               interrupted. However, it is less demanding on the AC97
 *               link. Note: this mode requires a larger delay than polling
 *               mode.
 *
 * Continuous:-  The codec automatically samples X,Y and pressure and then
 *               sends the data over the AC97 link in slots. This is the
 *               same method used by the codec when recording audio.
 *
 * Set mode = 0 for polling, 1 for coordinate and 2 for continuous.
 *
 */
MODULE_PARM(mode,"i");
MODULE_PARM_DESC(mode, "Set WM97XX operation mode");
/* static int mode = 0; */
static int mode = 1;
/* static int mode = 2;  */

/*
 * WM9712 - Set internal pull up for pen detect.
 *
 * Pull up is in the range 1.02k (least sensitive) to 64k (most sensitive)
 * i.e. pull up resistance = 64k Ohms / rpu.
 *
 * Adjust this value if you are having problems with pen detect not
 * detecting any down events.
 */
MODULE_PARM(rpu,"i");
MODULE_PARM_DESC(rpu, "Set internal pull up resitor for pen detect.");
static int rpu = 0;

/*
 * WM9705 - Pen detect comparator threshold.
 *
 * 0 to Vmid in 15 steps, 0 = use zero power comparator with Vmid threshold
 * i.e. 1 =  Vmid/15 threshold
 *      15 =  Vmid/1 threshold
 *
 * Adjust this value if you are having problems with pen detect not
 * detecting any down events.
 */
MODULE_PARM(pdd,"i");
MODULE_PARM_DESC(pdd, "Set pen detect comparator threshold");
static int pdd = 0;

/*
 * Set current used for pressure measurement.
 *
 * Set pil = 2 to use 400uA
 *     pil = 1 to use 200uA and
 *     pil = 0 to disable pressure measurement.
 *
 * This is used to increase the range of values returned by the adc
 * when measureing touchpanel pressure.
 */
MODULE_PARM(pil,"i");
MODULE_PARM_DESC(pil, "Set current used for pressure measurement.");
static int pil = 0;
/* static int pil = 2; */

/*
 * WM9712 - Set five_wire = 1 to use a 5 wire touchscreen.
 *
 * NOTE: Five wire mode does not allow for readback of pressure.
 */
MODULE_PARM(five_wire,"i");
MODULE_PARM_DESC(five_wire, "Set 5 wire touchscreen.");
static int five_wire = 0;

/*
 * Set adc sample delay.
 *
 * For accurate touchpanel measurements, some settling time may be
 * required between the switch matrix applying a voltage across the
 * touchpanel plate and the ADC sampling the signal.
 *
 * This delay can be set by setting delay = n, where n is the array
 * position of the delay in the array delay_table below.
 * Long delays > 1ms are supported for completeness, but are not
 * recommended.
 */
MODULE_PARM(delay,"i");
MODULE_PARM_DESC(delay, "Set adc sample delay.");
static int delay = 4;

/*
 * Pen down detection
 *
 * Pen down detection can either be via an interrupt (preferred) or
 * by polling the PDEN bit. This is an option because some systems may
 * not support the pen down interrupt.
 *
 * Set pen_int to 1 to enable interrupt driven pen down detection.
 */
MODULE_PARM(pen_int,"i");
MODULE_PARM_DESC(pen_int, "Set pen down interrupt");
static int pen_int = 0;


typedef struct {
	int is_wm9712:1;                  /* are we a WM9705/12 */
	int is_registered:1;              /* Is the driver AC97 registered */
	int adc_errs;                     /* sample read back errors */
	spinlock_t lock;
	struct ac97_codec *codec;
#if defined(CONFIG_PROC_FS)
	struct proc_dir_entry *wm97xx_ts_ps;
#endif
#if defined(CONFIG_PM)
	struct pm_dev * pm;
	int line_pgal:5;
	int line_pgar:5;
	int phone_pga:5;
	int mic_pgal:5;
	int mic_pgar:5;
#endif
	struct input_dev *idev;		/* input */
	struct completion thread_init;
	struct completion thread_exit;
	struct task_struct *rtask;
	struct semaphore  sem;
	int use_count;
	int restart;
} wm97xx_ts_t;

//DKS - add pad short since GCC likely adds it anyway on ARM and other code expects it
//typedef struct {
//	unsigned short pressure;  // touch pressure
//	unsigned short x;         // calibrated X
//	unsigned short y;         // calibrated Y
//} TS_EVENT;
typedef struct {
	unsigned short pressure;  // touch pressure
	unsigned short x;         // calibrated X
	unsigned short y;         // calibrated Y
	unsigned short pad;
} TS_EVENT;



static inline void poll_delay (void);
static int __init wm97xx_ts_init_module(void);
static inline int pendown (wm97xx_ts_t *ts);
static void wm97xx_interrupt(int irq, void *dev_id, struct pt_regs *regs);
static int wm97xx_poll_read_adc (wm97xx_ts_t* ts, u16 adcsel, u16* sample);
static void init_wm97xx_phy(void);
static int wm97xx_probe(struct ac97_codec *codec, struct ac97_driver *driver);
static void wm97xx_remove(struct ac97_codec *codec,  struct ac97_driver *driver);
static void wm97xx_ts_cleanup_module(void);
static int wm97xx_ts_evt_add(wm97xx_ts_t* ts, u16 pressure, u16 x, u16 y);
static int wm97xx_ts_evt_release(wm97xx_ts_t* ts);
static ssize_t wm97xx_read(struct file *filp, char *buf, size_t count, loff_t *l);
static int wm97xx_open(struct inode * inode, struct file * filp);
static int wm97xx_release(struct inode * inode, struct file * filp);

#if defined(CONFIG_PM)
static int wm97xx_pm_event(struct pm_dev *dev, pm_request_t rqst, void *data);
static void wm97xx_suspend(void);
static void wm97xx_resume(void);
static void wm9712_pga_save(wm97xx_ts_t* ts);
static void wm9712_pga_restore(wm97xx_ts_t* ts);
#endif

/* we only support a single touchscreen */
static wm97xx_ts_t wm97xx_ts;
static struct input_dev wm97xx_input;

/* AC97 registration info */
static struct ac97_driver wm9705_driver = {
	codec_id: 0x574D4C05,
	codec_mask: 0xFFFFFFFF,
	name: "Wolfson WM9705 Touchscreen/BMON",
	probe:	wm97xx_probe,
	remove: __devexit_p(wm97xx_remove),
};

static struct ac97_driver wm9712_driver = {
	codec_id: 0x574D4C12,
	codec_mask: 0xFFFFFFFF,
	name: "Wolfson WM9712 Touchscreen/BMON",
	probe:	wm97xx_probe,
	remove: __devexit_p(wm97xx_remove),
};



/*
 * ADC sample delay times in uS
 */
static const int delay_table[16] = {
	21,			// 1 AC97 Link frames
	42,			// 2
	84,			// 4
	167,		// 8
	333,		// 16
	667,		// 32
	1000,		// 48
	1333,		// 64
	2000,		// 96
	2667,		// 128
	3333,		// 160
	4000,		// 192
	4667,		// 224
	5333,		// 256
	6000,		// 288
	0 			// No delay, switch matrix always on
};


#ifdef DECT_TOUCH_INTERRUPT
static void wm97xx_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	printk("touch interrupt\n");
	if(!pen_int) pen_int=1;
}

#endif

/*
 * Delay after issuing a POLL command.
 *
 * The delay is 3 AC97 link frames + the touchpanel settling delay
 */

static inline void poll_delay(void)
{
	int pdelay = 3 * AC97_LINK_FRAME + delay_table[delay];
	udelay (pdelay);
}

/*
 * Read a sample from the adc in polling mode.
 */
static int wm97xx_poll_read_adc (wm97xx_ts_t* ts, u16 adcsel, u16* sample)
{
	u16 dig1;
	int timeout = 5 * delay;

	/* set up digitiser */
	dig1 = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER1);
	dig1&=0x0fff;
	ts->codec->codec_write(ts->codec, AC97_WM97XX_DIGITISER1, dig1 | adcsel |WM97XX_POLL);

	/* wait 3 AC97 time slots + delay for conversion */
	poll_delay();

	/* wait for POLL to go low */
	while ((ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER1) & WM97XX_POLL) && timeout)
	{
		udelay(AC97_LINK_FRAME);
		timeout--;
	}
	if (timeout > 0)
		*sample = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER_RD);
	else {
		ts->adc_errs++;
		err ("adc sample timeout");
		return 0;
	}

	/* check we have correct sample */
	if ((*sample & 0x7000) != adcsel )
	{
#if 1
		err ("adc wrong sample, read %x got %x", adcsel, *sample & 0x7000);
#endif
		return 0;
	}

	return 1;
}

/*
 * Read a sample from the adc in coordinate mode.
 */
static int wm97xx_coord_read_adc(wm97xx_ts_t* ts, u16* x, u16* y, u16* pressure)
{
	u16 dig1;
	int timeout = 5 * delay;

	/* set up digitiser */
	dig1 = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER1);
	dig1&=0x0fff;
	ts->codec->codec_write(ts->codec, AC97_WM97XX_DIGITISER1, dig1 | WM97XX_ADCSEL_PRES |WM97XX_POLL);

	/* wait 3 AC97 time slots + delay for conversion */
	poll_delay();

	/* read X then wait for 1 AC97 link frame + settling delay */
	*x = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER_RD);
	udelay (AC97_LINK_FRAME + delay_table[delay]);

	/* read Y */
	*y = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER_RD);

	/* wait for POLL to go low and then read pressure */
	while ((ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER1) & WM97XX_POLL)&& timeout)
	{
			udelay(AC97_LINK_FRAME);
			timeout--;
	}
	if (timeout > 0)
		*pressure = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER_RD);
	else {
		ts->adc_errs++;
		err ("adc sample timeout");
		return 0;
	}

	/* check we have correct samples */
	if (((*x & 0x7000) == 0x1000) && ((*y & 0x7000) == 0x2000) && ((*pressure & 0x7000) == 0x3000))
	{
		return 1;
	} else {
		ts->adc_errs++;
#if 0
		err ("adc got wrong samples, got x 0x%x y 0x%x pressure 0x%x", *x, *y, *pressure);
#endif
		return 0;
	}
}

/*
 * Sample the touchscreen in polling mode
 */
#ifdef DECT_TOUCH_EVENT
int wm97xx_poll_touch(wm97xx_ts_t *ts)
{
	u16 x, y, p;
	if (!wm97xx_poll_read_adc(ts, WM97XX_ADCSEL_X, &x))
	{
		info("x %d\n", x);
		return -EIO;
 	}

	if (!wm97xx_poll_read_adc(ts, WM97XX_ADCSEL_Y, &y))
	{
		info("y %d\n", y);
		return -EIO;
 	}
	if (pil && !five_wire)
	{
		if (!wm97xx_poll_read_adc(ts, WM97XX_ADCSEL_PRES, &p))
		{
			info("p %d\n", p);
			return -EIO;
		}
	}
	else
	{
		p = 0xffff;
	}

	wm97xx_ts_evt_add(ts, p, x, y);

	return 1;
}

/*
 * Sample the touchscreen in polling coordinate mode
*/

int wm97xx_poll_coord_touch(wm97xx_ts_t *ts)
{
	u16 x, y, p;

	if (wm97xx_coord_read_adc(ts, &x, &y, &p))
	{
		wm97xx_ts_evt_add(ts, p, x, y);
		return 1;
	} else
		return -EIO;
}

/*
 * Sample the touchscreen in continous mode
 */
int wm97xx_cont_touch(wm97xx_ts_t *ts)
{
	u16 x, y,val,dig1;
	int count;

	/* Not support gp2x */
	/*                  */
	wm97xx_ts_evt_add(ts, 0xffff, x, y);
	return count;
}
#endif

/** Is the pen down */
static inline int pendown (wm97xx_ts_t *ts)
{
	return ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER_RD) & WM97XX_PEN_DOWN;
}


#ifndef DECT_TOUCH_EVENT
static struct file_operations ts_fops =
{
	owner:		THIS_MODULE,
	read:       wm97xx_read,
	open:		wm97xx_open,
	release:	wm97xx_release,
};
#endif

#ifdef CONFIG_PROC_FS
static int wm97xx_read_proc (char *page, char **start, off_t off,
		    int count, int *eof, void *data)
{
	int len = 0, prpu;
	u16 dig1, dig2, digrd, adcsel, adcsrc, slt, prp, rev;
	unsigned long flags;
	char srev = ' ';

	wm97xx_ts_t* ts;

	if ((ts = data) == NULL)
		return -ENODEV;

	spin_lock_irqsave(&ts->lock, flags);
	if (!ts->is_registered) {
		spin_unlock_irqrestore(&ts->lock, flags);
		len += sprintf (page+len, "No device registered\n");
		return len;
	}

	dig1 = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER1);
	dig2 = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER2);
	ts->codec->codec_write(ts->codec, AC97_WM97XX_DIGITISER1, dig1 | WM97XX_POLL);
	poll_delay();

	digrd = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER_RD);
	rev = (ts->codec->codec_read(ts->codec, AC97_WM9712_REV) & 0x000c) >> 2;

	spin_unlock_irqrestore(&ts->lock, flags);

	adcsel = dig1 & 0x7000;
	adcsrc = digrd & 0x7000;
	slt = (dig1 & 0x7) + 5;
	prp = dig2 & 0xc000;
	prpu = dig2 & 0x003f;

	/* driver version */
	len += sprintf (page+len, "Wolfson WM97xx Version %s\n", WM_TS_VERSION);

	/* what we are using */
	len += sprintf (page+len, "Using %s", ts->is_wm9712 ? "WM9712" : "WM9705");
	if (ts->is_wm9712) {
		switch (rev) {
			case 0x0:
				srev = 'A';
			break;
			case 0x1:
				srev = 'B';
			break;
			case 0x2:
				srev = 'D';
			break;
			case 0x3:
				srev = 'E';
			break;
		}
		len += sprintf (page+len, " silicon rev %c\n",srev);
	} else
		len += sprintf (page+len, "\n");

	/* WM97xx settings */
	len += sprintf (page+len, "Settings     :\n%s%s%s%s",
			dig1 & WM97XX_POLL ? " -sampling adc data(poll)\n" : "",
			adcsel ==  WM97XX_ADCSEL_X ? " -adc set to X coordinate\n" : "",
			adcsel ==  WM97XX_ADCSEL_Y ? " -adc set to Y coordinate\n" : "",
			adcsel ==  WM97XX_ADCSEL_PRES ? " -adc set to pressure\n" : "");
	if (ts->is_wm9712) {
		len += sprintf (page+len, "%s%s%s%s",
			adcsel ==  WM9712_ADCSEL_COMP1 ? " -adc set to COMP1/AUX1\n" : "",
			adcsel ==  WM9712_ADCSEL_COMP2 ? " -adc set to COMP2/AUX2\n" : "",
			adcsel ==  WM9712_ADCSEL_BMON ? " -adc set to BMON\n" : "",
			adcsel ==  WM9712_ADCSEL_WIPER ? " -adc set to WIPER\n" : "");
		} else {
		len += sprintf (page+len, "%s%s%s%s",
			adcsel ==  WM9705_ADCSEL_PCBEEP ? " -adc set to PCBEEP\n" : "",
			adcsel ==  WM9705_ADCSEL_PHONE ? " -adc set to PHONE\n" : "",
			adcsel ==  WM9705_ADCSEL_BMON ? " -adc set to BMON\n" : "",
			adcsel ==  WM9705_ADCSEL_AUX ? " -adc set to AUX\n" : "");
		}

	len += sprintf (page+len, "%s%s%s%s%s%s",
			dig1 & WM97XX_COO ? " -coordinate sampling\n" : " -individual sampling\n",
			dig1 & WM97XX_CTC ? " -continuous mode\n" : " -polling mode\n",
			prp == WM97XX_PRP_DET ? " -pen detect enabled, no wake up\n" : "",
			prp == WM97XX_PRP_DETW ? " -pen detect enabled, wake up\n" : "",
			prp == WM97XX_PRP_DET_DIG ? " -pen digitiser and pen detect enabled\n" : "",
			dig1 & WM97XX_SLEN ? " -read back using slot " : " -read back using AC97\n");

	if ((dig1 & WM97XX_SLEN) && slt !=12)
		len += sprintf(page+len, "%d\n", slt);
	len += sprintf (page+len, " -adc sample delay %d uSecs\n", delay_table[(dig1 & 0x00f0) >> 4]);

	if (ts->is_wm9712) {
		if (prpu)
			len += sprintf (page+len, " -rpu %d Ohms\n", 64000/ prpu);
		len += sprintf (page+len, " -pressure current %s uA\n", dig2 & WM9712_PIL ? "400" : "200");
		len += sprintf (page+len, " -using %s wire touchscreen mode", dig2 & WM9712_45W ? "5" : "4");
	} else {
		len += sprintf (page+len, " -pressure current %s uA\n", dig2 & WM9705_PIL ? "400" : "200");
		len += sprintf (page+len, " -%s impedance for PHONE and PCBEEP\n", dig2 & WM9705_PHIZ ? "high" : "low");
	}

	/* WM97xx digitiser read */
	len += sprintf(page+len, "\nADC data:\n%s%d\n%s%s\n",
		" -adc value (decimal) : ", digrd & 0x0fff,
		" -pen ", digrd & 0x8000 ? "Down" : "Up");
	if (ts->is_wm9712) {
		len += sprintf (page+len, "%s%s%s%s",
			adcsrc ==  WM9712_ADCSEL_COMP1 ? " -adc value is COMP1/AUX1\n" : "",
			adcsrc ==  WM9712_ADCSEL_COMP2 ? " -adc value is COMP2/AUX2\n" : "",
			adcsrc ==  WM9712_ADCSEL_BMON ? " -adc value is BMON\n" : "",
			adcsrc ==  WM9712_ADCSEL_WIPER ? " -adc value is WIPER\n" : "");
		} else {
		len += sprintf (page+len, "%s%s%s%s",
			adcsrc ==  WM9705_ADCSEL_PCBEEP ? " -adc value is PCBEEP\n" : "",
			adcsrc ==  WM9705_ADCSEL_PHONE ? " -adc value is PHONE\n" : "",
			adcsrc ==  WM9705_ADCSEL_BMON ? " -adc value is BMON\n" : "",
			adcsrc ==  WM9705_ADCSEL_AUX ? " -adc value is AUX\n" : "");
		}

	/* register dump */
	len += sprintf(page+len, "\nRegisters:\n%s%x\n%s%x\n%s%x\n",
		" -digitiser 1    (0x76) : 0x", dig1,
		" -digitiser 2    (0x78) : 0x", dig2,
		" -digitiser read (0x7a) : 0x", digrd);

	/* errors */
	len += sprintf(page+len, "\nErrors:\n%s%d\n",
		       " -coordinate errors ", ts->adc_errs);
	return len;
}
#endif

#ifdef CONFIG_PM
/* WM97xx Power Management
 * The WM9712 has extra powerdown states that are controlled in
 * seperate registers from the AC97 power management.
 * We will only power down into the extra WM9712 states and leave
 * the AC97 power management to the sound driver.
 */
static int wm97xx_pm_event(struct pm_dev *dev, pm_request_t rqst, void *data)
{
	switch(rqst) {
		case PM_SUSPEND:
			wm97xx_suspend();
			break;
		case PM_RESUME:
			wm97xx_resume();
			break;
	}
	return 0;
}

/*
 * Power down the codec
 */
static void wm97xx_suspend(void)
{
	wm97xx_ts_t* ts = &wm97xx_ts;
	u16 reg;
	unsigned long flags;

	/* are we registered */
	spin_lock_irqsave(&ts->lock, flags);
	if (!ts->is_registered) {
		spin_unlock_irqrestore(&ts->lock, flags);
		return;
	}

	/* wm9705 does not have extra PM */
	if (!ts->is_wm9712) {
		spin_unlock_irqrestore(&ts->lock, flags);
		return;
	}

	/* save and mute the PGA's */
	wm9712_pga_save(ts);

	reg = ts->codec->codec_read(ts->codec, AC97_PHONE_VOL);
	ts->codec->codec_write(ts->codec, AC97_PHONE_VOL, reg | 0x001f);

	reg = ts->codec->codec_read(ts->codec, AC97_MIC_VOL);
	ts->codec->codec_write(ts->codec, AC97_MIC_VOL, reg | 0x1f1f);

	reg = ts->codec->codec_read(ts->codec, AC97_LINEIN_VOL);
	ts->codec->codec_write(ts->codec, AC97_LINEIN_VOL, reg | 0x1f1f);

	/* power down, dont disable the AC link */
	ts->codec->codec_write(ts->codec, AC97_WM9712_POWER, WM9712_PD(14) | WM9712_PD(13) |
							WM9712_PD(12) | WM9712_PD(11) | WM9712_PD(10) |
							WM9712_PD(9) | WM9712_PD(8) | WM9712_PD(7) |
							WM9712_PD(6) | WM9712_PD(5) | WM9712_PD(4) |
							WM9712_PD(3) | WM9712_PD(2) | WM9712_PD(1) |
							WM9712_PD(0));

	spin_unlock_irqrestore(&ts->lock, flags);
}

/*
 * Power up the Codec
 */
static void wm97xx_resume(void)
{
	wm97xx_ts_t* ts = &wm97xx_ts;
	unsigned long flags;

	/* are we registered */
	spin_lock_irqsave(&ts->lock, flags);
	if (!ts->is_registered) {
		spin_unlock_irqrestore(&ts->lock, flags);
		return;
	}

	/* wm9705 does not have extra PM */
	if (!ts->is_wm9712) {
		spin_unlock_irqrestore(&ts->lock, flags);
		return;
	}

	/* power up */
	ts->codec->codec_write(ts->codec, AC97_WM9712_POWER, 0x0);

	/* restore PGA state */
	wm9712_pga_restore(ts);

	spin_unlock_irqrestore(&ts->lock, flags);
}


/* save state of wm9712 PGA's */
static void wm9712_pga_save(wm97xx_ts_t* ts)
{
	ts->phone_pga = ts->codec->codec_read(ts->codec, AC97_PHONE_VOL) & 0x001f;
	ts->line_pgal = ts->codec->codec_read(ts->codec, AC97_LINEIN_VOL) & 0x1f00;
	ts->line_pgar = ts->codec->codec_read(ts->codec, AC97_LINEIN_VOL) & 0x001f;
	ts->mic_pgal = ts->codec->codec_read(ts->codec, AC97_MIC_VOL) & 0x1f00;
	ts->mic_pgar = ts->codec->codec_read(ts->codec, AC97_MIC_VOL) & 0x001f;
}

/* restore state of wm9712 PGA's */
static void wm9712_pga_restore(wm97xx_ts_t* ts)
{
	u16 reg;

	reg = ts->codec->codec_read(ts->codec, AC97_PHONE_VOL);
	ts->codec->codec_write(ts->codec, AC97_PHONE_VOL, reg | ts->phone_pga);

	reg = ts->codec->codec_read(ts->codec, AC97_LINEIN_VOL);
	ts->codec->codec_write(ts->codec, AC97_LINEIN_VOL, reg | ts->line_pgar | (ts->line_pgal << 8));

	reg = ts->codec->codec_read(ts->codec, AC97_MIC_VOL);
	ts->codec->codec_write(ts->codec, AC97_MIC_VOL, reg | ts->mic_pgar | (ts->mic_pgal << 8));
}

#endif

/*
 * set up the physical settings of the device
 */

static void init_wm97xx_phy(void)
{
	u16 dig1, dig2, aux, vid;
	wm97xx_ts_t *ts = &wm97xx_ts;

	/* default values */
/* 	dig1 = WM97XX_DELAY(4) | WM97XX_SLT(6); */
	dig1 = WM97XX_DELAY(4) | WM97XX_SLT(5);
	if (ts->is_wm9712)
		dig2 = WM9712_RPU(1);
	else {
		dig2 = 0x0;

		/*
		 * mute VIDEO and AUX as they share X and Y touchscreen
		 * inputs on the WM9705
		 */
		aux = ts->codec->codec_read(ts->codec, AC97_AUX_VOL);
		if (!(aux & 0x8000)) {
			info("muting AUX mixer as it shares X touchscreen coordinate");
			ts->codec->codec_write(ts->codec, AC97_AUX_VOL, 0x8000 | aux);
		}

		vid = ts->codec->codec_read(ts->codec, AC97_VIDEO_VOL);
		if (!(vid & 0x8000)) {
			info("muting VIDEO mixer as it shares Y touchscreen coordinate");
			ts->codec->codec_write(ts->codec, AC97_VIDEO_VOL, 0x8000 | vid);
		}
	}

	/* WM9712 rpu */
	if (ts->is_wm9712 && rpu)
	{
		dig2 &= 0xffc0;
		dig2 |= WM9712_RPU(rpu);
		info("setting pen detect pull-up to %d Ohms",64000 / rpu);
	}

	/* touchpanel pressure */
	if  (pil == 2) {    	/* 400uA */
		if (ts->is_wm9712)
			dig2 |= WM9712_PIL;
		else
			dig2 |= WM9705_PIL;
		info("setting pressure measurement current to 400uA.");
	} else if (pil)
		info ("setting pressure measurement current to 200uA.");

	/* WM9712 five wire */
	if (ts->is_wm9712 && five_wire)
	{
		dig2 |= WM9712_45W;
		info("setting 5-wire touchscreen mode.");
	}

	/* sample settling delay */
	if (delay!=4)
	{
		if (delay < 0 || delay > 15)
		{
			info ("supplied delay out of range.");
			delay = 4;
		}
		dig1 &= 0xff0f;
		dig1 |= WM97XX_DELAY(delay);
		info("setting adc sample delay to %d u Secs.", delay_table[delay]);
	}

	/* coordinate mode */
	if (mode == 1)
	{
		dig1 |= WM97XX_COO;
#ifdef CONFIG_MACH_GP2X_DEBUG
		info("using coordinate mode");
#endif
	}

	/* continous mode */
	if (mode == 2)
	{
		/* dig1 |= WM97XX_CTC | WM97XX_COO | WM97XX_CM_RATE_375 | WM97XX_SLEN | WM97XX_SLT(5); */
		dig1 &= ~(WM97XX_COO);
		dig1 |= WM97XX_CTC | WM97XX_CM_RATE_375 | WM97XX_SLEN | WM97XX_SLT(5) | WM97XX_ADCSEL_PRES;
		info("using continous mode");

		if (ts->is_wm9712)
			dig2 |= WM9712_PDEN;
		else
			dig2 |= WM9705_PDEN;
	}

	/* WM9705 pdd */
	if (pdd && !ts->is_wm9712)
	{
		dig2 |= (pdd & 0x000f);
		info("setting pdd to Vmid/%d", 1 - (pdd & 0x000f));
	}

	ts->codec->codec_write(ts->codec, AC97_WM97XX_DIGITISER1, dig1);
	ts->codec->codec_write(ts->codec, AC97_WM97XX_DIGITISER2, dig2);
}


/*
 * Called by the audio codec initialisation to register
 * the touchscreen driver.
 */

static int wm97xx_probe(struct ac97_codec *codec, struct ac97_driver *driver)
{
	unsigned long flags;
	u16 id1, id2;
	wm97xx_ts_t *ts = &wm97xx_ts;

	spin_lock_irqsave(&ts->lock, flags);

	/* we only support 1 touchscreen at the moment */
	if (ts->is_registered) {
		spin_unlock_irqrestore(&ts->lock, flags);
		return -1;
	}

	/*
	 * We can only use a WM9705 or WM9712 that has been *first* initialised
	 * by the AC97 audio driver. This is because we have to use the audio
	 * drivers codec read() and write() functions to sample the touchscreen
	 *
	 * If an initialsed WM97xx is found then get the codec read and write
	 * functions.
	 */

	/* test for a WM9712 or a WM9705 */
	id1 = codec->codec_read(codec, AC97_VENDOR_ID1);
	id2 = codec->codec_read(codec, AC97_VENDOR_ID2);
	if (id1 == WM97XX_ID1 && id2 == WM9712_ID2) {
		ts->is_wm9712 = 1;
#ifdef CONFIG_MACH_GP2X_DEBUG
		info("registered a WM9712");
#endif
	} else if (id1 == WM97XX_ID1 && id2 == WM9705_ID2) {
		    ts->is_wm9712 = 0;
		    info("registered a WM9705");
	} else {
		err("could not find a WM97xx codec. Found a 0x%4x:0x%4x instead",
		    id1, id2);
		spin_unlock_irqrestore(&ts->lock, flags);
		return -1;
	}

	/* set up AC97 codec interface */
	ts->codec = codec;
	codec->driver_private = (void*)&ts;

	/* set up physical characteristics */
	init_wm97xx_phy();

	ts->is_registered = 1;
	spin_unlock_irqrestore(&ts->lock, flags);

	return 0;
}

/* this is called by the audio driver when ac97_codec is unloaded */

static void wm97xx_remove(struct ac97_codec *codec, struct ac97_driver *driver)
{
	unsigned long flags;
	u16 dig1, dig2;
	wm97xx_ts_t *ts = codec->driver_private;

	spin_lock_irqsave(&ts->lock, flags);

	/* check that are registered */
	if (!ts->is_registered) {
		err("double unregister");
		spin_unlock_irqrestore(&ts->lock, flags);
		return;
	}

	ts->is_registered = 0;

	/* restore default digitiser values */
	dig1 = WM97XX_DELAY(4) | WM97XX_SLT(6);
	if (ts->is_wm9712)
		dig2 = WM9712_RPU(1);
	else
		dig2 = 0x0;

	codec->codec_write(codec, AC97_WM97XX_DIGITISER1, dig1);
	codec->codec_write(codec, AC97_WM97XX_DIGITISER2, dig2);
	ts->codec = NULL;

	spin_unlock_irqrestore(&ts->lock, flags);

}

#ifndef DECT_TOUCH_EVENT
static struct miscdevice wm97xx_misc = {
	minor:	TS_MINOR,
	name:	"touchscreen/wm97xx",
	fops:	&ts_fops,
};
#endif



#ifdef DECT_TOUCH_EVENT
/*
 * add a touch event
 */
static int wm97xx_ts_evt_add(wm97xx_ts_t* ts, u16 pressure, u16 x, u16 y)
{
	/* add event and remove adc src bits */
	input_report_abs(ts->idev, ABS_X, x & 0xfff);
	input_report_abs(ts->idev, ABS_Y, y & 0xfff);
	input_report_abs(ts->idev, ABS_PRESSURE, pressure & 0xfff);

	//input_sync(ts->idev);

	return 0;
}
/*
 * add a pen up event
 */
static int wm97xx_ts_evt_release(wm97xx_ts_t* ts)
{
	input_report_abs(ts->idev, ABS_PRESSURE, 0);
	return 0;
}

/*
 * The touchscreen sample reader thread
 */
static int wm97xx_thread(void *_ts)
{
	wm97xx_ts_t *ts = (wm97xx_ts_t *)_ts;
	struct task_struct *tsk = current;
	int valid = 0;

	ts->rtask = tsk;

	/* set up thread context */
	daemonize();
	reparent_to_init();
	strcpy(tsk->comm, "ktsd");
	tsk->tty = NULL;

#if 1
	/* we will die when we receive SIGKILL */
	spin_lock_irq(&tsk->sigmask_lock);
	siginitsetinv(&tsk->blocked, sigmask(SIGKILL));
	recalc_sigpending(tsk);
	spin_unlock_irq(&tsk->sigmask_lock);
#else
	siginitsetinv(&tsk->blocked, sigmask(SIGINT) |
			sigmask(SIGTERM) | sigmask(SIGKILL) |
			sigmask(SIGUSR1));

	spin_lock_irq(&tsk->sigmask_lock);
	flush_signals(tsk);
	recalc_sigpending(tsk);
	spin_unlock_irq(&tsk->sigmask_lock);
#endif
	/* init is complete */
	complete(&ts->thread_init);

	/* touch reader loop */
	for (;;)
	{
		ts->restart = 0;

		if(signal_pending(tsk))
			break;

		if(pendown(ts))
		{
			switch (mode)
			{
				case 0:
					wm97xx_poll_touch(ts);
					valid = 1;
					break;
				case 1:
					wm97xx_poll_coord_touch(ts);
					valid = 1;
					break;
				case 2:
					wm97xx_cont_touch(ts);
					valid = 1;
					break;
			}
		} else {
			if (valid) {
				valid = 0;
				wm97xx_ts_evt_release(ts);
			}
		}

		set_task_state(tsk, TASK_INTERRUPTIBLE);
		if (HZ >= 100)
			schedule_timeout(HZ/50);   //old 100
		else
			schedule_timeout(1);

	}

	ts->rtask = NULL;
	complete_and_exit(&ts->thread_exit, 0);

	return 0;

}
#endif

static ssize_t wm97xx_read(struct file *filp, char *buf, size_t count, loff_t *l)
{
	wm97xx_ts_t* ts = (wm97xx_ts_t*)filp->private_data;
	unsigned long flags;
	TS_EVENT event;
	u16 x=0,y=0,p=0;

	/* are we still registered with AC97 layer ? */
	spin_lock_irqsave(&ts->lock, flags);

	if (!ts->is_registered)
	{
		spin_unlock_irqrestore(&ts->lock, flags);
		return -ENXIO;
	}

#ifdef DECT_TOUCH_INTERRUPT
	if(pen_int)
	{
		if (!wm97xx_coord_read_adc(ts, &x, &y, &p))
		{
					pen_int= 0;
					spin_unlock_irqrestore(&ts->lock, flags);
					return -EIO;
		}
		pen_int= 0;
	}
#else
	if(pendown(ts))
	{

		switch (mode)
		{
			case 0:
				if (!wm97xx_poll_read_adc(ts, WM97XX_ADCSEL_X, &x))
				{
					spin_unlock_irqrestore(&ts->lock, flags);
					return -EIO;
 				}
 				if (!wm97xx_poll_read_adc(ts, WM97XX_ADCSEL_Y, &y))
				{
					spin_unlock_irqrestore(&ts->lock, flags);
					return -EIO;
				}
 				if (pil && !five_wire)
				{
					if (!wm97xx_poll_read_adc(ts, WM97XX_ADCSEL_PRES, &p))
					{
						spin_unlock_irqrestore(&ts->lock, flags);
						return -EIO;
					}
				}
				else p = 0xffff;
				break;
			case 1:
				if (!wm97xx_coord_read_adc(ts, &x, &y, &p))
				{
					spin_unlock_irqrestore(&ts->lock, flags);
					return -EIO;
				}
				break;
			case 2:
					/* not support */
				break;

		}

	}
#endif


	x&=0xfff,y&=0xfff;

#if 0
	if((x==0xfff) || (y==0xfff)) {
		event.x=event.y=event.pressure=0;
	}
	else{
		event.x=x ,event.y=y ,event.pressure=p;
	}
#else
	event.x=x ,event.y=y ,event.pressure=p;
#endif

#if 0
	if(x && y)
		printk("event.x=:0x%x ,event.y=0x%x ,event.pressure=0x%x\n",event.x,event.y,event.pressure);
#endif

	spin_unlock_irqrestore(&ts->lock, flags);

	if(copy_to_user(buf, &event, sizeof(TS_EVENT)))
		return -EFAULT;

	return sizeof(TS_EVENT);
}


static int wm97xx_open(struct inode * inode, struct file * filp)
{
	wm97xx_ts_t* ts;
	unsigned long flags;
	u16 val;
	int minor = MINOR(inode->i_rdev);
#ifdef DECT_TOUCH_INTERRUPT
	int ret;
#endif

	if (minor != TS_MINOR)
		return -ENODEV;

	filp->private_data = ts = &wm97xx_ts;

	spin_lock_irqsave(&ts->lock, flags);

	/* are we registered with AC97 layer ? */
	if (!ts->is_registered)
	{
		spin_unlock_irqrestore(&ts->lock, flags);
		return -ENXIO;
	}

#ifdef DECT_TOUCH_INTERRUPT
		set_gpio_ctrl(GPIO_F4, GPIOMD_IN, GPIOPU_EN);

		/* PENDWON PIN OUTPUT */
		val = ts->codec->codec_read(ts->codec, 0x4C);
		val &=0xfff7;
		ts->codec->codec_write(ts->codec, 0x4C,val);

		/* PENDWON PIN ENABLE */
		val = ts->codec->codec_read(ts->codec, 0x56);
		val |=0x08;
		ts->codec->codec_write(ts->codec, 0x56,val);

		set_external_irq(GPIO_F4, EINT_LOW_LEVEL, GPIOPU_EN);
		if ((ret = request_irq(GPIO_F4, wm97xx_interrupt, 0, "AC97-touchscreen", &ts)) != 0)
		{
			err("can't get irq %d falling back to pendown polling\n", GPIO_F4);
			pen_int = 0;
		}
#endif

	/* start digitiser */
	val = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER2);
	ts->codec->codec_write(ts->codec, AC97_WM97XX_DIGITISER2,
		val | WM97XX_PRP_DET_DIG);

	spin_unlock_irqrestore(&ts->lock, flags);
	return 0;
}


/*
 * Start the touchscreen thread and
 * the touch digitiser.
*/
#ifdef DECT_TOUCH_EVENT
static int wm97xx_ts_input_open(struct input_dev *idev)
{
	wm97xx_ts_t *ts = (wm97xx_ts_t *) &wm97xx_ts;
	u32 flags;
	int ret, val;

	spin_lock_irqsave( &ts->lock, flags );
	if ( ts->use_count++ == 0 )
	{

#ifdef DECT_TOUCH_INTERRUPT
		set_gpio_ctrl(GPIO_F4, GPIOMD_IN, GPIOPU_EN);
		/* PENDWON PIN OUTPUT */
		val = ts->codec->codec_read(ts->codec, 0x4C);
		val &=0xfff7;
		ts->codec->codec_write(ts->codec, 0x4C,val);

		/* PENDWON PIN ENABLE */
		val = ts->codec->codec_read(ts->codec, 0x56);
		val |=0x08;
		ts->codec->codec_write(ts->codec, 0x56,val);

		set_external_irq(GPIO_F4, EINT_LOW_LEVEL, GPIOPU_NOSET);
		if ((ret = request_irq(GPIO_F4, wm97xx_interrupt, 0, "AC97-touchscreen", &ts)) != 0)
		{
			err("can't get irq %d falling back to pendown polling\n", GPIO_F4);
			pen_int = 0;
		}
#endif
		/* start touchscreen thread */
		ts->idev = idev;
		init_completion(&ts->thread_init);
		ret = kernel_thread(wm97xx_thread, ts, 0);
		if (ret >= 0)
			wait_for_completion(&ts->thread_init);

		/* start digitiser */
		val = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER2);
		ts->codec->codec_write(ts->codec, AC97_WM97XX_DIGITISER2,
								   val | WM97XX_PRP_DET_DIG);
	}
	spin_unlock_irqrestore( &ts->lock, flags );
	return 0;
}
#endif


#ifndef DECT_TOUCH_EVENT
static int wm97xx_release(struct inode * inode, struct file * filp)
{
	wm97xx_ts_t* ts = (wm97xx_ts_t*)filp->private_data;
	unsigned long flags;
	u16 val;

	spin_lock_irqsave(&ts->lock, flags);

#ifdef DECT_TOUCH_INTERRUPT

	free_irq(GPIO_F4, NULL);

	/* PENDWON PIN INPUPUT */
	val = ts->codec->codec_read(ts->codec, 0x4C);
	val |=0x08;
	ts->codec->codec_write(ts->codec, 0x4C,val);

		/* PENDWON PIN DISABLE */
	val = ts->codec->codec_read(ts->codec, 0x56);
	val |=0x08;
	ts->codec->codec_write(ts->codec, 0x56,val);
#endif

	/* stop digitiser */
	val = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER2);
	ts->codec->codec_write(ts->codec, AC97_WM97XX_DIGITISER2,
		val & ~WM97XX_PRP_DET_DIG);

	spin_unlock_irqrestore(&ts->lock, flags);
	return 0;
}

#else
/*
 * Kill the touchscreen thread and stop
 * the touch digitiser.
 */
static void wm97xx_ts_input_close(struct input_dev *idev)
{
	wm97xx_ts_t *ts = (wm97xx_ts_t *) &wm97xx_ts;
	u32 flags;
	int val;

	spin_lock_irqsave(&ts->lock, flags);
	if (--ts->use_count == 0)
	{
#ifdef DECT_TOUCH_INTERRUPT

		free_irq(GPIO_F4, NULL);
		/* PENDWON PIN OUTPUT */
		val = ts->codec->codec_read(ts->codec, 0x4C);
		val |=0x08;
		ts->codec->codec_write(ts->codec, 0x4C,val);

		/* PENDWON PIN DISABLE */
		val = ts->codec->codec_read(ts->codec, 0x56);
		val |=0x08;
		ts->codec->codec_write(ts->codec, 0x56,val);
#endif
		/* kill thread */
		init_completion(&ts->thread_exit);
		if (ts->rtask)
		{
			send_sig(SIGKILL, ts->rtask, 1);
			wait_for_completion(&ts->thread_exit);
		}

		/* stop digitiser */
		val = ts->codec->codec_read(ts->codec, AC97_WM97XX_DIGITISER2);
		ts->codec->codec_write(ts->codec, AC97_WM97XX_DIGITISER2,
				       val & ~WM97XX_PRP_DET_DIG);


	}

	spin_unlock_irqrestore(&ts->lock, flags);
}
#endif


static int __init wm97xx_ts_init_module(void)
{
	wm97xx_ts_t* ts = &wm97xx_ts;
	int ret;
	char proc_str[64];

#ifdef CONFIG_MACH_GP2X_DEBUG
	info("Wolfson WM9705/WM9712 Touchscreen Controller");
	info("Version %s  liam.girdwood@wolfsonmicro.com", WM_TS_VERSION);
#endif

	memset(ts, 0, sizeof(wm97xx_ts_t));

#ifdef DECT_TOUCH_EVENT
	wm97xx_input.name = "wm97xx touchscreen";
	wm97xx_input.open = wm97xx_ts_input_open;
	wm97xx_input.close = wm97xx_ts_input_close;
	/* absolute position */
	__set_bit(EV_ABS, wm97xx_input.evbit);
	/* absolute position XY */
	__set_bit(ABS_X, wm97xx_input.absbit);
	__set_bit(ABS_Y, wm97xx_input.absbit);
	__set_bit(ABS_PRESSURE, wm97xx_input.absbit);
	input_register_device(&wm97xx_input);
#else
	/* register our misc device */
	if ((ret = misc_register(&wm97xx_misc)) < 0)
	{
		err("can't register misc device");
		return ret;
	}
#endif

	spin_lock_init(&ts->lock);
	init_MUTEX(&ts->sem);

	/* register with the AC97 layer */
	ac97_register_driver(&wm9705_driver);
	ac97_register_driver(&wm9712_driver); /* wm97xx_probe call */

#ifdef CONFIG_PROC_FS
	/* register proc interface */
	sprintf(proc_str, "driver/%s", TS_NAME);
	if ((ts->wm97xx_ts_ps = create_proc_read_entry (proc_str, 0, NULL,wm97xx_read_proc, ts)) == 0)
		err("could not register proc interface /proc/%s", proc_str);
#endif

#ifdef CONFIG_PM
	if ((ts->pm = pm_register(PM_UNKNOWN_DEV, PM_SYS_UNKNOWN, wm97xx_pm_event)) == 0)
		err("could not register with power management");
#endif
	return 0;
}

static void wm97xx_ts_cleanup_module(void)
{

#ifdef CONFIG_PM
	pm_unregister (wm97xx_ts.pm);
#endif

	ac97_unregister_driver(&wm9705_driver);
	ac97_unregister_driver(&wm9712_driver);

#ifdef DECT_TOUCH_EVENT
	input_unregister_device(&wm97xx_input);
#endif
}

/* Module information */
MODULE_AUTHOR("Liam Girdwood, liam.girdwood@wolfsonmicro.com, www.wolfsonmicro.com");
MODULE_DESCRIPTION("WM9705/WM9712 Touch Screen / BMON Driver");
MODULE_LICENSE("GPL");

module_init(wm97xx_ts_init_module);
module_exit(wm97xx_ts_cleanup_module);

#ifndef MODULE

static int __init wm97xx_ts_setup(char *options)
{
	char *this_opt = options;

	if (!options || !*options)
		return 0;

	/* parse the options and check for out of range values */
	for(this_opt=strtok(options, ",");
	    this_opt; this_opt=strtok(NULL, ",")) {
		if (!strncmp(this_opt, "pil:", 4)) {
			this_opt+=4;
			pil = simple_strtol(this_opt, NULL, 0);
			if (pil < 0 || pil > 2)
				pil = 0;
			continue;
		}
		if (!strncmp(this_opt, "rpu:", 4)) {
			this_opt+=4;
			rpu = simple_strtol(this_opt, NULL, 0);
			if (rpu < 0 || rpu > 31)
				rpu = 0;
			continue;
		}
		if (!strncmp(this_opt, "pdd:", 4)) {
			this_opt+=4;
			pdd = simple_strtol(this_opt, NULL, 0);
			if (pdd < 0 || pdd > 15)
				pdd = 0;
			continue;
		}
		if (!strncmp(this_opt, "delay:", 6)) {
			this_opt+=6;
			delay = simple_strtol(this_opt, NULL, 0);
			if (delay < 0 || delay > 15)
				delay = 4;
			continue;
		}
		if (!strncmp(this_opt, "five_wire:", 10)) {
			this_opt+=10;
			five_wire = simple_strtol(this_opt, NULL, 0);
			if (five_wire < 0 || five_wire > 1)
				five_wire = 0;
			continue;
		}
		if (!strncmp(this_opt, "mode:", 5)) {
			this_opt+=5;
			mode = simple_strtol(this_opt, NULL, 0);
			if (mode < 0 || mode > 2)
				mode = 0;
			continue;
		}
	}
	return 1;
}

__setup("wm97xx_ts=", wm97xx_ts_setup);

#endif /* MODULE */
