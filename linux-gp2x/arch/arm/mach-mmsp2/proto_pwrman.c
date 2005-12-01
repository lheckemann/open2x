#include <linux/config.h>
#include <linux/module.h>

#include <asm/arch/mmsp20.h>
#include <asm/arch/proto_pwrman.h>


MMSP20_CLOCK_POWER *pPMR_REG;

void 	PMR_Initialize        (  )
{
	pPMR_REG = MMSP20_GetBase_CLOCK_POWER();
}

void 	PMR_Close             ( void ) {}


void	PMR_UPLLDown          ( U16 Down )
{
	if(Down == 1)
		pPMR_REG->PWMODEREG |= UPLLDOWN;
	else
		pPMR_REG->PWMODEREG &= ~UPLLDOWN;	
}

void	PMR_APLLDown          ( U16 Down )
{
	if(Down == 1)
		pPMR_REG->PWMODEREG |= APLLDOWN;
	else
		pPMR_REG->PWMODEREG &= ~APLLDOWN;	
}

U16	PMR_GetClkChangeStatus( void )
{
	return pPMR_REG->CLKCHGSTREG;
}

void    PMR_SetA920FDiv		  ( U16 Divder )
{
	
	U16 temp;

	temp = pPMR_REG->SYSCSETREG & (~0x3);
	pPMR_REG->SYSCSETREG = (Divder & 0x0007) | temp; 
}

U16	PMR_GetA920FDiv		  ( void )
{
	return (pPMR_REG->SYSCSETREG & 0x0007); 
}

void	PMR_SetA940FDiv       ( U16 Divder )
{
	
	U16 temp;
	temp = (U16)( pPMR_REG->SYSCSETREG & (~(0x7 << 3)) );
	pPMR_REG->SYSCSETREG = ((Divder & 0x0007) << 3) | temp; 
}

U16	PMR_GetA940FDiv       ( void )
{
	return ((pPMR_REG->SYSCSETREG >> 3) & 0x0007); 
}

void	PMR_SetDCLKDiv        ( U16 Divder )
{
	
	U16 temp;
	temp = (U16)( pPMR_REG->SYSCSETREG & (~(0x7 << 6)) );
	pPMR_REG->SYSCSETREG = ((Divder & 0x0007) << 6) | temp; 
}

U16	PMR_GetDCLKDiv        ( void )
{
	return ((pPMR_REG->SYSCSETREG >> 6) & 0x0007); 
}
	
void	PMR_SetExtClk         ( tExtClkInfo *pEXTCLKINFO )
{
	
	U16 temp;
	temp = pPMR_REG->ESYSCSETREG & 0x000f;
	pPMR_REG->ESYSCSETREG = temp | (pEXTCLKINFO->EXTCLK_SOURCE << 14) | (pEXTCLKINFO->EXTCLK_DIVIDER << 8) |
	                      (pEXTCLKINFO->EXTCLK_POL << 7) | (pEXTCLKINFO->EXTCLK_DLAY << 4);
}

void PMR_GetExtClk     ( tExtClkInfo *pEXTCLKINFO )
{
	
	pEXTCLKINFO->EXTCLK_SOURCE = (pPMR_REG->ESYSCSETREG >> 14) & 0x3;
	pEXTCLKINFO->EXTCLK_DIVIDER = (pPMR_REG->ESYSCSETREG >> 8) & 0x3f;
	pEXTCLKINFO->EXTCLK_POL = (pPMR_REG->ESYSCSETREG >> 7) & 0x1;
	pEXTCLKINFO->EXTCLK_DLAY = (pPMR_REG->ESYSCSETREG >> 4) & 0x7;
}

void	PMR_SetSDRAMClk       ( tSDRAMClkInfo *pSDRAMCLKINFO )
{
	
	U16 temp;

	printk("*[%s:%d] : \n",__FUNCTION__,__LINE__);
	temp = pPMR_REG->ESYSCSETREG & 0xfff0;
	pPMR_REG->ESYSCSETREG = temp | (pSDRAMCLKINFO->SDCLK_POL << 3) | (pSDRAMCLKINFO->SDCLK_DELAY );
}

void PMR_GetSDRAMClk (tSDRAMClkInfo *pSDRAMCLKINFO  )
{
	pSDRAMCLKINFO->SDCLK_POL = (pPMR_REG->ESYSCSETREG >> 3) & 0x1;
	pSDRAMCLKINFO->SDCLK_POL = pPMR_REG->ESYSCSETREG & 0x7;
}

void	PMR_SetUSBIrDAClk     ( tUCLKInfo *pUCLKINFO )
{
	
	U16 temp;
	
	temp = pPMR_REG->UIRMCSETREG & 0xff00;
	pPMR_REG->UIRMCSETREG = temp | (pUCLKINFO->UCLK_SOURCE << 6) | (pUCLKINFO->UCLK_DIVIDER);
}

void PMR_GetUSBIrDAClk   ( tUCLKInfo *pUCLKINFO )
{

	pUCLKINFO->UCLK_SOURCE = (pPMR_REG->UIRMCSETREG >> 6) & 0x3;
	pUCLKINFO->UCLK_DIVIDER = (pPMR_REG->UIRMCSETREG) & 0x3f;
	
}

void	PMR_SetSDMSClk        ( tSDMSClkInfo *pSDMSCLKINFO )
{
	
	U16 temp;
	temp = pPMR_REG->UIRMCSETREG & 0x00ff;
	pPMR_REG->UIRMCSETREG = temp | (pSDMSCLKINFO->SDMSCLK_SOURCE << 14) | (pSDMSCLKINFO->SDMSCLK_DIVIDER << 8);
}

void PMR_GetSDMSClk   ( tSDMSClkInfo *pSDMSINFO )
{
	pSDMSINFO->SDMSCLK_SOURCE = (pPMR_REG->UIRMCSETREG >> 14) & 0x3;
	pSDMSINFO->SDMSCLK_DIVIDER = (pPMR_REG->UIRMCSETREG >> 8) & 0x3f;
}

void 	PMR_SetAC97I2SClk     ( tACLKInfo *pACLKINFO )
{
	pPMR_REG->AUDICSETREG = (pACLKINFO->ACLK_SOURCE << 6) | (pACLKINFO->ACLK_DIVIDER);
}

void PMR_GetAC97I2SClk   ( tACLKInfo *pACLKINFO )
{
	pACLKINFO->ACLK_SOURCE = (pPMR_REG->AUDICSETREG >> 6) & 0x3;
	pACLKINFO->ACLK_DIVIDER = pPMR_REG->AUDICSETREG  & 0x3f;
}

void 	PMR_SetSPDIFClk       ( tSPDIFClkInfo *pSPDIFCLKINFO )
{
	pPMR_REG->SPDICSETREG = (pSPDIFCLKINFO->SPDIFINCLK_SOURCE << 14) | 
	                     (pSPDIFCLKINFO->SPDIFINCLK_DIVIDER << 8) |
	                     (pSPDIFCLKINFO->SPDIFOUTCLK_SOURCE << 6) |
	                     (pSPDIFCLKINFO->SPDIFOUTCLK_DIVIDER);
}

void PMR_GetSPDIFClk ( tSPDIFClkInfo *pSPDIFCLKINFO )
{
	pSPDIFCLKINFO->SPDIFINCLK_SOURCE = (pPMR_REG->SPDICSETREG >> 14) & 0x3;
	pSPDIFCLKINFO->SPDIFINCLK_DIVIDER = (pPMR_REG->SPDICSETREG >> 8) & 0x3f;
	pSPDIFCLKINFO->SPDIFOUTCLK_SOURCE = (pPMR_REG->SPDICSETREG >> 6) & 0x3;
	pSPDIFCLKINFO->SPDIFOUTCLK_DIVIDER = (pPMR_REG->SPDICSETREG) & 0x3f;
}

void 	PMR_SetDispClk ( tDispClkInfo *pDISPCLKINFO )
{
	pPMR_REG->DISPCSETREG = (pDISPCLKINFO->DISPCLK_SOURCE << 14) | 
	                     (pDISPCLKINFO->DISPCLK_DIVIDER << 8) |
	                     (pDISPCLKINFO->DISPCLK_POL << 7);
}

void PMR_GetDispClk   ( tDispClkInfo *pDISPCLKINFO )
{
	pDISPCLKINFO->DISPCLK_SOURCE = (pPMR_REG->DISPCSETREG >> 14) & 0x3;
	pDISPCLKINFO->DISPCLK_DIVIDER = (pPMR_REG->DISPCSETREG >> 8) & 0x3f;
	pDISPCLKINFO->DISPCLK_POL = (pPMR_REG->DISPCSETREG >> 7);
}

void 	PMR_SetISPClk         ( tISPClkInfo *pISPCLKINFO )
{
	pPMR_REG->IMAGCSETREG = (pISPCLKINFO->ISPCLK_SOURCE << 14) | 
	                     (pISPCLKINFO->ISPCLK_DIVIDER << 8);
}

void PMR_GetISPClk     ( tISPClkInfo *pISPCLKINFO )
{
	pISPCLKINFO->ISPCLK_SOURCE =  (pPMR_REG->IMAGCSETREG >> 14) & 0x3;
	pISPCLKINFO->ISPCLK_DIVIDER = (pPMR_REG->IMAGCSETREG >> 8) & 0x3f;
}

void 	PMR_SetUART0Clk       ( tUART0ClkInfo *pUART0CLKINFO )
{
	pPMR_REG->URTCSETREG[0] = (pPMR_REG->URTCSETREG[0] & 0xff00) | (pUART0CLKINFO->UART0CLK_SOURCE << 6) | 
	                     (pUART0CLKINFO->UART0CLK_DIVIDER);
}

void PMR_GetUART0Clk ( tUART0ClkInfo *pUART0CLKINFO )
{
	pUART0CLKINFO->UART0CLK_SOURCE =  (pPMR_REG->URTCSETREG[0] >> 6) & 0x3;
	pUART0CLKINFO->UART0CLK_DIVIDER = (pPMR_REG->URTCSETREG[0]) & 0x3f;
}

void 	PMR_SetUART1Clk       ( tUART1ClkInfo *pUART1CLKINFO )
{
	pPMR_REG->URTCSETREG[0] = (pPMR_REG->URTCSETREG[0] & 0x00ff) | (pUART1CLKINFO->UART1CLK_SOURCE << 14) | 
	                     (pUART1CLKINFO->UART1CLK_DIVIDER << 8);
}

void PMR_GetUART1Clk ( tUART1ClkInfo *pUART1CLKINFO )
{
	
	pUART1CLKINFO->UART1CLK_SOURCE =  (pPMR_REG->URTCSETREG[0] >> 14) & 0x3;
	pUART1CLKINFO->UART1CLK_DIVIDER = (pPMR_REG->URTCSETREG[0] >> 8) & 0x3f;
}

void 	PMR_SetUART2Clk       ( tUART2ClkInfo *pUART2CLKINFO )
{
	pPMR_REG->URTCSETREG[1] = (pPMR_REG->URTCSETREG[1] & 0xff00) | (pUART2CLKINFO->UART2CLK_SOURCE << 6) | 
	                     (pUART2CLKINFO->UART2CLK_DIVIDER);
}

void PMR_GetUART2Clk ( tUART2ClkInfo *pUART2CLKINFO )
{
	pUART2CLKINFO->UART2CLK_SOURCE =  (pPMR_REG->URTCSETREG[1] >> 6) & 0x3;
	pUART2CLKINFO->UART2CLK_DIVIDER = (pPMR_REG->URTCSETREG[1]) & 0x3f;
}

void 	PMR_SetUART3Clk       ( tUART3ClkInfo *pUART3CLKINFO )
{
	pPMR_REG->URTCSETREG[1] = (pPMR_REG->URTCSETREG[1] & 0x00ff) | (pUART3CLKINFO->UART3CLK_SOURCE << 14) | 
	                     (pUART3CLKINFO->UART3CLK_DIVIDER << 8);
}

void PMR_GetUART3Clk ( tUART3ClkInfo *pUART3CLKINFO )
{
	pUART3CLKINFO->UART3CLK_SOURCE =  (pPMR_REG->URTCSETREG[1] >> 14) & 0x3;
	pUART3CLKINFO->UART3CLK_DIVIDER = (pPMR_REG->URTCSETREG[1] >> 8) & 0x3f;
}

void 	PMR_SetPWRMode        ( U16 Mode )
{
	U16 temp;
	temp = pPMR_REG->PWMODEREG & 0xfff0;
	pPMR_REG->PWMODEREG = (Mode | temp);
}

void 	PMR_SetAutoStopMode   ( U16 Mode )
{
	
	U16 temp;
	temp = (U16)( pPMR_REG->PWMODEREG & (~(1 << 4)) );
	pPMR_REG->PWMODEREG = (Mode | temp);
}

void	PMR_SetA940Mode       ( U16 Mode )
{
	pPMR_REG->ARM940TMODEREG = Mode;
}
	
void	PMR_SetSystemClkCtrl  ( tSysClkCtrl *pSYSCLKCTRL )
{
	pPMR_REG->SYSCLKENREG = (pSYSCLKCTRL->EXTCLK << 14) | 
							(pSYSCLKCTRL->PCLK_ADC << 13) |
							(pSYSCLKCTRL->PCLK_PWM << 12) | 
							(pSYSCLKCTRL->BCLK_DMA << 9) |
							(pSYSCLKCTRL->BCLK_DMA_GATE_DISABLE << 8) | 
							(pSYSCLKCTRL->TCLK_TIMER << 7) |
							(pSYSCLKCTRL->PCLK_CORP << 6) |
							(pSYSCLKCTRL->PCLK_GPIO << 5) |
							(pSYSCLKCTRL->PCLK_CORP_GATE_DISABLE << 4) |
							(pSYSCLKCTRL->BCLK_MEMC << 3) |
							(pSYSCLKCTRL->BCLK_MEMA << 1) |
							(pSYSCLKCTRL->A940CLK); 
}

void PMR_GetSystemClkCtrl ( tSysClkCtrl *pSYSCLKCTRL )
{
	pSYSCLKCTRL->EXTCLK 				= (pPMR_REG->SYSCLKENREG >> 14) & 0x1;
	pSYSCLKCTRL->PCLK_ADC 				= (pPMR_REG->SYSCLKENREG >> 13) & 0x1;
	pSYSCLKCTRL->PCLK_PWM 				= (pPMR_REG->SYSCLKENREG >> 12) & 0x1;
	pSYSCLKCTRL->BCLK_DMA 				= (pPMR_REG->SYSCLKENREG >> 9 ) & 0x1;
	pSYSCLKCTRL->BCLK_DMA_GATE_DISABLE 	= (pPMR_REG->SYSCLKENREG >> 8 ) & 0x1;
	pSYSCLKCTRL->TCLK_TIMER 			= (pPMR_REG->SYSCLKENREG >> 7 ) & 0x1;
	pSYSCLKCTRL->PCLK_CORP	 			= (pPMR_REG->SYSCLKENREG >> 6 ) & 0x1;
	pSYSCLKCTRL->PCLK_GPIO	 			= (pPMR_REG->SYSCLKENREG >> 5 ) & 0x1;
	pSYSCLKCTRL->PCLK_CORP_GATE_DISABLE	= (pPMR_REG->SYSCLKENREG >> 4 ) & 0x1;
	pSYSCLKCTRL->BCLK_MEMC	 			= (pPMR_REG->SYSCLKENREG >> 3 ) & 0x1;
	pSYSCLKCTRL->BCLK_MEMA	 			= (pPMR_REG->SYSCLKENREG >> 1 ) & 0x1;
	pSYSCLKCTRL->A940CLK	 			= (pPMR_REG->SYSCLKENREG      ) & 0x1;
}

void 	PMR_SetCommClkCtrl    ( tCommClkCtrl *pCOMMCLKCTRL )
{
	pPMR_REG->COMCLKENREG = (pCOMMCLKCTRL->SSPCLK 	<< 9) | 
							(pCOMMCLKCTRL->PCLK_I2C << 8) |
							(pCOMMCLKCTRL->UART3CLK << 7) | 
							(pCOMMCLKCTRL->UART2CLK << 6) |
							(pCOMMCLKCTRL->UART1CLK << 5) | 
							(pCOMMCLKCTRL->UART0CLK << 4) |
							(pCOMMCLKCTRL->PCLK_OWM << 3) |
							(pCOMMCLKCTRL->IRDACLK 	<< 2) |
							(pCOMMCLKCTRL->UDCCLK 	<< 1) |
							(pCOMMCLKCTRL->UHCCLK 	<< 0); 
}

void PMR_GetCommClkCtrl ( tCommClkCtrl *pCOMMCLKCTRL )
{
	pCOMMCLKCTRL->SSPCLK 	= (pPMR_REG->COMCLKENREG >> 9) & 0x1;
	pCOMMCLKCTRL->PCLK_I2C 	= (pPMR_REG->COMCLKENREG >> 8) & 0x1;
	pCOMMCLKCTRL->UART3CLK 	= (pPMR_REG->COMCLKENREG >> 7) & 0x1;
	pCOMMCLKCTRL->UART2CLK 	= (pPMR_REG->COMCLKENREG >> 6) & 0x1;
	pCOMMCLKCTRL->UART1CLK 	= (pPMR_REG->COMCLKENREG >> 5) & 0x1;
	pCOMMCLKCTRL->UART0CLK 	= (pPMR_REG->COMCLKENREG >> 4) & 0x1;
	pCOMMCLKCTRL->PCLK_OWM	= (pPMR_REG->COMCLKENREG >> 3) & 0x1;
	pCOMMCLKCTRL->IRDACLK	= (pPMR_REG->COMCLKENREG >> 2) & 0x1;
	pCOMMCLKCTRL->UDCCLK	= (pPMR_REG->COMCLKENREG >> 1) & 0x1;
	pCOMMCLKCTRL->UHCCLK	= (pPMR_REG->COMCLKENREG >> 0) & 0x1;
}

void	PMR_SetVideoGraphClkCtrl ( tVidGrpClkCtrl *pVIDGRPCLKCTRL )
{
	pPMR_REG->VGCLKENREG = 	(pVIDGRPCLKCTRL->BCLK_DBLCK 			<< 15) | 
							(pVIDGRPCLKCTRL->BCLK_MPEGIF 			<< 14) |
							(pVIDGRPCLKCTRL->PCLK_DECSS 			<< 13) | 
							(pVIDGRPCLKCTRL->MECLK 					<< 12) |
							(pVIDGRPCLKCTRL->BCLK_REF 				<< 11) | 
							(pVIDGRPCLKCTRL->BCLK_RVLD 				<< 10) |
							(pVIDGRPCLKCTRL->BCLK_QGMC 				<< 9 ) |
							(pVIDGRPCLKCTRL->BCLK_MPMC 				<< 8 ) |
							(pVIDGRPCLKCTRL->BCLK_PDF 				<< 7 ) |
							(pVIDGRPCLKCTRL->BCLK_HUFF 				<< 6 ) |
							(pVIDGRPCLKCTRL->MPGCLK_GATE_DISABLE 	<< 5 ) |
							(pVIDGRPCLKCTRL->BCLK_MPEG 				<< 4 ) |
							(pVIDGRPCLKCTRL->ISPCLK 				<< 3 ) |
							(pVIDGRPCLKCTRL->BCLK_GRAPH 			<< 2 ) |
							(pVIDGRPCLKCTRL->BCLK_SCALE 			<< 1 ) |
							(pVIDGRPCLKCTRL->DISPCLK 				<< 0 ) 
							; 
}

void PMR_GetVideoGraphClkCtrl (tVidGrpClkCtrl *pVIDGRPCLKCTRL)
{
	pVIDGRPCLKCTRL->BCLK_DBLCK 		    = (pPMR_REG->VGCLKENREG >> 15) & 0x1;
	pVIDGRPCLKCTRL->BCLK_MPEGIF 		= (pPMR_REG->VGCLKENREG >> 14) & 0x1;
	pVIDGRPCLKCTRL->PCLK_DECSS 		    = (pPMR_REG->VGCLKENREG >> 13) & 0x1;
	pVIDGRPCLKCTRL->MECLK 				= (pPMR_REG->VGCLKENREG >> 12) & 0x1;
	pVIDGRPCLKCTRL->BCLK_REF 			= (pPMR_REG->VGCLKENREG >> 11) & 0x1;
	pVIDGRPCLKCTRL->BCLK_RVLD 			= (pPMR_REG->VGCLKENREG >> 10) & 0x1;
	pVIDGRPCLKCTRL->BCLK_QGMC 			= (pPMR_REG->VGCLKENREG >> 9 ) & 0x1;
	pVIDGRPCLKCTRL->BCLK_MPMC 			= (pPMR_REG->VGCLKENREG >> 8 ) & 0x1;
	pVIDGRPCLKCTRL->BCLK_PDF 			= (pPMR_REG->VGCLKENREG >> 7 ) & 0x1;
	pVIDGRPCLKCTRL->BCLK_HUFF 			= (pPMR_REG->VGCLKENREG >> 6 ) & 0x1;
    pVIDGRPCLKCTRL->MPGCLK_GATE_DISABLE = (pPMR_REG->VGCLKENREG >> 5 ) & 0x1;
    pVIDGRPCLKCTRL->BCLK_MPEG 			= (pPMR_REG->VGCLKENREG >> 4 ) & 0x1;
    pVIDGRPCLKCTRL->ISPCLK 			    = (pPMR_REG->VGCLKENREG >> 3 ) & 0x1;
    pVIDGRPCLKCTRL->BCLK_GRAPH 		    = (pPMR_REG->VGCLKENREG >> 2 ) & 0x1;
    pVIDGRPCLKCTRL->BCLK_SCALE 		    = (pPMR_REG->VGCLKENREG >> 1 ) & 0x1;
    pVIDGRPCLKCTRL->DISPCLK 			= (pPMR_REG->VGCLKENREG >> 0 ) & 0x1;
}

void	PMR_SetAudioStorageClkCtrl ( tAudStrClkCtrl *pAUDSTRCLKCTRL )
{
	pPMR_REG->ASCLKENREG = 	(pAUDSTRCLKCTRL->PCLK_CDROM 	<< 13) | 
							(pAUDSTRCLKCTRL->BCLK_IDEIF 	<< 12) |
							(pAUDSTRCLKCTRL->MSCLK 			<< 9 ) | 
							(pAUDSTRCLKCTRL->PCLK_SD 		<< 8 ) |
							(pAUDSTRCLKCTRL->SPDIFOUTCLK 	<< 3 ) |
							(pAUDSTRCLKCTRL->SPDIFINCLK 	<< 2 ) |
							(pAUDSTRCLKCTRL->I2S_CLK 		<< 1 ) |
							(pAUDSTRCLKCTRL->AC97_CLK 		<< 0 ) 
							; 
}

void PMR_GetAudioStorageClkCtrl ( tAudStrClkCtrl *pAUDSTRCLKCTRL )
{
	pAUDSTRCLKCTRL->PCLK_CDROM 		= (pPMR_REG->ASCLKENREG >> 13) & 0x1;
	pAUDSTRCLKCTRL->BCLK_IDEIF 		= (pPMR_REG->ASCLKENREG >> 12) & 0x1;
	pAUDSTRCLKCTRL->MSCLK 			= (pPMR_REG->ASCLKENREG >> 9 ) & 0x1;
	pAUDSTRCLKCTRL->PCLK_SD  		= (pPMR_REG->ASCLKENREG >> 8 ) & 0x1;
	pAUDSTRCLKCTRL->SPDIFOUTCLK 	= (pPMR_REG->ASCLKENREG >> 3 ) & 0x1;
    pAUDSTRCLKCTRL->SPDIFINCLK 		= (pPMR_REG->ASCLKENREG >> 2 ) & 0x1;
    pAUDSTRCLKCTRL->I2S_CLK 		    = (pPMR_REG->ASCLKENREG >> 1 ) & 0x1;
    pAUDSTRCLKCTRL->AC97_CLK 		= (pPMR_REG->ASCLKENREG >> 0 ) & 0x1;

}

EXPORT_SYMBOL(PMR_Initialize);	
EXPORT_SYMBOL(PMR_SetISPClk);
EXPORT_SYMBOL(PMR_SetDispClk);
