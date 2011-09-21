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
	trap0(#H2_TRAP_GET_PCYCLES)
	jumpr r31

