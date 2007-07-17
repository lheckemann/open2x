/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Implementation: sdk2x_system                                        *
 *                                                                          *
 *    This program is free software; you can redistribute it and/or modify  *
 *    it under the terms of version 2 (and only version 2) of the GNU       *
 *    General Public License as published by the Free Software Foundation.  *
 *                                                                          *
 *    This program is distributed in the hope that it will be useful,       *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *    GNU General Public License for more details.                          *
 *                                                                          *
 *    You should have received a copy of the GNU General Public License     *
 *    along with this program; if not, write to the                         *
 *    Free Software Foundation, Inc.,                                       *
 *    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 * **************************************************************************/

#include <mmsp2dtk/mmsp2.h>
#include <mmsp2dtk/sdk2x_priv.h>
#include <common.h>
char sdk2x_private_string [200];
unsigned int oldKeys;

#ifdef SDK2X_SYSTEM_INITIALISE
static const sdk2x_RegisterTable_t CriticalRegisters[] =
{
        { nMSP_MEMCFGW               , 0x1C7F, 2 },
        { nMSP_MEMTIMEW0             , 0xFFDF, 2 },
        { nMSP_MEMTIMEW1             , 0xFFDF, 2 },
        { nMSP_MRMTIMEW2             , 0xFFDF, 2 },
        { nMSP_MEMTIMEW3             , 0xFFDF, 2 },
        { nMSP_MEMTIMEW4             , 0xF1FF, 2 },
        { nMSP_MEMWAITCTRLW          , 0x5555, 2 },
        { nMSP_MEMPAGEW              , 0x0040, 2 },
        { nMSP_MEMIDETIMEW           , 0x0842, 2 },
        { nMSP_MEMPCMCIAMW           , 0x0C63, 2 },
        { nMSP_MEMPCMCIAAW           , 0x1084, 2 },
        { nMSP_MEMPCMCIAIW           , 0x14A5, 2 },
        { nMSP_MEMPCMCIAWAITW        , 0x0C85, 2 },
        { nMSP_MEMIDEWAITW           , 0x0002, 2 },
        { nMSP_MEMDTIMEOUTW          , 0x4008, 2 },
        { nMSP_MEMDMACTRLW           , 0x1FE7, 2 },
        { nMSP_MEMDMAPOLW            , 0xF001, 2 },
        { nMSP_MEMDMATIMEW0          , 0x1084, 2 },
        { nMSP_MEMDMATIMEW1          , 0x1084, 2 },
        { nMSP_MEMDMATIMEW2          , 0x1084, 2 },
        { nMSP_MEMDMATIMEW3          , 0x1084, 2 },
        { nMSP_MEMDMATIMEW4          , 0x1084, 2 },
        { nMSP_MEMDMATIMEW5          , 0x1084, 2 },
        { nMSP_MEMDMATIMEW6          , 0x1084, 2 },
        { nMSP_MEMDMATIMEW7          , 0x4222, 2 },
        { nMSP_MEMDMATIMEW8          , 0x0222, 2 },
        { nMSP_MEMDMASTRBW           , 0x2222, 2 },
        { nMSP_MEMNANDCTRLW          , 0x0083, 2 },
        { nMSP_MEMNANDTIMEW          , 0x07F8, 2 },
        { nMSP_MEMNANDECC0W          , 0xFFCC, 2 },
        { nMSP_MEMNANDECC1W          , 0xFFF0, 2 },
        { nMSP_MEMNANDECC2W          , 0xFFCF, 2 },
        { nMSP_MEMNANDCNTW           , 0x0210, 2 },
        /*	{ nMSP_NFDATA                , 0xFFFF, 2 },
        	{ nMSP_NFCMD                 , 0xFF, 1 },
        	{ nMSP_NFADDR                , 0xFF, 1 },*/
        { nMSP_MEMCFGX               , 0x001A, 2 },
        { nMSP_MEMTIMEX0             , 0x7777, 2 },
        { nMSP_MEMTIMEX1             , 0x07F2, 2 },
        { nMSP_MEMACTPWDX            , 0x00FF, 2 },
        { nMSP_MEMREFX               , 0x0100, 2 },
        /*	{ nMSP_DUAL920DATA0          , 0x8888, 2 },
        	{ nMSP_DUAL920DATA1          , 0x09A4, 2 },
        	{ nMSP_DUAL920DATA2          , 0x800A, 2 },
        	{ nMSP_DUAL920DATA3          , 0xE6F5, 2 },
        	{ nMSP_DUAL920DATA4          , 0x0088, 2 },
        	{ nMSP_DUAL920DATA5          , 0x788B, 2 },
        	{ nMSP_DUAL920DATA6          , 0x0300, 2 },
        	{ nMSP_DUAL920DATA7          , 0x4028, 2 },
        	{ nMSP_DUAL920DATA8          , 0xC120, 2 },
        	{ nMSP_DUAL920DATA9          , 0x09A8, 2 },
        	{ nMSP_DUAL920DATA10         , 0xCA00, 2 },
        	{ nMSP_DUAL920DATA11         , 0x0088, 2 },
        	{ nMSP_DUAL920DATA12         , 0x4FFB, 2 },
        	{ nMSP_DUAL920DATA13         , 0x3820, 2 },
        	{ nMSP_DUAL920DATA14         , 0xC243, 2 },
        	{ nMSP_DUAL920DATA15         , 0x0800, 2 },
        	{ nMSP_DUAL940DATA0          , 0x4310, 2 },
        	{ nMSP_DUAL940DATA1          , 0xF6F7, 2 },
        	{ nMSP_DUAL940DATA2          , 0x40C0, 2 },
        	{ nMSP_DUAL940DATA3          , 0xE8E4, 2 },
        	{ nMSP_DUAL940DATA4          , 0x8309, 2 },
        	{ nMSP_DUAL940DATA5          , 0x01B9, 2 },
        	{ nMSP_DUAL940DATA6          , 0xE572, 2 },
        	{ nMSP_DUAL940DATA7          , 0x2C4C, 2 },
        	{ nMSP_DUAL940DATA8          , 0x8320, 2 },
        	{ nMSP_DUAL940DATA9          , 0x77FB, 2 },
        	{ nMSP_DUAL940DATA10         , 0x3090, 2 },
        	{ nMSP_DUAL940DATA11         , 0x00D0, 2 },
        	{ nMSP_DUAL940DATA12         , 0xFFFD, 2 },
        	{ nMSP_DUAL940DATA13         , 0x1005, 2 },
        	{ nMSP_DUAL940DATA14         , 0xDF7E, 2 },
        	{ nMSP_DUAL940DATA15         , 0x0230, 2 },
        	{ nMSP_DUALINT920            , 0x0000, 2 },
        	{ nMSP_DUALINT940            , 0x0000, 2 },
        	{ nMSP_DUALPEND920           , 0x0000, 2 },
        	{ nMSP_DUALPEND940           , 0x0000, 2 },
        	{ nMSP_DUALCTRL940           , 0x0080, 2 },*/
        { nMSP_PWMODEREG             , 0x0000, 2 },
        { nMSP_CLKCHGSTREG           , 0x0000, 2 },
        { nMSP_SYSCLKENREG           , 0xABFE, 2 },
        { nMSP_COMCLKENREG           , 0x0114, 2 },
        { nMSP_VGCLKENREG            , 0x0001, 2 },
        { nMSP_ASCLKENREG            , 0x0DF1, 2 },
        /*	{ nMSP_FPLLSETVREG           , 0x4904, 2 },
        	{ nMSP_FPLLVSETREG           , 0x4904, 2 },
        	{ nMSP_UPLLSETVREG           , 0x6002, 2 },
        	{ nMSP_UPLLVSETREG           , 0x6002, 2 },
        	{ nMSP_APLLSETVREG           , 0x9802, 2 },
        	{ nMSP_APLLVSETREG           , 0x9802, 2 },*/
        { nMSP_SYSCSETREG            , 0x0000, 2 },
        { nMSP_ESYSCSETREG           , 0x4170, 2 },
        { nMSP_UIRMCSETREG           , 0x0502, 2 },
        { nMSP_AUDICSETREG           , 0x0000, 2 },
        { nMSP_DISPCSETREG           , 0x4A00, 2 },
        { nMSP_IMAGCSETREG           , 0x0000, 2 },
        { nMSP_URT0CSETREG           , 0xC9C9, 2 },
        { nMSP_URT1CSETREG           , 0x000F, 2 },
        { nMSP_A940TIDLE             , 0x0000, 2 },
        { nMSP_SPDICSETREG           , 0x0000, 2 },
        /*	{ nMSP_RTCTSETREG            , 0x00000000, 4 },
        	{ nMSP_RTCTCNTREG            , 0xBFF800B1, 4 },*/
        { nMSP_RTCSTCNTREG           , 0x03FE, 2 },
        { nMSP_TICKSETREG            , 0x0000, 2 },
        { nMSP_ALARMTREG             , 0xFFFF, 2 },
        { nMSP_PWRMGRREG             , 0x0003, 2 },
        { nMSP_CLKMGRREG             , 0x0003, 2 },
        { nMSP_RSTCTRLREG            , 0x0000, 2 },
        { nMSP_RSTSTREG              , 0x0000, 2 },
        { nMSP_BOOTCTRLREG           , 0x0004, 2 },
        { nMSP_LOCKTIMEREG           , 0xA064, 2 },
        { nMSP_RSTTIMEREG            , 0x0FFF, 2 },
        { nMSP_EXTCTRLREG            , 0x0131, 2 },
        { nMSP_STOPTSETREG           , 0x0000, 2 },
        { nMSP_RTCCTRLREG            , 0x8000, 2 },
        { nMSP_RTCSTREG              , 0x0000, 2 },
        /*	{ nMSP_DMAINT                , 0x0000, 2 },
        	{ nMSP_DCH0SRM               , 0x0000, 2 },
        	{ nMSP_DCH0TRM               , 0x0000, 2 },
        	{ nMSP_DCH1SRM               , 0x0000, 2 },
        	{ nMSP_DCH1TRM               , 0x0000, 2 },
        	{ nMSP_DCH2SRM               , 0x0000, 2 },
        	{ nMSP_DCH2TRM               , 0x0000, 2 },
        	{ nMSP_DCH3SRM               , 0x0000, 2 },
        	{ nMSP_DCH3TRM               , 0x0000, 2 },
        	{ nMSP_DCH4SRM               , 0x0000, 2 },
        	{ nMSP_DCH4TRM               , 0x0000, 2 },
        	{ nMSP_DCH5SRM               , 0x0000, 2 },
        	{ nMSP_DCH5TRM               , 0x0000, 2 },
        	{ nMSP_DCH6SRM               , 0x0000, 2 },
        	{ nMSP_DCH6TRM               , 0x0000, 2 },
        	{ nMSP_DCH7SRM               , 0x0000, 2 },
        	{ nMSP_DCH7TRM               , 0x0000, 2 },
        	{ nMSP_DCH8SRM               , 0x0000, 2 },
        	{ nMSP_DCH8TRM               , 0x0000, 2 },
        	{ nMSP_DCH9SRM               , 0x0000, 2 },
        	{ nMSP_DCH9TRM               , 0x0000, 2 },
        	{ nMSP_DCH10SRM              , 0x0000, 2 },
        	{ nMSP_DCH10TRM              , 0x0000, 2 },
        	{ nMSP_DCH11SRM              , 0x0000, 2 },
        	{ nMSP_DCH11TRM              , 0x0000, 2 },
        	{ nMSP_DCH12SRM              , 0x0000, 2 },
        	{ nMSP_DCH12TRM              , 0x0000, 2 },
        	{ nMSP_DCH13SRM              , 0x0000, 2 },
        	{ nMSP_DCH13TRM              , 0x0000, 2 },
        	{ nMSP_DCH14SRM              , 0x0000, 2 },
        	{ nMSP_DCH14TRM              , 0x0000, 2 },
        	{ nMSP_DCH15SRM              , 0x0000, 2 },
        	{ nMSP_DCH15TRM              , 0x0000, 2 },
        	{ nMSP_DMA0COM0              , 0x0000, 2 },
        	{ nMSP_DMA0COM1              , 0x0000, 2 },
        	{ nMSP_DMA0COM2              , 0x0000, 2 },
        	{ nMSP_DMA0CONS              , 0x0000, 2 },
        	{ nMSP_SRC0LADDR             , 0x0000, 2 },
        	{ nMSP_SRC0HADDR             , 0x0000, 2 },
        	{ nMSP_TRG0LADDR             , 0x0000, 2 },
        	{ nMSP_TRG0HADDR             , 0x0000, 2 },
        	{ nMSP_DMA1COM0              , 0x0000, 2 },
        	{ nMSP_DMA1COM1              , 0x0000, 2 },
        	{ nMSP_DMA1COM2              , 0x0000, 2 },
        	{ nMSP_DMA1CONS              , 0x0000, 2 },
        	{ nMSP_SRC1LADDR             , 0x0000, 2 },
        	{ nMSP_SRC1HADDR             , 0x0000, 2 },
        	{ nMSP_TRG1LADDR             , 0x0000, 2 },
        	{ nMSP_TRG1HADDR             , 0x0000, 2 },
        	{ nMSP_DMA2COM0              , 0x0000, 2 },
        	{ nMSP_DMA2COM1              , 0x0000, 2 },
        	{ nMSP_DMA2COM2              , 0x0000, 2 },
        	{ nMSP_DMA2CONS              , 0x0000, 2 },
        	{ nMSP_SRC2LADDR             , 0x0000, 2 },
        	{ nMSP_SRC2HADDR             , 0x0000, 2 },
        	{ nMSP_TRG2LADDR             , 0x0000, 2 },
        	{ nMSP_TRG2HADDR             , 0x0000, 2 },
        	{ nMSP_DMA3COM0              , 0x0000, 2 },
        	{ nMSP_DMA3COM1              , 0x0000, 2 },
        	{ nMSP_DMA3COM2              , 0x0000, 2 },
        	{ nMSP_DMA3CONS              , 0x0000, 2 },
        	{ nMSP_SRC3LADDR             , 0x0000, 2 },
        	{ nMSP_SRC3HADDR             , 0x0000, 2 },
        	{ nMSP_TRG3LADDR             , 0x0000, 2 },
        	{ nMSP_TRG3HADDR             , 0x0000, 2 },
        	{ nMSP_DMA4COM0              , 0x0000, 2 },
        	{ nMSP_DMA4COM1              , 0x0000, 2 },
        	{ nMSP_DMA4COM2              , 0x0000, 2 },
        	{ nMSP_DMA4CONS              , 0x0000, 2 },
        	{ nMSP_SRC4LADDR             , 0x0000, 2 },
        	{ nMSP_SRC4HADDR             , 0x0000, 2 },
        	{ nMSP_TRG4LADDR             , 0x0000, 2 },
        	{ nMSP_TRG4HADDR             , 0x0000, 2 },
        	{ nMSP_DMA5COM0              , 0x0000, 2 },
        	{ nMSP_DMA5COM1              , 0x0000, 2 },
        	{ nMSP_DMA5COM2              , 0x0000, 2 },
        	{ nMSP_DMA5CONS              , 0x0000, 2 },
        	{ nMSP_SRC5LADDR             , 0x0000, 2 },
        	{ nMSP_SRC5HADDR             , 0x0000, 2 },
        	{ nMSP_TRG5LADDR             , 0x0000, 2 },
        	{ nMSP_TRG5HADDR             , 0x0000, 2 },
        	{ nMSP_DMA6COM0              , 0x0000, 2 },
        	{ nMSP_DMA6COM1              , 0x0000, 2 },
        	{ nMSP_DMA6COM2              , 0x0000, 2 },
        	{ nMSP_DMA6CONS              , 0x0000, 2 },
        	{ nMSP_SRC6LADDR             , 0x0000, 2 },
        	{ nMSP_SRC6HADDR             , 0x0000, 2 },
        	{ nMSP_TRG6LADDR             , 0x0000, 2 },
        	{ nMSP_TRG6HADDR             , 0x0000, 2 },
        	{ nMSP_DMA7COM0              , 0x0000, 2 },
        	{ nMSP_DMA7COM1              , 0x0000, 2 },
        	{ nMSP_DMA7COM2              , 0x0000, 2 },
        	{ nMSP_DMA7CONS              , 0x0000, 2 },
        	{ nMSP_SRC7LADDR             , 0x0000, 2 },
        	{ nMSP_SRC7HADDR             , 0x0000, 2 },
        	{ nMSP_TRG7LADDR             , 0x0000, 2 },
        	{ nMSP_TRG7HADDR             , 0x0000, 2 },
        	{ nMSP_DMA8COM0              , 0x0000, 2 },
        	{ nMSP_DMA8COM1              , 0x0000, 2 },
        	{ nMSP_DMA8COM2              , 0x0000, 2 },
        	{ nMSP_DMA8CONS              , 0x0000, 2 },
        	{ nMSP_SRC8LADDR             , 0x0000, 2 },
        	{ nMSP_SRC8HADDR             , 0x0000, 2 },
        	{ nMSP_TRG8LADDR             , 0x0000, 2 },
        	{ nMSP_TRG8HADDR             , 0x0000, 2 },
        	{ nMSP_DMA9COM0              , 0x0000, 2 },
        	{ nMSP_DMA9COM1              , 0x0000, 2 },
        	{ nMSP_DMA9COM2              , 0x0000, 2 },
        	{ nMSP_DMA9CONS              , 0x0000, 2 },
        	{ nMSP_SRC9LADDR             , 0x0000, 2 },
        	{ nMSP_SRC9HADDR             , 0x0000, 2 },
        	{ nMSP_TRG9LADDR             , 0x0000, 2 },
        	{ nMSP_TRG9HADDR             , 0x0000, 2 },
        	{ nMSP_DMA10COM0             , 0x0000, 2 },
        	{ nMSP_DMA10COM1             , 0x0000, 2 },
        	{ nMSP_DMA10COM2             , 0x0000, 2 },
        	{ nMSP_DMA10CONS             , 0x0000, 2 },
        	{ nMSP_SRC10LADDR            , 0x0000, 2 },
        	{ nMSP_SRC10HADDR            , 0x0000, 2 },
        	{ nMSP_TRG10LADDR            , 0x0000, 2 },
        	{ nMSP_TRG10HADDR            , 0x0000, 2 },
        	{ nMSP_DMA11COM0             , 0x0000, 2 },
        	{ nMSP_DMA11COM1             , 0x0000, 2 },
        	{ nMSP_DMA11COM2             , 0x0000, 2 },
        	{ nMSP_DMA11CONS             , 0x0000, 2 },
        	{ nMSP_SRC11LADDR            , 0x0000, 2 },
        	{ nMSP_SRC11HADDR            , 0x0000, 2 },
        	{ nMSP_TRG11LADDR            , 0x0000, 2 },
        	{ nMSP_TRG11HADDR            , 0x0000, 2 },
        	{ nMSP_DMA12COM0             , 0x0000, 2 },
        	{ nMSP_DMA12COM1             , 0x0000, 2 },
        	{ nMSP_DMA12COM2             , 0x0000, 2 },
        	{ nMSP_DMA12CONS             , 0x0000, 2 },
        	{ nMSP_SRC12LADDR            , 0x0000, 2 },
        	{ nMSP_SRC12HADDR            , 0x0000, 2 },
        	{ nMSP_TRG12LADDR            , 0x0000, 2 },
        	{ nMSP_TRG12HADDR            , 0x0000, 2 },
        	{ nMSP_DMA13COM0             , 0x0000, 2 },
        	{ nMSP_DMA13COM1             , 0x0000, 2 },
        	{ nMSP_DMA13COM2             , 0x0000, 2 },
        	{ nMSP_DMA13CONS             , 0x0000, 2 },
        	{ nMSP_SRC13LADDR            , 0x0000, 2 },
        	{ nMSP_SRC13HADDR            , 0x0000, 2 },
        	{ nMSP_TRG13LADDR            , 0x0000, 2 },
        	{ nMSP_TRG13HADDR            , 0x0000, 2 },
        	{ nMSP_DMA14COM0             , 0x0000, 2 },
        	{ nMSP_DMA14COM1             , 0x0000, 2 },
        	{ nMSP_DMA14COM2             , 0x0000, 2 },
        	{ nMSP_DMA14CONS             , 0x0000, 2 },
        	{ nMSP_SRC14LADDR            , 0x0000, 2 },
        	{ nMSP_SRC14HADDR            , 0x0000, 2 },
        	{ nMSP_TRG14LADDR            , 0x0000, 2 },
        	{ nMSP_TRG14HADDR            , 0x0000, 2 },
        	{ nMSP_DMA15COM0             , 0x0000, 2 },
        	{ nMSP_DMA15COM1             , 0x0000, 2 },
        	{ nMSP_DMA15COM2             , 0x0000, 2 },
        	{ nMSP_DMA15CONS             , 0x0000, 2 },
        	{ nMSP_SRC15LADDR            , 0x0000, 2 },
        	{ nMSP_SRC15HADDR            , 0x0000, 2 },
        	{ nMSP_TRG15LADDR            , 0x0000, 2 },
        	{ nMSP_TRG15HADDR            , 0x0000, 2 },*/
        /*	{ nMSP_SRCPEND               , 0x000000C0, 4 },
        	{ nMSP_INTMODE               , 0x00000000, 4 },*/
        { nMSP_INTMASK               , 0xFB71B9DF, 4 },
        { nMSP_PRIORITY              , 0x00000000, 4 },
        /*	{ nMSP_INTPEND               , 0x00000000, 4 },
        	{ nMSP_INTOFFSET             , 0x0000, 2 },*/
        /*	{ nMSP_TCOUNT                , 0x7A495574, 4 },*/
        { nMSP_TMATCH0               , 0x7A8E0000, 4 },
        { nMSP_TMATCH1               , 0x00000000, 4 },
        { nMSP_TMATCH2               , 0x00000000, 4 },
        { nMSP_TMATCH3               , 0x00000000, 4 },
        { nMSP_TCONTROL              , 0x0001, 2 },
        { nMSP_TSTATUS               , 0x0000, 2 },
        { nMSP_TINTEN                , 0x0001, 2 },
		  { nMSP_LCON0                 , 0x0003, 2 },
		  { nMSP_UCON0                 , 0x0385, 2 },
		  { nMSP_FCON0                 , 0x0001, 2 },
		  { nMSP_MCON0                 , 0x0000, 2 },
		  { nMSP_TRSTATUS0             , 0x0006, 2 },
		  { nMSP_ESTATUS0              , 0x0000, 2 },
		  { nMSP_FSTATUS0              , 0x0000, 2 },
		  { nMSP_MSTATUS0              , 0x0000, 2 },
        /*	{ nMSP_THB0                  , 0x00, 1 },
		  { nMSP_RHB0                  , 0x00, 1 },*/
		  { nMSP_BRD0                  , 0x0007, 2 },
		  { nMSP_TIMEOUTREG0           , 0x001F, 2 },
		  { nMSP_LCON1                 , 0x0003, 2 },
		  { nMSP_UCON1                 , 0x0385, 2 },
		  { nMSP_FCON1                 , 0x0001, 2 },
		  { nMSP_MCON1                 , 0x0000, 2 },
		  { nMSP_TRSTATUS1             , 0x0006, 2 },
		  { nMSP_ESTATUS1              , 0x0000, 2 },
		  { nMSP_FSTATUS1              , 0x0000, 2 },
		  { nMSP_MSTATUS1              , 0x0000, 2 },
        /*	{ nMSP_THB0                  , 0x00, 1 },
		  { nMSP_RHB0                  , 0x00, 1 },*/
		  { nMSP_BRD1                  , 0x0007, 2 },
		  { nMSP_TIMEOUTREG1           , 0x001F, 2 },
        /*	{ nMSP_LCON1                 , 0x0000, 2 },
        	{ nMSP_UCON1                 , 0x0000, 2 },
        	{ nMSP_FCON1                 , 0x0000, 2 },
        	{ nMSP_MCON1                 , 0x0000, 2 },
        	{ nMSP_TRSTATUS1             , 0x0006, 2 },
        	{ nMSP_ESTATUS1              , 0x0000, 2 },
        	{ nMSP_FSTATUS1              , 0x0000, 2 },
        	{ nMSP_MSTATUS1              , 0x00F7, 2 },
        	{ nMSP_THB1                  , 0x00, 1 },
        	{ nMSP_RHB1                  , 0x00, 1 },
        	{ nMSP_BRD1                  , 0x0000, 2 },
        	{ nMSP_TIMEOUTREG1           , 0x001F, 2 },
        	{ nMSP_LCON2                 , 0x0000, 2 },
        	{ nMSP_UCON2                 , 0x0000, 2 },
        	{ nMSP_FCON2                 , 0x0000, 2 },
        	{ nMSP_MCON2                 , 0x0000, 2 },
        	{ nMSP_TRSTATUS2             , 0x0006, 2 },
        	{ nMSP_ESTATUS2              , 0x0000, 2 },
        	{ nMSP_FSTATUS2              , 0x0000, 2 },
        	{ nMSP_MSTATUS2              , 0x0000, 2 },
        	{ nMSP_THB2                  , 0x00, 1 },
        	{ nMSP_RHB2                  , 0x00, 1 },
        	{ nMSP_BRD2                  , 0x0000, 2 },
        	{ nMSP_TIMEOUTREG2           , 0x001F, 2 },
        	{ nMSP_LCON3                 , 0x0000, 2 },
        	{ nMSP_UCON3                 , 0x0000, 2 },
        	{ nMSP_FCON3                 , 0x0000, 2 },
        	{ nMSP_MCON3                 , 0x0000, 2 },
        	{ nMSP_TRSTATUS3             , 0x0006, 2 },
        	{ nMSP_ESTATUS3              , 0x0000, 2 },
        	{ nMSP_FSTATUS3              , 0x0000, 2 },
        	{ nMSP_MSTATUS3              , 0x0000, 2 },
        	{ nMSP_THB3                  , 0x00, 1 },
        	{ nMSP_RHB3                  , 0x00, 1 },
        	{ nMSP_BRD3                  , 0x0000, 2 },
        	{ nMSP_TIMEOUTREG3           , 0x001F, 2 },*/
        { nMSP_INTSTATREG            , 0x0000, 2 },
        { nMSP_PORTCON               , 0x0000, 2 },

        /* GPIOA is a 16-bit port (15:0)
         * GPIOA15:0 are allocated to the LCD, as VD23:8 */
        { nMSP_GPIOAALTFNHI          , 0xAAAA, 2 },
        { nMSP_GPIOAALTFNLOW         , 0xAAAA, 2 },
        /* GPIOB is a 16-bit port (15:0)
         * GPIOB15:8 are VD7:0. GPIOB7 is CLKH. GPIOB6 is DE. GPIOB5 is HSYNC. GPIOB4 is VSYNC.
         * GPIOB3:2 are outputs. GPIOB1:0 are inputs. */
        { nMSP_GPIOBALTFNHI          , 0xAAAA, 2 },
        { nMSP_GPIOBALTFNLOW         , 0xAA50, 2 },
        /* GPIOC is a 16-bit port (15:0)
         * GPIOC15:0 are all inputs */
        { nMSP_GPIOCALTFNHI          , 0x0000, 2 },
        { nMSP_GPIOCALTFNLOW         , 0x0000, 2 },
        /* GPIOD is a 14-bit port (13:0)
         * GPIOD13:11 are inputs. GPIOD10 is RX2 (TX2 is not used?). GPIOD9 is RX1. GPIOD8 is TX1.
         * GPIOD7:6 are inputs. GPIOD5 is nDTR. GPIOD4 is output. GPIOD3 is nDCD. GPIOD2 is input.
         * GPIOD1 is RX0. GPIOD0 is TX0. */
        { nMSP_GPIODALTFNHI          , 0x002A, 2 },	/* Different read / write */
        { nMSP_GPIODALTFNLOW         , 0x098A, 2 },	/* Different read / write */
        /* GPIOE is a 16-bit port (15:0)
         * GPIOE15:0 are Non-DRAM data bus */
        { nMSP_GPIOEALTFNHI          , 0xAAAA, 2 },
        { nMSP_GPIOEALTFNLOW         , 0xAAAA, 2 },
        /* GPIOF is a 10-bit port (9:0)
         * GPIOF9:0 are non-DRAM address bus, bits 25:16 */
        { nMSP_GPIOFALTFNHI          , 0x000A, 2 },	/* Different read / write */
        { nMSP_GPIOFALTFNLOW         , 0xAAAA, 2 },	/* Different read / write */
        /* GPIOG is a 16-bit port (15:0)
         * GPIOG15:1 are non-DRAM address bus, bits 15:1
         * GPIOG0 is DQMz[0] (BTENB0) */
        { nMSP_GPIOGALTFNHI          , 0xAAAA, 2 },
        { nMSP_GPIOGALTFNLOW         , 0xAAAA, 2 },
        /* GPIOH is a 7-bit port (6:0) */
        /* GPIOH6 is XDOFF (Gate-off signal for LCD driver). GPIOH5:0 are outputs */
        { nMSP_GPIOHALTFNHI          , 0x0000, 2 },	/* Different read / write */
        { nMSP_GPIOHALTFNLOW         , 0x0955, 2 },	/* Different read / write */
        /* GPIOI is a 16-bit port (15:0)
         * GPIOI15 is "Dedicated GPIO (CBBUSREQ)"????
         * GPIOI14 is input
         * GPIOI13 is nPWAIT, a wait signal from IDE/PCMCIA/CF/Static memory
         * GPIOI12 is DQMz[1] (BTENB1)
         * GPIOI11:8 are SCS[3]:[0] - static RAM device chip selects
         * GPIOI7:6 are PCS[1]:[0] - PCMCIA / CF device chip selects
         * GPIOI5:4 are ICS[1]:[0] - IDE device chip selects
         * GPIOI3:2 are inputs
         * GPIOI1:0 are nNFCE[1]:[0] - Flash chip enable signals */
        { nMSP_GPIOIALTFNHI          , 0x8AAA, 2 },
        { nMSP_GPIOIALTFNLOW         , 0xAA0A, 2 },
        /* GPIOJ is a 16-bit port (15:0)
         * GPIOJ15:14 are inputs. GPIOJ13 is output. GPIOJ12:11 are PCMCIA/CF/SM control signals
         * GPIOJ10 is NAND CLE. GPIOJ9 is NAND ALE. GPIOJ8:3 are PCMCIA/CF control signals
         * GPIOJ2 is NAND RnB. GPIOJ1 is NAND Output enable. GPIOJ0 is NAND Write enable. */
        { nMSP_GPIOJALTFNHI          , 0x06AA, 2 },
        { nMSP_GPIOJALTFNLOW         , 0xAAAA, 2 },
        /* GPIOK is an 8-bit port (7:0)
         * GPIOK7:6 are inputs. GPIOK5 is DREQ1 (DMA Request 1). GPIOK4:2 are inputs.
         * GPIOK1 is DVAL1 (DMA Validate 1). GPIOK0 is input. */
        { nMSP_GPIOKALTFNHI          , 0x0000, 2 },	/* Different read / write */
        { nMSP_GPIOKALTFNLOW         , 0x0808, 2 },	/* Different read / write */
        /* GPIOL is a 15-bit port (14:0)
         * GPIOL14:11 are inputs. GPIOL10:6 are AC'97 signals. GPIOL5:0 are SD Card signals */
        { nMSP_GPIOLALTFNHI          , 0x002A, 2 },	/* Different read / write */
        { nMSP_GPIOLALTFNLOW         , 0xAAAA, 2 },	/* Different read / write */
        /* GPIOM is a 9-bit port (8:0)
         * GPIOM8:0 are inputs */
        { nMSP_GPIOMALTFNHI          , 0x0000, 2 },	/* Different read / write */
        { nMSP_GPIOMALTFNLOW         , 0x0000, 2 },	/* Different read / write */
        /* GPION is an 8-bit port (7:0)
         * GPION7:0 are inputs */
        { nMSP_GPIONALTFNHI          , 0x0000, 2 },	/* Different read / write */
        { nMSP_GPIONALTFNLOW         , 0x0000, 2 },	/* Different read / write */
        /* GPIOO is a 6-bit port (5:0)
         * GPIOO5:0 are inputs */
        { nMSP_GPIOOALTFNHI          , 0x0000, 2 },	/* Different read / write */
        { nMSP_GPIOOALTFNLOW         , 0x0000, 2 },	/* Different read / write */

        { nMSP_GPIOAOUT              , 0x0000, 2 },
        { nMSP_GPIOBOUT              , 0x000C, 2 },
        { nMSP_GPIOCOUT              , 0x0000, 2 },
        { nMSP_GPIODOUT              , 0x0010, 2 },
        { nMSP_GPIOEOUT              , 0x0000, 2 },
        { nMSP_GPIOFOUT              , 0x0000, 2 },
        { nMSP_GPIOGOUT              , 0x0000, 2 },
        { nMSP_GPIOHOUT              , 0x000E, 2 },
        { nMSP_GPIOIOUT              , 0x0000, 2 },
        { nMSP_GPIOJOUT              , 0x0000, 2 },
        { nMSP_GPIOKOUT              , 0x0000, 2 },
        { nMSP_GPIOLOUT              , 0x0000, 2 },
        { nMSP_GPIOMOUT              , 0x0000, 2 },
        { nMSP_GPIONOUT              , 0x0000, 2 },
        { nMSP_GPIOOOUT              , 0x0000, 2 },
        { nMSP_GPIOAEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOBEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOCEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIODEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOEEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOFEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOGEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOHEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOIEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOJEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOKEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOLEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOMEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIONEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOOEVTTYPLOW        , 0x0000, 2 },
        { nMSP_GPIOAEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOBEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOCEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIODEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOEEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOFEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOGEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOHEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOIEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOJEVTTYPHI         , 0x3000, 2 },
        { nMSP_GPIOKEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOLEVTTYPHI         , 0x0180, 2 },
        { nMSP_GPIOMEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIONEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOOEVTTYPHI         , 0x0000, 2 },
        { nMSP_GPIOAPUENB            , 0xFFFF, 2 },
        { nMSP_GPIOBPUENB            , 0xFF0F, 2 },
        { nMSP_GPIOCPUENB            , 0xFFFF, 2 },
        { nMSP_GPIODPUENB            , 0x3FF8, 2 },
        { nMSP_GPIOEPUENB            , 0x0000, 2 },
        { nMSP_GPIOFPUENB            , 0x0000, 2 },
        { nMSP_GPIOGPUENB            , 0x0000, 2 },
        { nMSP_GPIOHPUENB            , 0x007E, 2 },
        { nMSP_GPIOIPUENB            , 0x4200, 2 },
        { nMSP_GPIOJPUENB            , 0x0000, 2 },
        { nMSP_GPIOKPUENB            , 0x00FF, 2 },
        { nMSP_GPIOLPUENB            , 0x7FFF, 2 },
        { nMSP_GPIOMPUENB            , 0x01FF, 2 },
        { nMSP_GPIONPUENB            , 0x00FF, 2 },
        { nMSP_GPIOOPUENB            , 0x003F, 2 },
        { nMSP_GPIOAINTENB           , 0x0000, 2 },
        { nMSP_GPIOBINTENB           , 0x0000, 2 },
        { nMSP_GPIOCINTENB           , 0x0000, 2 },
        { nMSP_GPIODINTENB           , 0x0000, 2 },
        { nMSP_GPIOEINTENB           , 0x0000, 2 },
        { nMSP_GPIOFINTENB           , 0x0000, 2 },
        { nMSP_GPIOGINTENB           , 0x0000, 2 },
        { nMSP_GPIOHINTENB           , 0x0000, 2 },
        { nMSP_GPIOIINTENB           , 0x0000, 2 },
        /*	{ nMSP_GPIOJINTENB           , 0x4000, 2 },*/
        { nMSP_GPIOJINTENB           , 0x0000, 2 },
        { nMSP_GPIOKINTENB           , 0x0000, 2 },
        // 	{ nMSP_GPIOLINTENB           , 0x0800, 2 },
        { nMSP_GPIOLINTENB           , 0x0000, 2 },
        { nMSP_GPIOMINTENB           , 0x0000, 2 },
        { nMSP_GPIONINTENB           , 0x0000, 2 },
        { nMSP_GPIOOINTENB           , 0x0000, 2 },
        { nMSP_GPIOAEVT              , 0x0000, 2 },
        { nMSP_GPIOBEVT              , 0x0000, 2 },
        { nMSP_GPIOCEVT              , 0x0000, 2 },
        { nMSP_GPIODEVT              , 0x0000, 2 },
        { nMSP_GPIOEEVT              , 0x0000, 2 },
        { nMSP_GPIOFEVT              , 0x0000, 2 },
        { nMSP_GPIOGEVT              , 0x0000, 2 },
        { nMSP_GPIOHEVT              , 0x0000, 2 },
        { nMSP_GPIOIEVT              , 0x0000, 2 },
        { nMSP_GPIOJEVT              , 0x0E37, 2 },
        { nMSP_GPIOKEVT              , 0x0000, 2 },
        { nMSP_GPIOLEVT              , 0x0000, 2 },
        { nMSP_GPIOMEVT              , 0x0000, 2 },
        { nMSP_GPIONEVT              , 0x0000, 2 },
        { nMSP_GPIOOEVT              , 0x0000, 2 },
        { nMSP_GPIOPADSEL            , 0x0000, 2 },
        { nMSP_GPIOGREVTSTAT         , 0x0000, 2 },
        /*	{ nMSP_GPIOAPINLVL           , 0x97FF, 2 },
        	{ nMSP_GPIOBPINLVL           , 0xFFCF, 2 },
        	{ nMSP_GPIOCPINLVL           , 0xFFFF, 2 },
        	{ nMSP_GPIODPINLVL           , 0x3FFB, 2 },
        	{ nMSP_GPIOEPINLVL           , 0xFFFF, 2 },
        	{ nMSP_GPIOFPINLVL           , 0x0000, 2 },
        	{ nMSP_GPIOGPINLVL           , 0x0000, 2 },
        	{ nMSP_GPIOHPINLVL           , 0x006E, 2 },
        	{ nMSP_GPIOIPINLVL           , 0x3FFF, 2 },
        	{ nMSP_GPIOJPINLVL           , 0x5E6F, 2 },
        	{ nMSP_GPIOKPINLVL           , 0x002F, 2 },
        	{ nMSP_GPIOLPINLVL           , 0x7CBF, 2 },
        	{ nMSP_GPIOMPINLVL           , 0x01FF, 2 },
        	{ nMSP_GPIONPINLVL           , 0x00FF, 2 },
        	{ nMSP_GPIOOPINLVL           , 0x003F, 2 },*/
        { nMSP_GPIOEOUT_ALIVE        , 0x0000, 2 },
        { nMSP_GPIOEOUTENB_ALIVE     , 0xFFFF, 2 },
        { nMSP_GPIOEPUENB_ALIVE      , 0xFFFF, 2 },
        { nMSP_GPIOFOUT_ALIVE        , 0x0000, 2 },
        { nMSP_GPIOFOUTENB_ALIVE     , 0x03FF, 2 },
        { nMSP_GPIOFPUENB_ALIVE      , 0x03FF, 2 },
        { nMSP_GPIOGOUT_ALIVE        , 0x0000, 2 },
        { nMSP_GPIOGOUTENB_ALIVE     , 0xFFFF, 2 },
        { nMSP_GPIOGPUENB_ALIVE      , 0xFFFF, 2 },
        { nMSP_GPIOIOUT_ALIVE        , 0x0000, 2 },
        { nMSP_GPIOIOUTENB_ALIVE     , 0xFFFF, 2 },
        { nMSP_GPIOIPUENB_ALIVE      , 0xFFFF, 2 },
        { nMSP_GPIOJOUT_ALIVE        , 0x0000, 2 },
        { nMSP_GPIOJOUTENB_ALIVE     , 0xFFFF, 2 },
        { nMSP_GPIOJPUENB_ALIVE      , 0xFFFF, 2 },
        { nMSP_SC_CMD                , 0x0100, 2 },
        { nMSP_SC_STATUS             , 0x0004, 2 },
        { nMSP_SC_MIRROR             , 0x0000, 2 },
        { nMSP_SC_PRE_VRATIO         , 0x0400, 2 },
        { nMSP_SC_PRE_HRATIO         , 0x0400, 2 },
        { nMSP_SC_POST_VRATIOL       , 0x4000, 2 },
        { nMSP_SC_POST_VRATIOH       , 0x0000, 2 },
        { nMSP_SC_POST_HRATIOL       , 0x4000, 2 },
        { nMSP_SC_POST_HRATIOH       , 0x0000, 2 },
        { nMSP_SC_SRC_ODD_ADDRL      , 0x0000, 2 },
        { nMSP_SC_SRC_ODD_ADDRH      , 0x0000, 2 },
        { nMSP_SC_SRC_EVEN_ADDRL     , 0x0000, 2 },
        { nMSP_SC_SRC_EVEN_ADDRH     , 0x0000, 2 },
        { nMSP_SC_DST_ADDRL          , 0x0000, 2 },
        { nMSP_SC_DST_ADDRH          , 0x0000, 2 },
        { nMSP_SC_SRC_PXL_HEIGHT     , 0x00F0, 2 },
        { nMSP_SC_SRC_PXL_WIDTH      , 0x0140, 2 },
        { nMSP_SC_SRC_PXL_REQCNT     , 0x0140, 2 },
        { nMSP_SC_DST_PXL_HEIGHT     , 0x00F0, 2 },
        { nMSP_SC_DST_PXL_WIDTH      , 0x0140, 2 },
        { nMSP_SC_DST_WPXL_WIDTH     , 0x0000, 2 },
        { nMSP_SC_LUMA_OFFSET        , 0x0000, 2 },
        { nMSP_SC_CB_OFFSET          , 0x0000, 2 },
        { nMSP_SC_CR_OFFSET          , 0x0000, 2 },
        { nMSP_SC_DELAY              , 0x0073, 2 },
        { nMSP_SC_MEM_CNTR           , 0x0010, 2 },
        { nMSP_SC_IRQ                , 0x0000, 2 },
        { nMSP_SC_RESERVED           , 0x2330, 2 },
        { nMSP_MLC_OVLAY_CNTR        , 0x1004, 2 },
        { nMSP_MLC_YUV_EFFECT        , 0x0000, 2 },
        { nMSP_MLC_YUV_CNTL          , 0x0000, 2 },
        { nMSP_MLC_YUVA_TP_HSC       , 0x0000, 2 },
        { nMSP_MLC_YUVA_BT_HSC       , 0x0000, 2 },
        { nMSP_MLC_YUVA_TP_HSCL      , 0x0000, 2 },
        { nMSP_MLC_YUVA_TP_HSCH      , 0x0000, 2 },
        { nMSP_MLC_YUVA_BT_HSCL      , 0x0000, 2 },
        { nMSP_MLC_YUVA_BT_HSCH      , 0x0000, 2 },
        { nMSP_MLC_YUVA_TP_PXW       , 0x0000, 2 },
        { nMSP_MLC_YUVA_BT_PXW       , 0x0000, 2 },
        { nMSP_MLC_YUVA_STX          , 0x0000, 2 },
        { nMSP_MLC_YUVA_ENDX         , 0x0000, 2 },
        { nMSP_MLC_YUVA_TP_STY       , 0x0000, 2 },
        { nMSP_MLC_YUVA_TP_ENDY      , 0x0000, 2 },
        { nMSP_MLC_YUVA_BT_ENDY      , 0x0000, 2 },
        { nMSP_MLC_YUVA_TP_OADRL     , 0x0000, 2 },
        { nMSP_MLC_YUVA_TP_OADRH     , 0x0000, 2 },
        { nMSP_MLC_YUVA_TP_EADRL     , 0x0000, 2 },
        { nMSP_MLC_YUVA_TP_EADRH     , 0x0000, 2 },
        { nMSP_MLC_YUVA_BT_OADRL     , 0x0000, 2 },
        { nMSP_MLC_YUVA_BT_OADRH     , 0x0000, 2 },
        { nMSP_MLC_YUVA_BT_EADRL     , 0x0000, 2 },
        { nMSP_MLC_YUVA_BT_EADRH     , 0x0000, 2 },
        { nMSP_MLC_YUVB_TP_HSC       , 0x0000, 2 },
        { nMSP_MLC_YUVB_BT_HSC       , 0x0000, 2 },
        { nMSP_MLC_YUVB_TP_VSCL      , 0x0000, 2 },
        { nMSP_MLC_YUVB_TP_VSCH      , 0x0000, 2 },
        { nMSP_MLC_YUVB_BT_VSCL      , 0x0000, 2 },
        { nMSP_MLC_YUVB_BT_VSCH      , 0x0000, 2 },
        { nMSP_MLC_YUVB_TP_PXW       , 0x0000, 2 },
        { nMSP_MLC_YUVB_BT_PXW       , 0x0000, 2 },
        { nMSP_MLC_YUVB_STX          , 0x0000, 2 },
        { nMSP_MLC_YUVB_ENDX         , 0x0000, 2 },
        { nMSP_MLC_YUVB_TP_STY       , 0x0000, 2 },
        { nMSP_MLC_YUVB_TP_ENDY      , 0x0000, 2 },
        { nMSP_MLC_YUVB_BT_ENDY      , 0x0000, 2 },
        { nMSP_MLC_YUVB_TP_OADRL     , 0x0000, 2 },
        { nMSP_MLC_YUVB_TP_OADRH     , 0x0000, 2 },
        { nMSP_MLC_YUVB_TP_EADRL     , 0x0000, 2 },
        { nMSP_MLC_YUVB_TP_EADRH     , 0x0000, 2 },
        { nMSP_MLC_YUVB_BT_OADRL     , 0x0000, 2 },
        { nMSP_MLC_YUVB_BT_OADRH     , 0x0000, 2 },
        { nMSP_MLC_YUVB_BT_EADRL     , 0x0000, 2 },
        { nMSP_MLC_YUVB_BT_EADRH     , 0x0000, 2 },
        { nMSP_MLC_STL_CNTL          , 0x04AB, 2 },
        { nMSP_MLC_STL_MIXMUX        , 0x0000, 2 },
        { nMSP_MLC_RGB_ALPHAL        , 0x0008, 2 },
        { nMSP_MLC_STL_ALPHAL        , 0x0008, 2 },
        { nMSP_MLC_STL_ALPHAH        , 0x0000, 2 },
        { nMSP_MLC_STL1_STX          , 0x0000, 2 },
        { nMSP_MLC_STL1_ENDX         , 0x013F, 2 },
        { nMSP_MLC_STL1_STY          , 0x0000, 2 },
        { nMSP_MLC_STL1_ENDY         , 0x00EF, 2 },
        { nMSP_MLC_STL2_STX          , 0x0000, 2 },
        { nMSP_MLC_STL2_ENDX         , 0x0000, 2 },
        { nMSP_MLC_STL2_STY          , 0x0000, 2 },
        { nMSP_MLC_STL2_ENDY         , 0x0000, 2 },
        { nMSP_MLC_STL3_STX          , 0x0000, 2 },
        { nMSP_MLC_STL3_ENDX         , 0x0000, 2 },
        { nMSP_MLC_STL3_STY          , 0x0000, 2 },
        { nMSP_MLC_STL3_ENDY         , 0x0000, 2 },
        { nMSP_MLC_STL4_STX          , 0x0000, 2 },
        { nMSP_MLC_STL4_ENDX         , 0x0000, 2 },
        { nMSP_MLC_STL4_STY          , 0x0000, 2 },
        { nMSP_MLC_STL4_ENDY         , 0x0000, 2 },
        { nMSP_MLC_STL_CKEY_GB       , 0xFFFF, 2 },
        { nMSP_MLC_STL_CKEY_R        , 0x00FF, 2 },
        { nMSP_MLC_STL_HSC           , 0x0400, 2 },
        { nMSP_MLC_STL_VSCL          , 0x0280, 2 },
        { nMSP_MLC_STL_VSCH          , 0x0280, 2 },
        { nMSP_MLC_STL_HW            , 0x0280, 2 },
        { nMSP_MLC_STL_OADRL         , 0x1000, 2 },
        { nMSP_MLC_STL_OADRH         , 0x0310, 2 },
        { nMSP_MLC_STL_EADRL         , 0x1000, 2 },
        { nMSP_MLC_STL_EADRH         , 0x0310, 2 },
        { nMSP_MLC_OSD_OADRL         , 0x0000, 2 },
        { nMSP_MLC_OSD_OADRH         , 0x0000, 2 },
        { nMSP_MLC_OSD_EADRL         , 0x0000, 2 },
        { nMSP_MLC_OSD_EADRH         , 0x0000, 2 },
        { nMSP_MLC_HWC_CNTR          , 0xFF00, 2 },
        { nMSP_MLC_HWC_STX           , 0x0000, 2 },
        { nMSP_MLC_HWC_STY           , 0x0000, 2 },
        { nMSP_MLC_HWC_FGR           , 0x0000, 2 },
        { nMSP_MLC_HWC_FB            , 0x0000, 2 },
        { nMSP_MLC_HWC_BGR           , 0x0000, 2 },
        { nMSP_MLC_HWC_BB            , 0x0000, 2 },
        { nMSP_MLC_HWC_OADRL         , 0x0000, 2 },
        { nMSP_MLC_HWC_OADRH         , 0x0000, 2 },
        { nMSP_MLC_HWC_EADRL         , 0x0000, 2 },
        { nMSP_MLC_HWC_EADRH         , 0x0000, 2 },
        { nMSP_MLC_LUMA_ENH          , 0x0000, 2 },
        { nMSP_MLC_HUECB1AB          , 0x4000, 2 },
        { nMSP_MLC_HUECR1AB          , 0x0040, 2 },
        { nMSP_MLC_HUECB2AB          , 0x4000, 2 },
        { nMSP_MLC_HUECR2AB          , 0x0040, 2 },
        { nMSP_MLC_HUECB3AB          , 0x4000, 2 },
        { nMSP_MLC_HUECR3AB          , 0x0040, 2 },
        { nMSP_MLC_HUECB4AB          , 0x4000, 2 },
        { nMSP_MLC_HUECR4AB          , 0x0040, 2 },
        { nMSP_MLC_DITHER            , 0x0000, 2 },
        /*	{ nMSP_MLC_OSD_PALLT_A       , 0x0000, 2 },
        	{ nMSP_MLC_OSD_PALLT_D       , 0x5ABF, 2 },
        	{ nMSP_MLC_STL_PALLT_A       , 0x0000, 2 },
        	{ nMSP_MLC_STL_PALLT_D       , 0x0FDF, 2 },
        	{ nMSP_MLC_GAMMA_A           , 0x0000, 2 },
        	{ nMSP_MLC_GAMMA_D           , 0x252B, 2 },*/
        { nMSP_MLC_SPU_CTRL          , 0x0000, 2 },
        { nMSP_MLC_SPU_DELAY         , 0x0000, 2 },
        { nMSP_MLC_SPU_BASEI_ADDRL   , 0x0000, 2 },
        { nMSP_MLC_SPU_BASEI_ADDRH   , 0x0000, 2 },
        { nMSP_MLC_SPU_HLI_ADDRL     , 0x0000, 2 },
        { nMSP_MLC_SPU_HLI_ADDRH     , 0x0000, 2 },
        { nMSP_MLC_SPU_PAL           , 0x777D, 2 },
        { nMSP_DPC_CNTL              , 0x0005, 2 },
        { nMSP_DPC_FPICNTL           , 0x0000, 2 },
        { nMSP_DPC_FPIPOL1           , 0x0010, 2 },
        { nMSP_DPC_FPIPOL2           , 0xFFFF, 2 },
        { nMSP_DPC_FPIPOL3           , 0x00FF, 2 },
        { nMSP_DPC_FPIATV1           , 0xFFFF, 2 },
        { nMSP_DPC_FPIATV2           , 0xFFFF, 2 },
        { nMSP_DPC_FPIATV3           , 0x00FF, 2 },
        { nMSP_DPC_X_MAX             , 0x013F, 2 },
        { nMSP_DPC_Y_MAX             , 0x00EF, 2 },
        { nMSP_DPC_HS_WIDTH          , 0x041D, 2 },
        { nMSP_DPC_HS_STR            , 0x0009, 2 },
        { nMSP_DPC_HS_END            , 0x0009, 2 },
        { nMSP_DPC_V_SYNC            , 0x0403, 2 },
        { nMSP_DPC_V_END             , 0x0816, 2 },
        { nMSP_DPC_TV_BNK            , 0x0000, 2 },
        { nMSP_DPC_DE                , 0x0250, 2 },
        { nMSP_DPC_PS                , 0x0000, 2 },
        { nMSP_DPC_FG                , 0x0000, 2 },
        { nMSP_DPC_LP                , 0x0000, 2 },
        { nMSP_DPC_CLKVH             , 0x0000, 2 },
        { nMSP_DPC_CLKVL             , 0x0000, 2 },
        { nMSP_DPC_POL               , 0x0000, 2 },
        { nMSP_DPC_CISSYNC           , 0x0000, 2 },
        { nMSP_DPC_MID_SYNC          , 0x0000, 2 },
        { nMSP_DPC_C_SYNC            , 0x0064, 2 },
        { nMSP_DPC_Y_BLANK           , 0x00FC, 2 },
        { nMSP_DPC_C_BLANK           , 0x0200, 2 },
        { nMSP_DPC_YP_CSYNC          , 0x02EC, 2 },
        { nMSP_DPC_YN_CSYNC          , 0x02EC, 2 },
        { nMSP_DPC_CP_CSYNC          , 0x0010, 2 },
        { nMSP_DPC_CN_CSYNC          , 0x0114, 2 },
        { nMSP_DPC_INTR              , 0x0000, 2 },
        { nMSP_DPC_CLKCNTL           , 0x0010, 2 },
        { 0, 0, 0 }											/* Terminating entry */
    };
#endif /* SDK2X_SYSTEM_INITIALISE */

void sdk2x_SysInitTable (const sdk2x_RegisterTable_t *pTable)
{
	while (pTable->pointer)
	{
		switch (pTable->size)
		{
		case 1:
			*((unsigned char *)pTable->pointer) = (unsigned char)pTable->value;
			break;
		case 2:
			*((unsigned short *)pTable->pointer) = (unsigned short)pTable->value;
			break;
		case 4:
			*((unsigned int *)pTable->pointer) = (unsigned int)pTable->value;
			break;
		}
		pTable++;
	}
}

static unsigned int CalcPll (unsigned short pll)
{
	unsigned int temp1, temp2;

	temp1 = 7372800 * ((unsigned int)(((pll >> 8)&0xff)+8));
	temp2 = ((unsigned int)(((pll >> 2)&0x03f)+2)) * ( 1 << ((unsigned int)(pll & 3)));
	return temp1 / temp2;
}

unsigned int sdk2x_GetFPLL (void)
{
	return CalcPll (MSP_FPLLVSETREG);
}

unsigned int sdk2x_GetUPLL (void)
{
	return CalcPll (MSP_UPLLVSETREG);
}

unsigned int sdk2x_GetAPLL (void)
{
	return CalcPll (MSP_APLLVSETREG);
}

unsigned int sdk2x_SetFPLL (unsigned short FpllSetReg)
{
	int i;
	unsigned int imask;

	imask = MSP_INTMASK;

	/* Mask ALL interrupts */
	MSP_INTMASK = 0xFF8FFFE7;

	MSP_FPLLSETVREG = FpllSetReg;

	while (MSP_CLKCHGSTREG & 1);

	MSP_INTMASK = imask;

	for (i=0;(i<10000000) && (MSP_FPLLVSETREG != FpllSetReg); i++);

	return CalcPll (MSP_FPLLVSETREG);
}

unsigned int sdk2x_SetUPLL (unsigned short UpllSetReg)
{
	int i;
	unsigned int imask;

	imask = MSP_INTMASK;

	/* Mask ALL interrupts */
	MSP_INTMASK = 0xFF8FFFE7;

	MSP_UPLLSETVREG = UpllSetReg;

	while (MSP_CLKCHGSTREG & 2);

	MSP_INTMASK = imask;

	for (i=0;(i<10000000) && (MSP_UPLLVSETREG != UpllSetReg); i++);

	return CalcPll (MSP_UPLLVSETREG);
}

unsigned int sdk2x_SetAPLL (unsigned short ApllSetReg)
{
	int i;
	unsigned int imask;

	imask = MSP_INTMASK;

	/* Mask ALL interrupts */
	MSP_INTMASK = 0xFF8FFFE7;

	MSP_APLLSETVREG = ApllSetReg;

	while (MSP_CLKCHGSTREG & 4);

	MSP_INTMASK = imask;

	for (i=0;(i<10000000) && (MSP_APLLVSETREG != ApllSetReg); i++);

	return CalcPll (MSP_APLLVSETREG);
}

unsigned int sdk2x_SysReadKeys (void)
{
	unsigned short c,d,m;
	unsigned int newKeys;
	unsigned int ret;

	c = ~MSP_GPIOCPINLVL;
	d = ~MSP_GPIODPINLVL;
	m = ~MSP_GPIOMPINLVL;

	newKeys = 0;
	if (c & 0x0400)
		newKeys |= SDK2X_KEY_LEFT;
	if (c & 0x0800)
		newKeys |= SDK2X_KEY_RIGHT;
	if (m & 0x0001)
		newKeys |= SDK2X_KEY_JSUP;
	if (m & 0x0010)
		newKeys |= SDK2X_KEY_JSDN;
	if (m & 0x0004)
		newKeys |= SDK2X_KEY_JSLT;
	if (m & 0x0040)
		newKeys |= SDK2X_KEY_JSRT;
	if (m & 0x0002)
		newKeys |= SDK2X_KEY_JSUPLT;
	if (m & 0x0080)
		newKeys |= SDK2X_KEY_JSUPRT;
	if (m & 0x0008)
		newKeys |= SDK2X_KEY_JSDNLT;
	if (m & 0x0020)
		newKeys |= SDK2X_KEY_JSDNRT;
	if (d & 0x0800)
		newKeys |= SDK2X_KEY_JSPRESS;
	if (d & 0x0080)
		newKeys |= SDK2X_KEY_VOLUP;
	if (d & 0x0040)
		newKeys |= SDK2X_KEY_VOLDN;
	if (c & 0x4000)
		newKeys |= SDK2X_KEY_X;
	if (c & 0x8000)
		newKeys |= SDK2X_KEY_Y;
	if (c & 0x1000)
		newKeys |= SDK2X_KEY_A;
	if (c & 0x2000)
		newKeys |= SDK2X_KEY_B;
	if (c & 0x0200)
		newKeys |= SDK2X_KEY_SELECT;
	if (c & 0x0100)
		newKeys |= SDK2X_KEY_START;

	if (newKeys & SDK2X_KEY_JSUP)
		newKeys &= (~(SDK2X_KEY_JSUPLT | SDK2X_KEY_JSUPRT));
	if (newKeys & SDK2X_KEY_JSDN)
		newKeys &= (~(SDK2X_KEY_JSDNLT | SDK2X_KEY_JSDNRT));
	if (newKeys & SDK2X_KEY_JSLT)
		newKeys &= (~(SDK2X_KEY_JSUPLT | SDK2X_KEY_JSDNLT));
	if (newKeys & SDK2X_KEY_JSRT)
		newKeys &= (~(SDK2X_KEY_JSUPRT | SDK2X_KEY_JSDNRT));

	ret = newKeys;// | oldKeys;
	//oldKeys = newKeys;
	return ret;
}

