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
	.global h2_futex_lock_pi
	.global h2_futex_unlock_pi

	/* h2_futex_wait
	 * Basically, we call trap and return.
	 */
h2_futex_wait:
	trap0(#H2_TRAP_FUTEX_WAIT)
	jumpr r31

	/* h2_futex_wake
	 * Basically, we call trap and return.
	 */
h2_futex_wake:
	trap0(#H2_TRAP_FUTEX_WAKE)
	jumpr r31

	/* h2_futex_wait
	 * Basically, we call trap and return.
	 */
h2_futex_lock_pi:
	trap0(#H2_TRAP_FUTEX_LOCK_PI)
	jumpr r31

	/* h2_futex_wake
	 * Basically, we call trap and return.
	 */
h2_futex_unlock_pi:
	trap0(#H2_TRAP_FUTEX_UNLOCK_PI)
	jumpr r31

