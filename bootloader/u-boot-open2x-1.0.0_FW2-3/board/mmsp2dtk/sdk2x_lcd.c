/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Implementation: sdk2x_lcd                                           *
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

#include "mmsp2dtk/mmsp2.h"
#include "mmsp2dtk/sdk2x_priv.h"

static unsigned char *pPrintBase;
static int nIs16Bpp = 1;

void sdk2x_LcdTextOut (int nX, int nY, const char *pText, unsigned int nColour)
{
	unsigned char *addr8, *end8;
	unsigned short *addr16, *end16;
	int i;
	unsigned char mask;
	unsigned char byte1;
	unsigned char byte2;
	unsigned char *pByte;

#ifdef LCD_TEXT_OUT_SERIAL_ECHO
	sdk2x_UartSendString(0,text);
	sdk2x_UartSendString(0,"\r\n");
#endif

	addr16 = (unsigned short *)(pPrintBase + (nY * 640) + (nX << 1));
	end16 = (unsigned short *)(pPrintBase + 153600);
	addr8 = pPrintBase + (nY * 320) + nX;
	end8 = pPrintBase + 76800;

	while (*pText)
	{
		pByte = ((unsigned char *)fontresEng) + ((*pText)<<4);
		for (i=0; i<8; i++)
		{
			byte1=*pByte;
			pByte++;
			byte2=*pByte;
			pByte++;
			mask=128;
			if (nIs16Bpp)
			{
				while (mask)
				{
					if (addr16 >= end16)
						return;
					if (byte2 & mask)
						*addr16 = (unsigned short)nColour;
					addr16 += 320;
					mask >>= 1;
				}
				mask=128;
				while (mask)
				{
					if (addr16 >= end16 )
						return;
					if (byte1 & mask)
						*addr16 = (unsigned short)nColour;
					addr16 += 320;
					mask >>= 1;
				}
				addr16 -= 5119;
			}
			else
			{
				while (mask)
				{
					if (addr8 >= end8)
						return;
					if (byte2 & mask)
						*addr8 = (unsigned char)nColour;
					addr8 += 320;
					mask >>= 1;
				}
				mask=128;
				while (mask)
				{
					if (addr8 >= end8)
						return;
					if (byte1 & mask)
						*addr8 = (unsigned char)nColour;
					addr8 += 320;
					mask>>=1;
				}
				addr8 -= 5119;
			}
		}
		pText++;
	}
}

void sdk2x_LcdSetPrintBase (unsigned int nBufferAddress)
{
	pPrintBase = (unsigned char *)nBufferAddress;
}

