/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

	.text
	.global h2_fatal_crash
h2_fatal_crash:
	trap0(#H2_TRAP_SOFT_NMI)
	jumpr r31

