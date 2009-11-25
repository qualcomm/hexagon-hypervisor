/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * blast_intwait.S
 * 
 * This file contains the implementation for 
 * blast_intwait functionality 
 */

#include "blast_trap_constants.h"

	.text
	.global blast_futex_wait
	.global blast_futex_wake

	/* blast_futex_wait
	 * Basically, we call trap and return.
	 */
blast_futex_wait:
	allocframe(#0)
	trap0(#BLAST_TRAP_FUTEX_WAIT)
	deallocframe
	jumpr r31

	/* blast_futex_wake
	 * Basically, we call trap and return.
	 */
blast_futex_wake:
	allocframe(#0)
	trap0(#BLAST_TRAP_FUTEX_WAKE)
	deallocframe
	jumpr r31

