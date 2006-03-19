/*
 *  linux/include/asm-arm/arch-mmsp2/time.h
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ASM_ARCH_TIME_H_
#define _ASM_ARCH_TIME_H_

static inline unsigned long mmsp2_get_rtc_time(void)
{
	return RTCTCNT;
}

static int mmsp2_set_rtc(void)
{
	unsigned long current_time = xtime.tv_sec;

	if (RTCCTRL & RTC_ALE) {
		/* make sure not to forward the clock over an alarm */
		unsigned long alarm = ALARMT;
		if (current_time >= alarm && alarm >= RTCTCNT)
			return -ERESTARTSYS;
	}
	/* plus one because RTCTSET value is set to RTCTCNT after 1 sec */
	RTCTSET = current_time + 1;
	return 0;
}

/* IRQs are disabled before entering here from do_gettimeofday() */
static unsigned long mmsp2_gettimeoffset(void)
{
	unsigned long ticks_to_match, elapsed, usec;

	/* Get ticks before next timer match */
	ticks_to_match = TMATCH0 - TCOUNT;

	/* We need elapsed ticks since last match */
	elapsed = LATCH - ticks_to_match;

	/* Now convert them to usec */
	usec = (unsigned long)(elapsed*tick)/LATCH;

	return usec;
}

static void mmsp2_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	long flags;
	int next_match;
	
	do_profile(regs);

	/* Loop until we get ahead of the free running timer.
	 * This ensures an exact clock tick count and time accuracy.
	 * IRQs are disabled inside the loop to ensure coherence between
	 * lost_ticks (updated in do_timer()) and the match reg value, so we
	 * can use do_gettimeofday() from interrupt handlers.
	 */
	 
	do {
		do_leds();
		do_set_rtc();
		save_flags_cli(flags);
		do_timer(regs);
		TSTATUS = TCNT0;	/* Clear match on timer 0 */
		next_match = (TMATCH0 += LATCH);
		restore_flags(flags);
//		do_while_count++;
  } while ((signed long)(next_match - TCOUNT) <= 8);
/*
	zero_tcounter++;
  if( zero_tcounter >= TCOUNT_ZERO_SET_COUNT)
  {
    printk(KERN_CRIT "RTC .%u:%u\n", RTCSTCNT, do_while_count);
    do_while_count = 0;
    zero_tcounter = 0;
  }
*/
}

#define WORKAROUND_CPU_BUG_TCOUNT
extern inline void setup_timer(void)
{
	gettimeoffset = mmsp2_gettimeoffset;
	timer_irq.handler = mmsp2_timer_interrupt;
	TMATCH0 = 0;		/* set initial math at 0 */
	TSTATUS = 0xf;		/* clear status on all timers */
	setup_arm_irq(IRQ_TIMER0, &timer_irq);
	TCONTROL &= ~(TIMER_EN);	/* diable all timers */
	/* initialize free-running timer, force first match */
#ifdef WORKAROUND_CPU_BUG_TCOUNT
	TCOUNT = 1;
	{ int i; for (i = 10; i > 0; i--) ; }
	TCOUNT = 0;
#else
	TCOUNT = 0;
#endif
	TCONTROL |= TIMER_EN;	/* enable all timers */
}

#endif /* _ASM_ARCH_TIME_H_ */
