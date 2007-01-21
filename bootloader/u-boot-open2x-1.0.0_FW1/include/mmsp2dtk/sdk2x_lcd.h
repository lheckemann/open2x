#ifndef SDK2X_LCD_H
#define SDK2X_LCD_H

/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Header: sdk2x_lcd                                                   *
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

/*
	File: sdk2x_lcd.h

	Provides functions for setting up and maintaining the LCD controller module, setting the 8-bit palette,
	and sending text to the screen.
*/

/*
	Function: sdk2x_LcdDisable
	Shut down the CPU's LCD controller.
	This will disable the output signals to the LCD, which may cause it to display ugly patterns.
*/
void sdk2x_LcdDisable (void);

/*
	Function: sdk2x_LcdEnable
	Activate the CPU's LCD controller.
	This will enable the output signals to the LCD.
*/
void sdk2x_LcdEnable (void);

/*
	Function: sdk2x_LcdInit
	Initialise the CPU's LCD controller.

	Parameters:
	n16BPP - Colour depth. If 0, then 8-bpp, palette mode is used. If non-zero, then 16-bpp, high-colour mode is used.
	nBufferAddress - Initial address for the LCD frame buffer.

	Sets up the special function registers and enables the LCD so that the frame at nBufferAddress is displayed.
*/
void sdk2x_LcdInit (int n16BPP, unsigned int nBufferAddress);

/*
	Function: sdk2x_LcdEnable
	Activate the CPU's LCD controller.
	This will enable the output signals to the LCD.

	Parameters:
	none.

	Returns:
	none.
*/
void sdk2x_LcdSetFrameBuffer (unsigned int nBufferAddress);

/*
	Function: sdk2x_LcdEnable
	Activate the CPU's LCD controller.
	This will enable the output signals to the LCD.

	Parameters:
	none.

	Returns:
	none.
*/
void sdk2x_LcdRealisePalette16 (const unsigned short *pPalette);

/*
	Function: sdk2x_LcdEnable
	Activate the CPU's LCD controller.
	This will enable the output signals to the LCD.

	Parameters:
	none.

	Returns:
	none.
*/
void sdk2x_LcdTextOut (int nX, int nY, const char *pText, unsigned int nColour);

/*
	Function: sdk2x_LcdEnable
	Activate the CPU's LCD controller.
	This will enable the output signals to the LCD.

	Parameters:
	none.

	Returns:
	none.
*/
void sdk2x_LcdSetPrintBase (unsigned int nBufferAddress);

#endif /* SDK2X_LCD_H */

