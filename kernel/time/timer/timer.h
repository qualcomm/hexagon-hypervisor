/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TIMER_H
#define H2K_TIMER_H 1

#include <context.h>
#include <timeinfo.h>
#include <tree.h>
#include <hw.h>

void H2K_timer_int(u32_t unused, H2K_thread_context *me, u32_t hwtnum);
void H2K_timer_init();

static inline void H2K_timer_cancel_withlock(H2K_thread_context *me)
{
	if (me->timeout) H2K_tree_remove(&H2K_gp->time.timeouts,&me->tree);
}

static inline void H2K_timer_cancel(H2K_thread_context *me)
{
	BKL_LOCK();
	H2K_timer_cancel_withlock(me);
	BKL_UNLOCK();
}

enum {
	H2K_TIMER_TRAP_GET_FREQ,
	H2K_TIMER_TRAP_GET_RESOLUTION,
	H2K_TIMER_TRAP_GET_TIME,
	H2K_TIMER_TRAP_GET_TIMEOUT,
	H2K_TIMER_TRAP_SET_TIMEOUT,
	H2K_TIMER_TRAP_DELTA_TIMEOUT,
	H2K_TIMER_TRAP_INVALID
};

u64_t H2K_timer_trap(u32_t traptype, u64_t arg, H2K_thread_context *me);

#endif
