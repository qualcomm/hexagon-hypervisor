/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

	.text
	.p2align 3
	.global h2_info
h2_info:
	trap0(#H2_TRAP_INFO)
	jumpr r31
