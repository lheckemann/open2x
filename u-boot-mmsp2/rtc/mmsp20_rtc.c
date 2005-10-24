/*
 * (C) Copyright 2003
 * David Müller ELSOFT AG Switzerland. d.mueller@elsoft.ch
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Date & Time support for the built-in Samsung MMSP20 RTC
 */

#include <common.h>
#include <command.h>

#if defined(CONFIG_RTC_MMSP20) && (CONFIG_COMMANDS & CFG_CMD_DATE)

#include <rtc.h>
#include <mmsp20.h>
/*#define	DEBUG*/


/* this not support julian */
void rtc_get (struct rtc_time *tmp)
{
	MMSP20_RTC * const rtc = MMSP20_GetBase_RTC();
	
}
void rtc_set (struct rtc_time *tmp)
{
	MMSP20_RTC * const rtc = MMSP20_GetBase_RTC();

}

void rtc_reset (void)
{
	MMSP20_RTC * const rtc = MMSP20_GetBase_RTC();

	rtc->RTCTSETREG = 0x00000000;
}

/* ------------------------------------------------------------------------- */

#endif	/* CONFIG_RTC_MMSP20 && CFG_CMD_DATE */
