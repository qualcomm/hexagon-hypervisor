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

#include "h2_trap_constants.h"

	.text
	.p2align 3
	.global h2_thread_create
h2_thread_create:
	trap0(#H2_TRAP_THREAD_CREATE)
	jumpr r31

	.global h2_thread_stop
h2_thread_stop:
	trap0(#H2_TRAP_THREAD_STOP)
	jumpr r31

	.global h2_thread_myid
h2_thread_myid:
	trap0(#H2_TRAP_THREAD_ID)
	jumpr r31

	.global h2_yield
h2_yield:
	trap0(#H2_TRAP_YIELD)
	jumpr r31

	.global h2_thread_set_tid
h2_thread_set_tid:
	trap0(#H2_TRAP_SET_TID)
	jumpr r31

	.global h2_thread_get_tid
h2_thread_get_tid:
	trap0(#H2_TRAP_GET_TID)
	jumpr r31
