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
	.global h2_thread_create
h2_thread_create:
	allocframe(#0)
	trap0(#H2_TRAP_THREAD_CREATE)
1:
	deallocframe
	jumpr r31

	.global h2_thread_stop
h2_thread_stop:
	allocframe(#0)
	trap0(#H2_TRAP_THREAD_STOP)
	// shouldn't return, but...
	jump 1b

	.global h2_thread_myid
h2_thread_myid:
	allocframe(#0)
	trap0(#H2_TRAP_THREAD_ID)
	jump 1b

	.global h2_yield
h2_yield:
	allocframe(#0)
	trap0(#H2_TRAP_YIELD)
	jump 1b

	.global h2_thread_set_tid
h2_thread_set_tid:
	allocframe(#0)
	trap0(#H2_TRAP_SET_TID)
	jump 1b

	.global h2_thread_get_tid
h2_thread_get_tid:
	allocframe(#0)
	trap0(#H2_TRAP_GET_TID)
	jump 1b
