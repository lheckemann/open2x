#ifndef __MMSP2PWRMAN_H__
#define __MMSP2PWRMAN_H__

#include "../sound/typedef.h"

typedef struct {
    volatile U16 PWRMODE;		  // 0x0900 + IO_BASE
    volatile U16 CLKCHGST;         // 0x0902 + IO_BASE
    volatile U16 SYSCLKEN;         // 0x0904 + IO_BASE
    volatile U16 Reserved0;
    volatile U16 COMCLKEN;         // 0x0908 + IO_BASE
    volatile U16 VGCLKEN;          // 0x090A + IO_BASE
    volatile U16 ASCLKEN;          // 0x090C + IO_BASE
    volatile U16 Reverved1;
    volatile U16 FPLLSETV;         // 0x0910 + IO_BASE
    volatile U16 FPLLVSET;         // 0x0912 + IO_BASE
    volatile U16 UPLLSETV;         // 0x0914 + IO_BASE
    volatile U16 UPLLVSET;         // 0x0916 + IO_BASE
    volatile U16 APLLSETV;         // 0x0918 + IO_BASE
    volatile U16 APLLVSET;         // 0x091A + IO_BASE
    volatile U16 SYSCSET;          // 0x091C + IO_BASE
    volatile U16 ESYSCSET;         // 0x091E + IO_BASE
    volatile U16 UIRMCSET;         // 0x0920 + IO_BASE
    volatile U16 AUDICSET;         // 0x0922 + IO_BASE
    volatile U16 DISPCSET;         // 0x0924 + IO_BASE
    volatile U16 IMAGCSET;         // 0x0926 + IO_BASE
    volatile U16 URT0CSET;         // 0x0928 + IO_BASE
    volatile U16 URT1CSET;         // 0x092A + IO_BASE
    volatile U16 A940TMODE;        // 0x092C + IO_BASE
    volatile U16 SPDICSET;         // 0x092e + IO_BASE
} PMR_REG;

typedef struct {
	U16 P;
  	U16 M;
	U16 S;
} tPMS;

typedef struct {
	U16 EXTCLK_SOURCE;
	U16 EXTCLK_DIVIDER;
	U16 EXTCLK_POL;
	U16 EXTCLK_DLAY;
} tExtClkInfo;

typedef struct {
	U16 SDCLK_POL;
	U16 SDCLK_DELAY;
} tSDRAMClkInfo;

typedef struct {
	U16 UCLK_SOURCE;
	U16 UCLK_DIVIDER;
} tUCLKInfo;

typedef struct {
	U16 SDMSCLK_SOURCE;
	U16 SDMSCLK_DIVIDER;
} tSDMSClkInfo;

typedef struct {
	U16 ACLK_SOURCE;
	U16 ACLK_DIVIDER;
} tACLKInfo;

typedef struct {
	U16 SPDIFINCLK_SOURCE;
	U16 SPDIFINCLK_DIVIDER;
	U16 SPDIFOUTCLK_SOURCE;
	U16 SPDIFOUTCLK_DIVIDER;
} tSPDIFClkInfo;

typedef struct {
	U16 DISPCLK_SOURCE;
	U16 DISPCLK_DIVIDER;
	U16 DISPCLK_POL;
} tDispClkInfo;

typedef struct {
	U16 ISPCLK_SOURCE;
	U16 ISPCLK_DIVIDER;
} tISPClkInfo;

typedef struct {
	U16 UART0CLK_SOURCE;
	U16 UART0CLK_DIVIDER;
} tUART0ClkInfo;

typedef struct {
	U16 UART1CLK_SOURCE;
	U16 UART1CLK_DIVIDER;
} tUART1ClkInfo;

typedef struct {
	U16 UART2CLK_SOURCE;
	U16 UART2CLK_DIVIDER;
} tUART2ClkInfo;

typedef struct {
	U16 UART3CLK_SOURCE;
	U16 UART3CLK_DIVIDER;
} tUART3ClkInfo;

typedef struct {
	U16 EXTCLK;
	U16 PCLK_ADC;
	U16 PCLK_PWM;
	U16 BCLK_DMA;
	U16 BCLK_DMA_GATE_DISABLE;
	U16 TCLK_TIMER;
	U16 PCLK_CORP;
	U16 PCLK_GPIO;
	U16 PCLK_CORP_GATE_DISABLE;
	U16 BCLK_MEMC;
	U16 BCLK_MEMA;
	U16 A940CLK;
} tSysClkCtrl;

typedef struct {
	U16 SSPCLK;
	U16 PCLK_I2C;
	U16 UART3CLK;
	U16 UART2CLK;
	U16 UART1CLK;
	U16 UART0CLK;
	U16 PCLK_OWM;
	U16 IRDACLK;
	U16 UDCCLK;
	U16 UHCCLK;
} tCommClkCtrl;

typedef struct {
	U16 BCLK_DBLCK;
	U16 BCLK_MPEGIF;
	U16 PCLK_DECSS;
	U16 MECLK;
	U16 BCLK_REF;
	U16 BCLK_RVLD;
	U16 BCLK_QGMC;
	U16 BCLK_MPMC;
	U16 BCLK_PDF; //Pred-Q/DCT/Format converter
	U16 BCLK_HUFF;
	U16 MPGCLK_GATE_DISABLE;
	U16 BCLK_MPEG;
	U16 ISPCLK;
	U16 BCLK_GRAPH;
	U16 BCLK_SCALE;
	U16 DISPCLK;
} tVidGrpClkCtrl;

typedef struct {
	U16 PCLK_CDROM;
	U16 BCLK_IDEIF;
	U16 MSCLK;
	U16 PCLK_SD;
	U16 SPDIFOUTCLK;
	U16 SPDIFINCLK;
	U16 I2SCLK;
	U16 AC97CLK;
} tAudStrClkCtrl;

#endif // __MMSP2PWRMAN_H__
