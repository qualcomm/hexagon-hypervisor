/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <prio.h>
#include <runlist.h>

u32_t BLASTK_prio_set(BLASTK_thread_context *dest, u32_t prio, BLASTK_thread_context *me)
{
	u32_t ret;
	if (prio > MAX_PRIOS) prio = MAX_PRIOS;
	if (dest == me) {
		BLASTK_runlist_remove(me);
		ret = me->prio;
		me->prio = prio;
		BLASTK_runlist_push(me);
	} else {
		/* UNIMPLEMENTED */
		ret = prio;
	}
	return ret;
}

u32_t BLASTK_prio_get(BLASTK_thread_context *me)
{
	return me->prio;
}

