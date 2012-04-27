/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <globals.h>
#include <timer.h>
#include <tree.h>
#include <hw.h>
#include <atomic.h>
#include <cpuint.h>

/* EJP: FIXME: locking for timeinfo */
/* EJP: FIXME: cancel timer carefully on thread death */

/* ***************** HW INTERFACES ********************* */

#if ARCHV <= 4

#define HW_COUNT 1
#define HW_MATCH 0
#define HW_ENABLE 2
#define TICK_GRANULARITY 4

#define TIMERHW_TICK_REALFREQ	19200000
#define TIMERHW_NSEC_PER_TICK 	52
//#define TIMERHW_NSEC_PER_TICK 	53
//#define TIMERHW_NSEC_PER_TICK 	56
#define TIMERHW_NSEC_FREQ 	(TIMERHW_TICK_REALFREQ * TIMERHW_NSEC_PER_TICK)
#define TIMERHW_NS2TICKS(X)	((X)/(TIMERHW_NSEC_PER_TICK))
#define TIMERHW_TICKS2NS(X)	((X)*TIMERHW_NSEC_PER_TICK)

static inline void H2K_timer_hw_soft_raise()
{
	/* EJP: FIXME: Implement this */
}

static inline ticks_t H2K_timer_hw_read_count()
{
	u32_t counter = H2K_gp->time.devptr[HW_COUNT];
	u64_t ticks = H2K_gp->time.last_ticks;
	u32_t lastcounter = ticks;
	if (counter < lastcounter) {
		ticks += (1ULL << 32);
	}
	ticks &= 0xFFFFFFFF00000000ULL;
	ticks |= counter;
	return ticks;
}

/*
 * Set HW counter: 
 *  * If in the past or close to now, raise the timer interrupt ourselves.
 *  * If far in the future, set the timer interrupt sooner to avoid overflow.
 *    The interrupt will reenable for the appropriate time.
 */

/* EJP: FIXME: what sets time? set nextticks & hw ?? */

static inline ticks_t H2K_timer_hw_set_timeout(u64_t nextticks)
{
	u64_t nowticks = H2K_timer_hw_read_count();
	u64_t max_future = nowticks + 0x0FFFFC000ULL;
	if (nextticks < nowticks) nextticks = nowticks;
	if (nextticks > max_future) nextticks = max_future;
	if ((nextticks - nowticks) > TICK_GRANULARITY) {
		H2K_gp->time.devptr[HW_MATCH] = nextticks;
		H2K_gp->time.next_ticks = nextticks;
	} else {
		/* EXPIRE NOW */
		H2K_timer_hw_soft_raise();
	}
	return nowticks;
}

static void H2K_timer_hw_init()
{
	H2K_gp->time.last_ticks = H2K_TIME_BIGBANG;
	H2K_gp->time.last_pcycles = 0;
	H2K_timer_hw_set_timeout(H2K_TIME_FOREVER);
	H2K_gp->time.devptr[HW_ENABLE] = 1;
}

#else /* <= V4 TIMERS */
#fatal Insert code for V5 timers here
#endif

/* ***************** KERNEL INFRASTRUCTURE ********************* */

static inline void H2K_timer_update_time(ticks_t nowticks)
{
	if (nowticks > H2K_gp->time.last_ticks) {
		H2K_gp->time.last_ticks = nowticks;
		H2K_gp->time.last_pcycles = H2K_get_pcycle_reg();
	} /* ELSE TIME WENT BACKWARDS! */
}

static inline void H2K_timer_update_timeout(ticks_t nextticks)
{
	if (H2K_gp->time.next_ticks > nextticks) {
		H2K_timer_hw_set_timeout(nextticks);
	};
}

/* 
 * Get time from device, update timeinfo
 */
static ticks_t H2K_timer_get_ticks()
{
	ticks_t nowticks;
	nowticks = H2K_timer_hw_read_count();
	H2K_timer_update_time(nowticks);
	return nowticks;
}

static void H2K_timer_dotimeout(H2K_treenode_t *treenode, void *me)
{
	H2K_thread_context *dest = containerof(H2K_thread_context,tree,treenode);
	dest->timeout = H2K_TIME_BIGBANG;
	H2K_vm_cpuint_post(dest->vmblock,dest,H2K_TIME_GUESTINT);
}

void H2K_timer_int(u32_t unused, H2K_thread_context *me, u32_t hwtnum)
{
	u64_t ticks,nextticks;
	H2K_treenode_t *timedout = NULL;
	H2K_treenode_t *newtimeout = NULL;
	/* Get time off the device, or assume it's the next time? */
	ticks = H2K_timer_get_ticks();
	ticks += TICK_GRANULARITY;
	/* Bisect tree according to time */

	H2K_spinlock_lock(&H2K_gp->time.lock);
	H2K_tree_bisect(&timedout,&H2K_gp->time.timeouts,H2K_gp->time.timeouts,ticks);
	newtimeout = H2K_tree_min(H2K_gp->time.timeouts);
	H2K_spinlock_unlock(&H2K_gp->time.lock);

	if (newtimeout) nextticks = newtimeout->key;
	else nextticks = H2K_TIME_FOREVER;
	H2K_timer_hw_set_timeout(nextticks);

	/* Do Timeouts.  Make preemptible? cpu_offline? Better be fast, at least. */
	H2K_tree_destructive_iterate(timedout,me,H2K_timer_dotimeout);
}

void H2K_timer_init()
{
	/* Register fastint */
	H2K_spinlock_init(&H2K_gp->time.lock);
	H2K_gp->time.timeouts = NULL;
	if (H2K_gp->time.devptr == NULL) H2K_gp->time.devptr = (void *)(TIMER_BASE_VA);
	H2K_timer_hw_init();
	H2K_timer_get_ticks();
}

/* ***************** USER INTERFACES ********************* */

static u64_t H2K_timer_get_freq(u32_t unused, u64_t unused2, H2K_thread_context *me)
{
	return TIMERHW_NSEC_FREQ;
}

static u64_t H2K_timer_get_resolution(u32_t unused, u64_t unused2, H2K_thread_context *me)
{
	return TIMERHW_NSEC_PER_TICK;
}

static u64_t H2K_timer_get_roughtime(u32_t unused, u64_t unused2, H2K_thread_context *me)
{
	return H2K_gp->time.last_ticks * TIMERHW_NSEC_PER_TICK 
		/* + pcycles since last tick? */;
}

static u64_t H2K_timer_get_time(u32_t unused, u64_t unused2, H2K_thread_context *me)
{
	return H2K_timer_get_ticks() * TIMERHW_NSEC_PER_TICK;
}

static u64_t H2K_timer_get_timeout(u32_t unused, u64_t unused2, H2K_thread_context *me)
{
	return me->timeout * TIMERHW_NSEC_PER_TICK;
}

static u64_t H2K_timer_set_timeout_tick(u32_t unused, u64_t timeout_tick, H2K_thread_context *me)
{
	/* If already registered for a timer, remove from tree */
	if (me->timeout) H2K_tree_remove(&H2K_gp->time.timeouts,&me->tree);
	if (timeout_tick <= H2K_gp->time.last_ticks) timeout_tick = H2K_TIME_BIGBANG;
	me->timeout = timeout_tick;
	if (timeout_tick != H2K_TIME_BIGBANG) H2K_tree_add(&H2K_gp->time.timeouts,&me->tree);
	H2K_timer_update_timeout(timeout_tick);
	return TIMERHW_TICKS2NS(timeout_tick);
}

static u64_t H2K_timer_set_timeout(u32_t unused, u64_t timeout, H2K_thread_context *me)
{
	u64_t timeout_tick = TIMERHW_NS2TICKS(timeout);
	if (timeout == H2K_TIME_FOREVER) timeout_tick = H2K_TIME_BIGBANG;
	return H2K_timer_set_timeout_tick(0,timeout_tick,me);
}

static u64_t H2K_timer_set_timeout_sooner(u32_t unused, u64_t timeout, H2K_thread_context *me)
{
	u64_t timeout_tick = TIMERHW_NS2TICKS(timeout);
	if (timeout_tick >= me->timeout) return TIMERHW_TICKS2NS(me->timeout);
	return H2K_timer_set_timeout_tick(0,timeout_tick,me);
}

typedef u64_t (*H2K_timer_handler_t)(u32_t unused, u64_t arg, H2K_thread_context *me);

static H2K_timer_handler_t H2K_timer_trap_handlers[] = {
	H2K_timer_get_freq,
	H2K_timer_get_resolution,
	H2K_timer_get_roughtime,
	H2K_timer_get_time,
	H2K_timer_get_timeout,
	H2K_timer_set_timeout,
	H2K_timer_set_timeout_sooner,
};

u64_t H2K_timer_trap(u32_t traptype, u64_t arg, H2K_thread_context *me)
{
	if (traptype >= H2K_TIMER_TRAP_INVALID) return ~0ULL;
	return H2K_timer_trap_handlers[traptype](traptype,arg,me);
}

