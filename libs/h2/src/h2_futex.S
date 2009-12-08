/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_intwait.S
 * 
 * This file contains the implementation for 
 * h2_intwait functionality 
 */

#include "h2_trap_constants.h"

	.text
	.global h2_futex_wait
	.global h2_futex_wake

	/* h2_futex_wait
	 * Basically, we call trap and return.
	 */
h2_futex_wait:
	allocframe(#0)
	trap0(#H2_TRAP_FUTEX_WAIT)
	deallocframe
	jumpr r31

	/* h2_futex_wake
	 * Basically, we call trap and return.
	 */
h2_futex_wake:
	allocframe(#0)
	trap0(#H2_TRAP_FUTEX_WAKE)
	deallocframe
	jumpr r31

