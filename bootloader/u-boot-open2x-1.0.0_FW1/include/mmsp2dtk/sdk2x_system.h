#ifndef SDK2X_SYSTEM_H
#define SDK2X_SYSTEM_H

/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Header: sdk2x_system                                                *
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

#define SDK2X_KEY_LEFT		(0x00000001)
#define SDK2X_KEY_RIGHT		(0x00000002)
#define SDK2X_KEY_JSUP		(0x00000004)
#define SDK2X_KEY_JSDN		(0x00000008)
#define SDK2X_KEY_JSLT		(0x00000010)
#define SDK2X_KEY_JSRT		(0x00000020)
#define SDK2X_KEY_JSUPLT	(0x00000040)
#define SDK2X_KEY_JSUPRT	(0x00000080)
#define SDK2X_KEY_JSDNLT	(0x00000100)
#define SDK2X_KEY_JSDNRT	(0x00000200)
#define SDK2X_KEY_JSPRESS	(0x00000400)
#define SDK2X_KEY_VOLUP		(0x00000800)
#define SDK2X_KEY_VOLDN		(0x00001000)
#define SDK2X_KEY_X			(0x00002000)
#define SDK2X_KEY_Y			(0x00004000)
#define SDK2X_KEY_A			(0x00008000)
#define SDK2X_KEY_B			(0x00010000)
#define SDK2X_KEY_SELECT	(0x00020000)
#define SDK2X_KEY_START		(0x00040000)

void sdk2x_SdkInit (void);
void sdk2x_SystemReboot (void);
unsigned int sdk2x_GetFPLL (void);
unsigned int sdk2x_GetUPLL (void);
unsigned int sdk2x_GetAPLL (void);
unsigned int sdk2x_SetFPLL (unsigned short FpllSetReg);
unsigned int sdk2x_SetUPLL (unsigned short UpllSetReg);
unsigned int sdk2x_SetAPLL (unsigned short ApllSetReg);
unsigned int sdk2x_SysReadKeys (void);
void sdk2x_SetupMMU (void);

#endif /* SDK2X_SYSTEM_H */

