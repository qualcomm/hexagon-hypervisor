/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"

#include <h2_trap_constants.h>

FUNC_START h2_get_pcycles .text.h2.cycles .p2align 3
	trap0(#H2_TRAP_CPUTIME)
	jumpr r31
FUNC_END h2_get_pcycles

FUNC_START h2_get_core_pcycles .text.h2.cycles .p2align 3
	//trap0(#H2_TRAP_GET_PCYCLES)
	//r1:0 = c15:14
	.word 0x680ec000		// seriously, assembler guys...
	jumpr r31
FUNC_END h2_get_core_pcycles

