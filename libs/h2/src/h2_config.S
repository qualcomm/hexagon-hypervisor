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
		r0 = #0	// H2K_trap_config_addthreads
	}
	trap0(#H2_TRAP_CONFIG)
	jumpr r31

	.global h2_config_vmblock_size
h2_config_vmblock_size:
	{
		r1 = #0 // unused
	  r2 = r0 // num_cpus
		r3 = r1 // num_ints
		r0 = #2 // H2K_trap_config_vmblock_size
	}
	trap0(#H2_TRAP_CONFIG)
	jumpr r31

	.global h2_config_vmblock_init
h2_config_vmblock_init:
	{
		r4 = r3 // arg2
		r3 = r2 // arg1
		r2 = r1 // op
	}
	{
    r1 = r0 // ptr
		r0 = #3 // H2K_trap_config_vmblock_init
  }
	trap0(#H2_TRAP_CONFIG)
	jumpr r31
