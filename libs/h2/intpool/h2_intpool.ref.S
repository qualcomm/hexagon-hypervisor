/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

FUNC_START h2_intpool_config .text.h2.config .falign
	trap0(#H2_INTPOOL_CONFIG)
	jumpr r31
FUNC_END h2_config_trap

FUNC_START h2_intpool_wait .text.h2.intpool .falign
	trap0(#H2_INTPOOL_WAIT)
	jumpr r31
FUNC_END h2_config_trap

