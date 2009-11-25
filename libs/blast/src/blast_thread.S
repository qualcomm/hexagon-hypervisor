/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * blast_thread.S
 * 
 * This file contains the implementation for 
 * blast_thread functionality 
 */

#include "blast_trap_constants.h"

	.text
	.global blast_thread_create
blast_thread_create:
	allocframe(#0)
	trap0(#BLAST_TRAP_THREAD_CREATE)
1:
	deallocframe
	jumpr r31

	.global blast_thread_stop
blast_thread_stop:
	allocframe(#0)
	trap0(#BLAST_TRAP_THREAD_STOP)
	// shouldn't return, but...
	jump 1b

	.global blast_thread_myid
blast_thread_myid:
	allocframe(#0)
	trap0(#BLAST_TRAP_THREAD_ID)
	jump 1b

	.global blast_yield
blast_yield:
	allocframe(#0)
	trap0(#BLAST_TRAP_YIELD)
	jump 1b

	.global blast_thread_set_tid
blast_thread_set_tid:
	allocframe(#0)
	trap0(#BLAST_TRAP_SET_TID)
	jump 1b

	.global blast_thread_get_tid
blast_thread_get_tid:
	allocframe(#0)
	trap0(#BLAST_TRAP_GET_TID)
	jump 1b
