/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_fastint.S
 * This file contains the implementation for registering a fast interrupt
 */

#include "h2_trap_constants.h"

	.global h2_register_fastint
h2_register_fastint:
	trap0(#H2_TRAP_REGISTER_FASTINT)
	jumpr r31

	.global h2_deregister_fastint
h2_deregister_fastint:
	r1 = #0 // null means deregister
	trap0(#H2_TRAP_REGISTER_FASTINT)
	jumpr r31

