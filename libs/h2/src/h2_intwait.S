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
	.global h2_intwait

	/* h2_intwait
	 * Provide a mask.  Basically, we call trap and return.
	 */
h2_intwait:
	allocframe(#0)
	trap0(#H2_TRAP_INTWAIT)
	deallocframe
	jumpr r31

