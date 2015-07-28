/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

FUNC_START h2_waitcycles .text.h2.waitcycles .p2align 2
	trap0(#H2_TRAP_WAITCYCLES)
	jumpr r31
FUNC_END h2_waitcycles

