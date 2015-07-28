/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

	.text
FUNC_START h2_get_prio .text.h2.prio .falign
	trap0(#H2_TRAP_PRIO_GET)
	jumpr r31
FUNC_END h2_get_prio

FUNC_START h2_set_prio .text.h2.prio .falign
	trap0(#H2_TRAP_PRIO_SET)
	jumpr r31
FUNC_END h2_set_prio

