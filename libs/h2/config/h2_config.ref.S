/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

	.text
	.global h2_config_trap
h2_config_trap:
	trap0(#H2_TRAP_CONFIG)
	jumpr r31
