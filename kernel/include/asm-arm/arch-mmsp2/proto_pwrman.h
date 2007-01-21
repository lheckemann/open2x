#ifndef __MMSP2PWRMAN_H__
#define __MMSP2PWRMAN_H__

#include <asm/arch/wincetype.h>


typedef struct {
	U16 S	: 2;
	U16 P	: 6;
  	U16 M	: 8;
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
	U16 I2S_CLK;
	U16 AC97_CLK;
} tAudStrClkCtrl;

#define FPLL 1
#define UPLL 2
#define APLL 3

void 	PMR_Initialize        ( void );
void 	PMR_Close             ( void );

void	PMR_UPLLDown          ( U16 );
void	PMR_APLLDown          ( U16 );
void	PMR_SetUPLLOpMode          ( U16 Mode );
void	PMR_SetAPLLOpMode          ( U16 Mode );
U16		PMR_GetClkChangeStatus( void );

void    PMR_SetA920FDiv		  ( U16 Divder );
U16		PMR_GetA920FDiv		  ( void );
void	PMR_SetA940FDiv       ( U16 Divder );
U16		PMR_GetA940FDiv       ( void );
void	PMR_SetDCLKDiv        ( U16 Divder );
U16		PMR_GetDCLKDiv        ( void );
void	PMR_SetExtClk         ( tExtClkInfo *pEXTCLKINFO );
void	PMR_GetExtClk    ( tExtClkInfo *pEXTCLKINFO );
void	PMR_SetSDRAMClk       ( tSDRAMClkInfo *pSDRAMCLKINFO );
void	PMR_GetSDRAMClk (tSDRAMClkInfo *pSDRAMCLKINFO  );
void	PMR_SetUSBIrDAClk     ( tUCLKInfo *pUCLKINFO );
void	PMR_GetUSBIrDAClk   ( tUCLKInfo *pUCLKINFO );
void	PMR_SetSDMSClk        ( tSDMSClkInfo *pSDMSCLKINFO );
void	PMR_GetSDMSClk   ( tSDMSClkInfo *pSDMSINFO );
void 	PMR_SetAC97I2SClk     ( tACLKInfo *pACLKINFO );
void	PMR_GetAC97I2SClk   ( tACLKInfo *pACLKINFO );
void 	PMR_SetSPDIFClk       ( tSPDIFClkInfo *pSPDIFCLKINFO );
void	PMR_GetSPDIFClk ( tSPDIFClkInfo *pSPDIFCLKINFO );
void 	PMR_SetDispClk        ( tDispClkInfo *pDISPCLKINFO );
void	PMR_GetDispClk   ( tDispClkInfo *pDISPCLKINFO );
void 	PMR_SetISPClk         ( tISPClkInfo *pISPCLKINFO );
void	PMR_GetISPClk     ( tISPClkInfo *pISPCLKINFO );
void 	PMR_SetUART0Clk       ( tUART0ClkInfo *pUART0CLKINFO );
void	PMR_GetUART0Clk ( tUART0ClkInfo *pUART0CLKINFO );
void	PMR_SetUART1Clk       ( tUART1ClkInfo *pUART1CLKINFO );
void	PMR_GetUART1Clk ( tUART1ClkInfo *pUART1CLKINFO );
void	PMR_SetUART2Clk       ( tUART2ClkInfo *pUART2CLKINFO );
void	PMR_GetUART2Clk ( tUART2ClkInfo *pUART2CLKINFO );
void	PMR_SetUART3Clk       ( tUART3ClkInfo *pUART3CLKINFO );
void	PMR_GetUART3Clk ( tUART3ClkInfo *pUART3CLKINFO );
void 	PMR_SetPWRMode        ( U16 Mode );
void 	PMR_SetAutoStopMode   ( U16 Mode );
void	PMR_SetA940Mode       ( U16 Mode );
void	PMR_SetSystemClkCtrl  ( tSysClkCtrl *pSYSCLKCTRL );
void	PMR_GetSystemClkCtrl ( tSysClkCtrl *pSYSCLKCTRL );
void 	PMR_SetCommClkCtrl    ( tCommClkCtrl *pCOMMCLKCTRL );
void	PMR_GetCommClkCtrl ( tCommClkCtrl *pCOMMCLKCTRL );
void	PMR_SetVideoGraphClkCtrl ( tVidGrpClkCtrl *pVIDGRPCLKCTRL );
void	PMR_GetVideoGraphClkCtrl (tVidGrpClkCtrl *pVIDGRPCLKCTRL);
void	PMR_SetAudioStorageClkCtrl ( tAudStrClkCtrl *pAUDSTRCLKCTRL );
void	PMR_GetAudioStorageClkCtrl ( tAudStrClkCtrl *pAUDSTRCLKCTRL );

#endif // __MMSP2PWRMAN_H__
