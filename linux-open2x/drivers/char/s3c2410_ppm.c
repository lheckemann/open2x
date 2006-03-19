/*
 *  linux/drivers/char/mmsp2_ppm.c
 *
 *  Pulse Period Measurement(PPM)
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include "s3c2410_ppm.h"

static int irq = 18;
static int ppm_count=0;

U16     Bufflow[64], Buffhigh[64];
U8      Wlow,Whigh;
U8      Rlow,Rhigh;
U8      Flag_int;
U8      sphase, savesphase=3;
U8      Rcnt;
U8      cnt, flag;
U8      C_Addr, C_Data;

UINT     gPPM_IOBaseOffset;

static void PPM_GetPeriod
(
        PPMPERD         *pPeriod
)
{
//Brian ASSERT( Period == NULL );
        //MES_ASSERT( pPeriod == NULL );

        pPeriod->OverflowHigh   = PPMSTATUS & 0x0008;
        pPeriod->High                   = PPMHIGHPERD;
        pPeriod->OverflowLow    = PPMSTATUS & 0x0010;
        pPeriod->Low                    = PPMLOWPERD;
}


static int ppm_open(struct inode *inode, struct file *file)
{
	if(ppm_count==0) {
		ppm_count = 1;
		return 0;
	}
	else
		return -EBUSY;
}

static int ppm_release(struct inode *inode, struct file *file)
{
	ppm_count = 0;
	return 0;
}

static int ppm_ioctl(struct inode *inode, struct file *file,
                     unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
/*     		case Button_Power:
                {      
			ret = 1;
			printk("[Power Buttonn");
       			break;
                }
  	     	case Button_VOL_UP:
                {      
			printk("[Vol_Up_Button]\n");
       			break;
                }
       		case Button_VOL_DOWN:
                {      
			printk("[Vol_Down_Button]\n");
       			break;
                }
       		case Button_CH_UP:
                {      
			printk("[Ch_Up Button]\n");
       			break;
                }
       		case Button_CH_DOWN:
                {      
			printk("[Ch_Down Button]\n");
       			break;
                }
*/
		default:
//			ret = -EINVAL;
			break;
	}
//	return ret;
	return 0;
}

static void ppm_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
        PPMPERD         Period;

	//For check pulse
        static U16      step=0;

	//For decode
	static U8 buff[5],      bcnt=0;
	U8      i, Data_code;

        PPM_GetPeriod( &Period );


/**************************
 *       check pulse      *
 **************************/
        switch(step)
        {
        case 0:                                                                 // Start code check
                if      ((Period.Low==0) || (Period.Low>NUM_FIRST_LOW) && (Period.High>NUM_FIRST_HIGH))
                {
                        PPMCTRL |= 0x04;                                // Overflow interrupt Enable
                        Bufflow[Wlow++]=Period.Low;
                        Buffhigh[Whigh++]=Period.High;
                        Wlow &= 0x3F;
                        Whigh &= 0x3F;
                        Rcnt++;
                        step++;
                }
                break;
        case 1:
                if ((Period.OverflowHigh)||(Period.High > NUM_1_HIGH))                  // End check(if High Overflow)
                {
                        PPMCTRL &= ~0x04;                               // Overflow interrupt Enable
                        PPMSTATUS =     0x07;
                        Bufflow[Wlow++]=Period.Low;
                        Buffhigh[Whigh++]=Period.High;
                        Wlow &= 0x3F;
                        Whigh &= 0x3F;
                        Rcnt++;
                        step--;
                }
                else
                {
                        Bufflow[Wlow++]=Period.Low;
                        Buffhigh[Whigh++]=Period.High;
                        Wlow &= 0x3F;
                        Whigh &= 0x3F;
                        Rcnt++;
                }
                break;
        default:
                break;
        }
        PPMSTATUS =     0x07;
/**************************
 *       now decode       *
 **************************/
        switch(sphase)
        {
        case(PPM_START_CODE_DETECTION):                                 // PPM_START_CODE_DETECTION
//          WriteDebugString("1\n");
                if(     (Bufflow[Rlow]==0) || (Bufflow[Rlow]>NUM_FIRST_LOW) )
                {
                        if(Buffhigh[Rhigh] > NUM_FIRST_HIGH)    // Start
                                {
                                bcnt=0;
//                                      WriteDebugString("\n[H][%04x:%04x]\n",Bufflow[Rlow],Buffhigh[Rhigh]);
                                sphase=PPM_DATA_INPUT;
                                }
                }

                Rlow++;
                Rhigh++;
                PPMSTATUS =     0x07;
                Rlow &= 0x3F;
                Rhigh &= 0x3F;
                Rcnt--;
                break;

        case(PPM_DATA_INPUT):                                   // PPM_DATA_INPUT
//      WriteDebugString("2\n");
                for(i=0; i<8; i++)
                {
                  if ((Buffhigh[Rhigh]==0) && (Bufflow[Rlow]>NUM_LOW))  //End check
                  {
/*
                        Rlow++;
                        Rhigh++;
                        PPMSTATUS =     0x07;
                        Rlow &= 0x3F;
                        Rhigh &= 0x3F;
                        Rcnt--;
*/
                        sphase=2;
                        break;
                  }
                  else
                  {
                        buff[bcnt] = buff[bcnt] << 1;

                        if(Buffhigh[Rhigh] < NUM_0_HIGH )       // '0'
                        {
                                buff[bcnt] &=~0x01;
//                              WriteDebugString("[B%1d(0):%04x:%04x] ", i, Bufflow[Rlow],Buffhigh[Rhigh]);
                        }else if((Buffhigh[Rhigh] > NUM_0_HIGH) && (Buffhigh[Rhigh] < NUM_1_HIGH))// '1'
                        {
                                buff[bcnt] |=0x01;
//                      WriteDebugString("[B%1d(1):%04x:%04x] ", i, Bufflow[Rlow],Buffhigh[Rhigh]);
                        }
                        //Delay(10);
			udelay(10);
/*
                        if((i==3)||(i==7))
                                WriteDebugString("\n");
*/
                        Rlow++;
                        Rhigh++;
                        Rlow &= 0x3F;
                        Rhigh &= 0x3F;
                        Rcnt--;
                  }
                } //for
        if(bcnt==3)
        {
                        C_Addr=buff[0];
                        C_Data=buff[2];
//              bcnt=0;
                sphase++;
        }
        else
                bcnt++;
        break;


        case(2):
//      WriteDebugString("3\n");
                if((C_Addr==Custom_Code_0) || (C_Addr==Custom_Code_1))
                {
                        sphase++;
                        break;
                }
                else
                {
                        printk("Remocon don't Find\n");
                        printk("[Custom Code %02x]\n",C_Addr);

                        C_Addr=0;
                        C_Data=0;
                Rlow++;
                Rhigh++;
                Rlow &= 0x3F;
                Rhigh &= 0x3F;
                Rcnt--;
                        sphase=PPM_START_CODE_DETECTION;
                        break;
                }

        case(3):
//      WriteDebugString("4\n");
                switch(C_Data)
                {
                case(Button_Power):
                        if(!cnt--)
                        flag=1;
                        printk("[POWER Button]\n");
                        break;
                case(Button_VOL_UP):
                        printk("[VOL_UP Button]\n");
                        break;

                case(Button_VOL_DOWN):
                        printk("[VOL_DOWN Button]\n");
                        break;

                case(Button_CH_UP):
                        printk("[CH_UP Button]\n");
                        break;

                case(Button_CH_DOWN):
                        printk("[CH_DOWN Button]\n");
                        break;
                default:
                        break;
                }

                Rlow++;
                Rhigh++;
                Rlow &= 0x3F;
                Rhigh &= 0x3F;
                Rcnt--;
                sphase=PPM_START_CODE_DETECTION;
                break;
        default:
//      WriteDebugString("5\n");
                break;
        }


}

static struct file_operations ppm_fops = {
	owner:		THIS_MODULE,
	ioctl:		ppm_ioctl,
	open:		ppm_open,
	release:	ppm_release,
};

static struct miscdevice ppm_miscdev = {
	PPM_MINOR, "ppm", &ppm_fops
};


static int __init ppm_init(void)
{
	int ret;

	gPPM_IOBaseOffset = PPM_VIO_BASE;
	PPMCTRL = 0x8000 | 0x4000;	// PPM Enable | PPM Input Polarity Bypass
	PPMSTATUS = 0x7;		// Overflow Clear, Falling edge detect, rising edge detect

        Rlow=0;
        Rhigh=0;
        Wlow=0;
        Whigh=0;
        Rcnt=0;
        sphase = 0;
        cnt = 1;
        flag = 0;
        C_Addr=0;
        C_Data=0;

	ret = misc_register(&ppm_miscdev);

	printk("PPM Module init!\n");
        if (ret) {
                printk(KERN_ERR "ppm: can't misc_register on minor=%d\n", PPM_MINOR);
	        return ret;
        }else {
		printk("PPM MINIOR : %d\n",PPM_MINOR);
	}

        ret = request_irq(irq, ppm_interrupt, SA_INTERRUPT, "ppm", NULL);
        if(ret) {
                printk(KERN_ERR "wdt: IRQ %d is not free.\n", irq);
	        misc_deregister(&ppm_miscdev);
		return ret;
        }else {
		printk("PPM IRQ Number : %d\n",irq);
	}

	return 0;
}

static void __exit ppm_exit(void)
{
	misc_deregister(&ppm_miscdev);
	free_irq(irq, NULL);
}

module_init(ppm_init);
module_exit(ppm_exit);
