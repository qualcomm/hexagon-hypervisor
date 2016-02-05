/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_fastint.S
 * This file contains the implementation for registering a fast interrupt
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

FUNC_START h2_register_fastint .text.h2.fastint .p2align 2
	trap0(#H2_TRAP_REGISTER_FASTINT)
	jumpr r31
FUNC_END h2_register_fastint

FUNC_START h2_deregister_fastint .text.h2.fastint .p2align 2
	r1 = #0 // null means deregister
	trap0(#H2_TRAP_REGISTER_FASTINT)
	jumpr r31
FUNC_END h2_deregister_fastint

