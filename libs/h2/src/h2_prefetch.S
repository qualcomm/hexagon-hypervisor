/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_trap_constants.h>

	.text
	.global h2_set_prefetch
h2_set_prefetch:
	trap0(#H2_TRAP_SET_PREFETCH)
	jumpr r31

