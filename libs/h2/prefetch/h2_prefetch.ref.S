/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"

#include <h2_trap_constants.h>

FUNC_START h2_set_prefetch .text.h2.prefetch .p2align 2
#if ARCHV <= 3
	jumpr r31
#else
	r1 = usr
	r1 = insert(r0,#7,#15)
	usr = r1
	jumpr r31
#endif
FUNC_END h2_set_prefetch

