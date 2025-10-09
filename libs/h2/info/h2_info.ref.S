/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

FUNC_START h2_info_trap .text.h2.info
	trap0(#H2_TRAP_INFO)
	jumpr r31
FUNC_END h2_info
