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
	.global h2_intwait

	/* h2_intwait
	 * Provide an interrupt.  Basically, we call trap and return.
	 */
FUNC_START h2_intwait .text.h2.intwait
	trap0(#H2_TRAP_INTWAIT)
	jumpr r31
FUNC_END h2_intwait

