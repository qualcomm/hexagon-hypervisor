/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_thread.S
 * 
 * This file contains the implementation for 
 * h2_thread functionality 
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

/* call trap if ugp == 0, else return ugp[0] */
FUNC_START h2_thread_myid .text.h2.thread .p2align 4
	r1 = ugp
	{
		p0 = cmp.eq(r1,#0)
		if (p0.new) jump:nt 1f
		if (!p0.new) r0 = memw(r1)
	}
	jumpr r31
1:
	trap0(#H2_TRAP_THREAD_ID)
	jumpr r31
FUNC_END h2_thread_myid

FUNC_START h2_thread_create .text.h2.thread .p2align 2
	trap0(#H2_TRAP_THREAD_CREATE)
	jumpr r31
FUNC_END h2_thread_create

FUNC_START h2_thread_stop .text.h2.thread .p2align 2
	trap0(#H2_TRAP_THREAD_STOP)
	jumpr r31
FUNC_END h2_thread_stop

FUNC_START h2_yield .text.h2.thread .p2align 2
	trap0(#H2_TRAP_YIELD)
	jumpr r31
FUNC_END h2_yield

FUNC_START h2_thread_set_tid .text.h2.thread .p2align 2
	trap0(#H2_TRAP_SET_TID)
	jumpr r31
FUNC_END h2_thread_set_tid

FUNC_START h2_thread_get_tid .text.h2.thread .p2align 2
	trap0(#H2_TRAP_GET_TID)
	jumpr r31
FUNC_END h2_thread_get_tid

FUNC_START h2_thread_state .text.h2.thread .p2align 2
	trap0(#H2_TRAP_THREAD_STATE)
	jumpr r31
FUNC_END h2_thread_state

