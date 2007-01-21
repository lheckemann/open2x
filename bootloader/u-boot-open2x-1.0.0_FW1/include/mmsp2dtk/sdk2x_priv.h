#ifndef SDK2X_PRIV_H_INCLUDED
#define SDK2X_PRIV_H_INCLUDED

/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Header: sdk2x_priv                                                  *
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

#include "sdk2x.h"

/* This variable is defined in sdk2x_system.c */
extern char sdk2x_private_string [200];
extern const unsigned char fontresEng[];

typedef struct _tag_sdk2c_RegisterTable
{
	unsigned int pointer;
	unsigned int value;
	int size;
} sdk2x_RegisterTable_t;

void sdk2x_SysInitTable (const sdk2x_RegisterTable_t *pTable);


#endif /* SDK2X_PRIV_H_INCLUDED */
