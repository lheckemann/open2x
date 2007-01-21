#ifndef SDK2X_TIMER_H
#define SDK2X_TIMER_H

/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Header: sdk2x_timer                                                 *
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

void sdk2x_SysTimerEnable(void);
int sdk2x_GetCurrentTime (void);
int sdk2x_TimerWait(const int TimeFrom, const int uSecs, const int max);
int sdk2x_USleep(const int uSecs);
int sdk2x_MSleep(const int mSecs);
int sdk2x_Sleep(const int Secs);
int sdk2x_GetElapsedMSecs (const int FromTime);

#endif /* SDK2X_TIMER_H */

