/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

	.text
	.global h2_config_add_thread_storage
h2_config_add_thread_storage:
	{
		r1 = r0	// ptr
		r2 = r1 // size
		r0 = #0	// which one
	}
	trap0(#H2_TRAP_CONFIG)
	jumpr r31

