/*-----------------------------------------------------------------------------
	ac97 test program header
	file name : AC97.h
	by Nam Tae Woo
-------------------------------------------------------------------------------
	2003/10/16		modify for common environmemt
	2004/02/11		modify filename : test_ac97.h  ->  AC97test.h
-----------------------------------------------------------------------------*/
#ifndef __AC97_H__
#define __AC97_H__


#define HEX_1M1    				0x00100000
#define HEX_512K1  				0x00080000

#define PA_ARM940_BASE1         0x03000000
#define DUALCPU_MEM_SIZE1	    (HEX_1M1)  							// 1M

#define PA_FB0_BASE1    		(PA_ARM940_BASE1+DUALCPU_MEM_SIZE1)  	// 0x0300_0000+1M 2.5M(0x0028_0000)
#define FB0_MEM_SIZE1   		(HEX_1M1+HEX_1M1+HEX_512K1) 			// 2.5M

#define PA_FB1_BASE1    		(PA_FB0_BASE1+FB0_MEM_SIZE1)  	// 0x0300_0000+1M+2.5M 2.5M(0x0028_0000)
#define FB1_MEM_SIZE1   		(HEX_1M1+HEX_1M1+HEX_512K1)    	// 2.5M

#define PA_SOUND_DMA_BASE1  	(PA_FB1_BASE1+FB1_MEM_SIZE1) 	// 0x0300_0000+1M+2.5M+2.5M 0.5M(0x0008_0000)

/* GP2X SOUND LOGO NANDFLASH  WAV BASE */
#define WAV_BASE				0x1A0000


//#define	IO_BASE				0xC0000000	// I/O base address
#define LOCKTIMEREG			(*((volatile U16*)(0x0C1A + IO_BASE)))
#define GPIOL_ALTFNSELL		(*((volatile U16*)(0x1036 + IO_BASE)))
#define GPIOL_ALTFNSELH		(*((volatile U16*)(0x1056 + IO_BASE)))
#define SYSCLKENREG         (*((volatile U16*)(0x0904 + IO_BASE)))
#define FPLLSETVREG         (*((volatile U16*)(0x0910 + IO_BASE)))
#define ASCLKENREG          (*((volatile U16*)(0x090C + IO_BASE)))
#define APLLSETVREG         (*((volatile U16*)(0x0918 + IO_BASE)))
#define AUDICSETREG         (*((volatile U16*)(0x0922 + IO_BASE)))

#define	DMA0_BASE			(0x0200+IO_BASE)
#define	DMACOM0REG			0x0000		// DMA command register 0
#define	DMACOM1REG			0x0002		// DMA command register 1
#define	DMACOM2REG			0x0004		// DMA command register 2
#define	DMACONSREG			0x0006		// DMA command control/status register
#define	SRCADDRREG			0x0008
#define	SRCLADRREG			0x0008		// DMA source low address register
#define	SRCHADRREG			0x000A		// DMA source high address register
#define	TRGADDRREG			0x000C		// DMA target  address register
#define	TRGLADRREG			0x000C		// DMA target low address register
#define	TRGHADRREG			0x000E		// DMA target high address register
//#define	SRCPENDREG			(*((volatile U32*)(0x0800+IO_BASE)))	// source pending reg. (32-bit)
//#define	INTPENDREG			(*((volatile U32*)(0x0810+IO_BASE)))	// interrupt pending reg. (32-bit)

void PlayerSoundLogo(int flag);

#endif
