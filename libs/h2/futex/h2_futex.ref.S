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

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

	.text

	/* h2_futex_wait
	 * Basically, we call trap and return.
	 */
FUNC_START h2_futex_wait .text.h2.futex .p2align 2
	trap0(#H2_TRAP_FUTEX_WAIT)
	jumpr r31
FUNC_END h2_futex_wait

	/* h2_futex_wake
	 * Basically, we call trap and return.
	 */
FUNC_START h2_futex_wake .text.h2.futex .p2align 2
	trap0(#H2_TRAP_FUTEX_WAKE)
	jumpr r31
FUNC_END h2_futex_wake

	/* h2_futex_wait
	 * Basically, we call trap and return.
	 */
FUNC_START h2_futex_lock_pi .text.h2.futex .p2align 2
	trap0(#H2_TRAP_FUTEX_LOCK_PI)
	jumpr r31
FUNC_END h2_futex_lock_pi

	/* h2_futex_wake
	 * Basically, we call trap and return.
	 */

FUNC_START h2_futex_unlock_pi .text.h2.futex .p2align 2
	trap0(#H2_TRAP_FUTEX_UNLOCK_PI)
	jumpr r31
FUNC_END h2_futex_unlock_pi

