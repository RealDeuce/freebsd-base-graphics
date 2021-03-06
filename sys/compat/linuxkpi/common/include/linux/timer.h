/*-
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013, 2014 Mellanox Technologies, Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */
#ifndef _LINUX_TIMER_H_
#define	_LINUX_TIMER_H_

#include <linux/types.h>

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/callout.h>

#include <linux/list.h>
#include <linux/ktime.h>

/*
 * A deferrable timer will work normally when the system is busy, but
 * will not cause a CPU to come out of idle just to service it; instead,
 * the timer will be serviced when the CPU eventually wakes up with a
 * subsequent non-deferrable timer.
 *
 * An irqsafe timer is executed with IRQ disabled and it's safe to wait for
 * the completion of the running instance from IRQ handlers, for example,
 * by calling del_timer_sync().
 *
 * Note: The irq disabled callback execution is a special case for
 * workqueue locking issues. It's not meant for executing random crap
 * with interrupts disabled. Abuse is monitored!
 */
#define TIMER_CPUMASK		0x0003FFFF
#define TIMER_MIGRATING		0x00040000
#define TIMER_BASEMASK		(TIMER_CPUMASK | TIMER_MIGRATING)
#define TIMER_DEFERRABLE	0x00080000
#define TIMER_PINNED		0x00100000
#define TIMER_IRQSAFE		0x00200000
#define TIMER_ARRAYSHIFT	22
#define TIMER_ARRAYMASK		0xFFC00000


struct timer_list {
	struct callout timer_callout;
	void    (*function) (unsigned long);
	unsigned long data;
	unsigned long expires;
};

extern unsigned long linux_timer_hz_mask;

#define	setup_timer(timer, func, dat)					\
do {									\
	(timer)->function = (typeof((timer)->function))(func);		\
	(timer)->data = (dat);						\
	callout_init(&(timer)->timer_callout, 1);			\
} while (0)

#define __setup_timer(timer, func, dat, flags) setup_timer(timer, func, dat)

#define setup_timer_on_stack setup_timer

#define	init_timer(timer)						\
do {									\
	(timer)->function = NULL;					\
	(timer)->data = 0;						\
	callout_init(&(timer)->timer_callout, 1);			\
} while (0)


#define init_timer_on_stack init_timer


extern void mod_timer(struct timer_list *, unsigned long);
extern void add_timer(struct timer_list *);
extern void add_timer_on(struct timer_list *, int cpu);

#define	del_timer(timer)	(callout_stop(&(timer)->timer_callout) == 1)
#define	del_timer_sync(timer)	(callout_drain(&(timer)->timer_callout) == 1)
#define del_singleshot_timer_sync(t) del_timer_sync(t)
#define	timer_pending(timer)	callout_pending(&(timer)->timer_callout)
#define	round_jiffies(j) \
	((unsigned long)(((j) + linux_timer_hz_mask) & ~linux_timer_hz_mask))
#define	round_jiffies_relative(j) \
	round_jiffies(j)

#define	round_jiffies_up(j)		round_jiffies(j) /* TODO */
#define	round_jiffies_up_relative(j)	round_jiffies_up(j) /* TODO */

#define mod_timer_pinned(timer, exp)  mod_timer(timer, exp)

static inline void destroy_timer_on_stack(struct timer_list *timer) { }


#endif					/* _LINUX_TIMER_H_ */
