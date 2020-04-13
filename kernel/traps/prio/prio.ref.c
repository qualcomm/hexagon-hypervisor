/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <prio.h>
#include <runlist.h>
#include <check_sanity.h>
#include <hw.h>

#if 0
s32_t H2K_prio_set(H2K_thread_context *dest, u32_t prio, H2K_thread_context *me)
{
	s32_t ret;
	if (prio > MAX_PRIO) prio = MAX_PRIO;
	if (dest == me) {
		H2K_runlist_remove(me);
		ret = me->prio;
		me->prio = prio;
		H2K_runlist_push(me);
	} else {
		/* UNIMPLEMENTED */
		ret = -1;
	}
	return ret;
}
#else
s32_t H2K_prio_set(H2K_thread_context *dest, u32_t prio, H2K_thread_context *me)
{
	s32_t ret = me->base_prio;
	if (prio > MAX_PRIO) return -1;
	if ((me->vmblock != NULL) && (prio < me->vmblock->bestprio)) return -1;
	me->base_prio = (u8_t)prio;
	return ret;
}
#endif

u32_t H2K_prio_get(unsigned int threadid_in, H2K_thread_context *me)
{
	/* Return this, or base prio? */
	return me->base_prio;
}

