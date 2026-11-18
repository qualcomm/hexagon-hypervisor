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
#include <intcontrol.h>
#include <h2_common_timer.h>

/* EJP: FIXME: locking for timeinfo */
/* EJP: FIXME: cancel timer carefully on thread death */

/* ***************** HW INTERFACES ********************* */

#define HW_COUNT_LO (0x1000/4)
#define HW_COUNT_HI (0x1004/4)
#define HW_MATCH_LO (0x1020/4)
#define HW_MATCH_HI (0x1024/4)
#define HW_ENABLE   (0x102C/4)

#define HW_CNTFRQ	(0x0000/4)
#define HW_CNTSR	(0x0004/4)
#define HW_CNTACR	(0x0040/4)

#define TICK_GRANULARITY 4
#define TIMERHW_TICK_REALFREQ	19200000
#define TIMERHW_NSEC_PER_TICK 	52

#define TIMERHW_NSEC_FREQ 	(TIMERHW_TICK_REALFREQ * TIMERHW_NSEC_PER_TICK)
#define TIMERHW_NS2TICKS(X)	((X)/(TIMERHW_NSEC_PER_TICK))
#define TIMERHW_TICKS2NS(X)	((X)*TIMERHW_NSEC_PER_TICK)

static inline ticks_t H2K_timer_hw_read_count()
{
#ifndef TESTING
	return H2K_get_timer_reg();
#else
	u32_t tmphi;
	union {
		u64_t pair;
		struct {
			u32_t lo;
			u32_t hi;
		};
	} ticktmp;
	do {
		tmphi = H2K_gp->time.devptr[HW_COUNT_HI];
		ticktmp.lo = H2K_gp->time.devptr[HW_COUNT_LO];
		ticktmp.hi = H2K_gp->time.devptr[HW_COUNT_HI];
	} while (tmphi != ticktmp.hi);
	return ticktmp.pair;
#endif
}

/*
 * The new timer will raise the interrupt if it is in the past
 */
static inline void H2K_timer_hw_set_timeout(u64_t nextticks)
{
	H2K_gp->time.next_ticks = nextticks;
	H2K_gp->time.devptr[HW_MATCH_HI] = ~0;
	H2K_gp->time.devptr[HW_MATCH_LO] = (u32_t)nextticks;
	H2K_gp->time.devptr[HW_MATCH_HI] = nextticks >> 32;
}

#ifndef NO_DEVICES
static void H2K_timer_hw_init() //I think this should be lock free, since this is init code.
{
#ifdef TESTING // H2K_bzero already zeros all kernel struct.
	H2K_gp->time.last_ticks = H2K_TIME_BIGBANG;
	H2K_gp->time.last_pcycles = 0;
#endif
	H2K_timer_hw_set_timeout(H2K_TIME_FOREVER);
	H2K_gp->time.devptr[HW_CNTSR] = ~0;
	H2K_gp->time.devptr[HW_CNTACR] = ~0;
	H2K_gp->time.devptr[HW_CNTFRQ] = 19200000;
	H2K_gp->time.devptr[HW_ENABLE] = 1;
	H2K_intcontrol_enable(TIMER_INT_CORE);
}
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
	/* Ignore error from post to dead cpu */
	H2K_vm_cpuint_post_locked(dest->vmblock,dest,H2K_TIME_GUESTINT,dest->vmblock->intinfo);
}

void H2K_timer_int(u32_t unused, H2K_thread_context *me, u32_t hwtnum)
{
	u64_t ticks,nextticks;
	H2K_treenode_t *timedout = NULL;
	H2K_treenode_t *newtimeout = NULL;

	/* This looks a bit weird at first. The BKL is needed to assure the
	 * right order of lock acquisition. H2K_thread_stop_withlock holds
	 * the BKL and calls H2K_timer_cancel, which locks timer_lock. So the
	 * order which needs to be preserved is: lock(BKL) -> lock(timer_lock).
	 */
	BKL_LOCK();
	H2K_spinlock_lock(&H2K_gp->timer_lock);
	/* Get time off the device, or assume it's the next time? */
	ticks = H2K_timer_get_ticks();
	ticks += TICK_GRANULARITY;
	/* Bisect tree according to time */
	H2K_tree_bisect(&timedout,&H2K_gp->time.timeouts,H2K_gp->time.timeouts,ticks);
	newtimeout = H2K_tree_min(H2K_gp->time.timeouts);

	if (newtimeout) nextticks = newtimeout->key;
	else nextticks = H2K_TIME_FOREVER;
	H2K_timer_hw_set_timeout(nextticks);

	/* Do Timeouts.  Make preemptible? cpu_offline? Better be fast, at least. */
	H2K_tree_destructive_iterate(timedout,me,H2K_timer_dotimeout);
	BKL_UNLOCK();
	H2K_spinlock_unlock(&H2K_gp->timer_lock);
	H2K_intcontrol_enable(TIMER_INT_CORE);
}

void H2K_timer_init() {
#ifdef TESTING
	H2K_spinlock_init(&H2K_gp->timer_lock); // H2K_bzero already zeros all kernel struct.
#endif
#ifndef NO_DEVICES
#ifdef TESTING
	H2K_gp->time.timeouts = NULL;
#endif
  
	H2K_timer_hw_init();
	H2K_timer_get_ticks();

#endif
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

static u64_t H2K_timer_get_time(u32_t unused, u64_t unused2, H2K_thread_context *me)
{
	H2K_spinlock_lock(&H2K_gp->timer_lock);
	u64_t time = H2K_timer_get_ticks() * TIMERHW_NSEC_PER_TICK;
	H2K_spinlock_unlock(&H2K_gp->timer_lock);
	return time;
}

static u64_t H2K_timer_get_timeout(u32_t unused, u64_t unused2, H2K_thread_context *me)
{
	return me->timeout * TIMERHW_NSEC_PER_TICK;
}

static u64_t H2K_timer_set_timeout_tick_unlock(u32_t unused, u64_t timeout_tick, H2K_thread_context *me)
{
	/* If already registered for a timer, remove from tree */
	if (timeout_tick <= H2K_gp->time.last_ticks) timeout_tick = H2K_TIME_BIGBANG;
	if (me->timeout) H2K_tree_remove(&H2K_gp->time.timeouts,&me->tree);
	me->timeout = timeout_tick;
	/* 
	 * This seemed to be one of the more important fixes, but I think we should be wrapping 
	 * this inside the tree functions itself.
	 */
	me->tree.rightleft = 0;
	if (timeout_tick != H2K_TIME_BIGBANG) H2K_tree_add(&H2K_gp->time.timeouts,&me->tree);
	H2K_timer_update_timeout(timeout_tick);
	H2K_spinlock_unlock(&H2K_gp->timer_lock);
	return TIMERHW_TICKS2NS(timeout_tick);
}
static u64_t H2K_timer_set_timeout_tick(u32_t unused, u64_t timeout_tick, H2K_thread_context *me)
{
	H2K_spinlock_lock(&H2K_gp->timer_lock);
	return H2K_timer_set_timeout_tick_unlock(unused, timeout_tick, me);
}

static u64_t H2K_timer_set_timeout(u32_t unused, u64_t timeout, H2K_thread_context *me)
{
	u64_t timeout_tick = TIMERHW_NS2TICKS(timeout);
	/* Setting a timer for FOREVER is the same as BIGBANG... disable the timer */
	if (timeout == H2K_TIME_FOREVER) timeout_tick = H2K_TIME_BIGBANG;
	return H2K_timer_set_timeout_tick(0,timeout_tick,me);
}

static u64_t H2K_timer_delta_timeout(u32_t unused, u64_t delta, H2K_thread_context *me)
{
	u64_t delta_ticks = max(TIMERHW_NS2TICKS(delta),TICK_GRANULARITY);
	H2K_spinlock_lock(&H2K_gp->timer_lock);
	u64_t timeout_ticks = H2K_timer_get_ticks() + delta_ticks;
	if (delta == H2K_TIME_FOREVER) timeout_ticks = H2K_TIME_BIGBANG;
	return H2K_timer_set_timeout_tick_unlock(0,timeout_ticks,me);
}

typedef u64_t (*H2K_timer_handler_t)(u32_t unused, u64_t arg, H2K_thread_context *me);

static H2K_timer_handler_t H2K_timer_trap_handlers[] = {
	H2K_timer_get_freq,
	H2K_timer_get_resolution,
	H2K_timer_get_time,
	H2K_timer_get_timeout,
	H2K_timer_set_timeout,
	H2K_timer_delta_timeout,
};

u64_t H2K_timer_trap(u32_t traptype, u64_t arg, H2K_thread_context *me)
{
	if (traptype >= H2K_TIMER_TRAP_INVALID) return ~0ULL;
	return H2K_timer_trap_handlers[traptype](traptype,arg,me);
}

void H2K_vmtrap_timer(H2K_thread_context *me)
{
	me->r0100 = H2K_timer_trap(me->r00,me->r0302,me);
}

