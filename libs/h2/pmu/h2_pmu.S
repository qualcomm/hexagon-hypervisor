/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

	.text
	.global h2_pmuconfig_trap
h2_pmuconfig_trap:
	trap0(#H2_TRAP_PMUCONFIG)
	jumpr r31

