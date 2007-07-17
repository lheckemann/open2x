#ifndef SDK2X_SD_H
#define SDK2X_SD_H

/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Header: sdk2x_sd                                                    *
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
#include <linux/string.h>

struct stat;

/* If updating this list of errors, make sure that you renumber correctly, and update SDK2X_ERR_SDPT_MIN and MAX below
 * and also syscalls.c */
#define SDK2X_ERR_SDFS_MIN								(100)
#define SDK2X_ERR_SDFS_MAX								(SDK2X_ERR_SDFS_MIN + 0)

#define SDK2X_ERR_SDFS_FILENAME_TOO_LONG			(SDK2X_ERR_SDFS_MIN + 0)

#define SDK2X_SD_DEBUG 					(0)

#define SDK2X_SD_PATH_MAX				(260)
#define SDK2X_SD_FILENAME_MAX			(260)

/*
	File: sdk2x_sd.h

	Provides functions for initialising the SD/MMC interface, and for reading/writing data to and from the card.
*/

/*
	Function: sdk2x_SdInit
	Initialise the SD code. Call this before trying to access the card.
*/
int sdk2x_SdInit (void);

/*
	Function: sdk2x_SdShutdown
	Shut down the SD code and release all resources. Call this before changing cards.
*/
void sdk2x_SdShutdown (void);

/*
	Function: sdk2x_SdOpen
	Open a file.
*/
int sdk2x_SdOpen(const char *pName, int nFlags, int *pFD);

/*
	Function: sdk2x_SdClose
	Close a file.
*/
int sdk2x_SdClose(int nFD);

/*
	Function: sdk2x_SdRead
	Read data from a file.
*/
int sdk2x_SdRead(int nFD, void *pBuf, size_t nSize, int *pActual);

/*
	Function: sdk2x_SdWrite
	Write data to a file.
*/
int sdk2x_SdWrite(int nFD, const void *pBuf, size_t nSize, int *pActual);

/*
	Function: sdk2x_SdLSeek
	Set a file's current read/write offset.
*/
int sdk2x_SdLSeek (int nFD, off_t nOffset, int Whence, off_t *pActual);

/*
	Function: sdk2x_SdUnlink
	Delete a file.
*/
int sdk2x_SdUnlink(const char *pName);

/*
	Function: sdk2x_SdFStat
	Return information about a currently open file.
*/
int sdk2x_SdFStat(int nFD, struct stat *st);

/*
	Function: sdk2x_SdStat
	Return information about a named file.
*/
int sdk2x_SdStat(const char *pName, struct stat *st);

/*
	Function: sdk2x_SdChDir
	Change the current working directory.
*/
int sdk2x_SdChDir(const char *pName);

/*
	Function: sdk2x_SdMkDir
	Create a directory.
*/
int sdk2x_SdMkDir(const char *pName);

/*
	Function: sdk2x_SdRmDir
	Remove a directory, which must already be empty.
*/
int sdk2x_SdRmDir(const char *pName);

#endif /* SDK2X_SD_H */

