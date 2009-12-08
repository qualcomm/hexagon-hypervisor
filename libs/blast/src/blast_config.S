/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "blast_trap_constants.h"

	.text
	.global blast_config_add_thread_storage
blast_config_add_thread_storage:
	{
		r1 = r0	// ptr
		r2 = r1 // size
		r0 = #0	// which one
		allocframe(#0)
	}
	trap0(#BLAST_TRAP_CONFIG)
	deallocframe
	jumpr r31

