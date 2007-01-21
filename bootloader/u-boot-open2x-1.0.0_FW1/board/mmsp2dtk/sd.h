//[*]----------------------------------------------------------------------------------------------------[*]
/*
 *      A     EEEE  SSSS   OOO  PPPP 
 *     A A    E     S     O   O P   P
 *    AAAAA   EEEE  SSSS  O   O PPPP
 *   A     A  E         S O   O P
 *   A     A  EEEE  SSSS   OOO  P
 *
 *  An Entertainment Solution On a Platform (AESOP) is a completely Open Source 
 *  based graphical user environment and suite of applications for PDAs and other 
 *  devices running Linux. It is included in various embedded Linux distributions 
 *  such as OpenZaurus - http://www.aesop-embedded.org 
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 *	Title           : sd.h
 *	Author          : 
 *	Created date    : 2005. 06. 26. 23:06:29 KST
 *	Description     : 
 *
 *	$Revision: 1.1.1.1 $
 *	$Log: sd.h,v $
 *	Revision 1.1.1.1  2005/06/27 17:03:53  linuxpark
 *	Initial import.
 *	
 *	
 *
 */ 
//[*]----------------------------------------------------------------------------------------------------[*]
#ident  "@(*) $Header: /cvsroot/aesop-embedded/u-boot-aesop/board/aesop2440/sd.h,v 1.1.1.1 2005/06/27 17:03:53 linuxpark Exp $"
//[*]----------------------------------------------------------------------------------------------------[*]
void SD_Write(unsigned char * pucdata, unsigned long ulLBA, unsigned long ulSectors);
void SD_Read(unsigned char * pucdata, unsigned long ulLBA, unsigned long ulSectors);
unsigned int SD_card_init(void);
//[*]----------------------------------------------------------------------------------------------------[*]
