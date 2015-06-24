/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_trap_constants.h>

	.text
	.global h2_get_pcycles
h2_get_pcycles:
	trap0(#H2_TRAP_CPUTIME)
	jumpr r31

	.global h2_get_core_pcycles
h2_get_core_pcycles:
	//r1:0 = c15:14
	.word 0x680ec000		// seriously, assembler guys...
	jumpr r31

