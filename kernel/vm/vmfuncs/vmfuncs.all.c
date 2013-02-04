/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <create.h>

/* 18 */
void H2K_vmtrap_start(H2K_thread_context *me)
{
	s32_t relprio = (s32_t)me->r02;
	s32_t prio = (s32_t)me->base_prio + relprio;

	if ( relprio < -MAX_PRIO || relprio > MAX_PRIO) {
		me->r00 = -1;
		return;
	}
	
	/* FIXME: need to pass arg1?  use vmblock bestprio instead of base_prio? */
	                               /*      pc       sp  arg1 */
	me->r00 = H2K_thread_create_no_squash(me->r00, me->r01, 0, (u32_t)prio, me->vmblock, me);
}

