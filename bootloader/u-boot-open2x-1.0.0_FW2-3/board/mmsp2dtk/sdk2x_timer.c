/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Implementation: sdk2x_timer                                         *
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

void sdk2x_SysTimerEnable(void)
{
	unsigned int temp;
	/* Clear any timer interrupts that may be pending, and disable the
	* interrupt in the timer module */
	MSP_TSTATUS = 8;
	MSP_TINTEN = 0;
	MSP_SRCPEND = 32;
	MSP_INTPEND = 32;

	/* Disable the Timer module interrupt */
	MSP_INTMASK |= 0x20;

	/* Enable the timer */
	MSP_TCONTROL = 1;

	/* Enable the clock into the timer module */
	MSP_SYSCLKENREG |= 0x80;

	/* ART103: Don't ask my why we have to do this, but the first write to 	MSP_TCOUNT is a dummy write... 
	           You have to wait one cuont before writing it correctly */
	MSP_TCOUNT = 0;
	temp = MSP_TCOUNT;
	while (temp == MSP_TCOUNT);
	MSP_TCOUNT = 0;
}

int sdk2x_GetCurrentTime (void)
{
	return (int)MSP_TCOUNT;
}

int sdk2x_TimerWait(const int TimeFrom, const int uSecs, const int max)
{
	int from = TimeFrom;
	int targetDiff = uSecs;
	int newDiff = -1;
	int now;
	long long temp;

	now = (int)MSP_TCOUNT;

	/* Special case: if TimeFrom is zero, just do uSecs from now */
	if (!from)
		from = now;

	/* If TimeFrom is in the future, set it to now and extend the delay period */
	if ((from - now) > 0)
	{
		targetDiff += (from - now);
		from = now;
	}

	/* Keep targetDiff to a sane range */
	if ((targetDiff > max) || (targetDiff < 0))
		targetDiff = max;

	/* Multiply targetDiff by 7.3728. To make it simple, multiply by 7550 and divide by 1024. */
	temp = ((long long)targetDiff) * 7550LL;
	targetDiff = (int)(temp >> 10);

	/* Wait for the time to elapse. */
	while (newDiff < targetDiff)
		newDiff = ((int)MSP_TCOUNT) - from;

	return (int)MSP_TCOUNT;
}

int sdk2x_USleep(const int uSecs)
{
	return sdk2x_TimerWait(0,uSecs,1000000000);
}

int sdk2x_MSleep(const int mSecs)
{
	int i;

	for (i=0; i<mSecs; i++)
		sdk2x_USleep(1000);

	return (int)MSP_TCOUNT;
}

int sdk2x_Sleep(const int Secs)
{
	int i;

	for (i=0; i<Secs; i++)
		sdk2x_MSleep(1000);

	return (int)MSP_TCOUNT;
}

int sdk2x_GetElapsedMSecs (const int FromTime)
{
	long long temp;

	temp = (long long)(((int)MSP_TCOUNT) - FromTime);
	temp <<= 10;
	temp /= 7550000LL;

	return (int)temp;
}

