/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <prio.h>
#include <hw.h>

s32_t H2K_prio_set(u32_t prio, H2K_thread_context *me)
{
	s32_t ret = me->base_prio;
	if (prio > MAX_PRIO) return -1;
	if ((me->vmblock != NULL) && (prio < me->vmblock->bestprio)) return -1;
	me->base_prio = (u8_t)prio;
	if (me->base_prio < me->prio) me->prio = me->base_prio;
	return ret;
}


u32_t H2K_prio_get(H2K_thread_context *me)
{
	return me->base_prio;
}
