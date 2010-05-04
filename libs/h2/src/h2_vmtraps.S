/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

	.text
	.global h2_vmtrap_newptb
	.global h2_vmtrap_clrmap
	.global h2_vmtrap_setvec

h2_vmtrap_setvec:
	trap1(#H2_VMTRAP_SETVEC)
	jumpr r31

h2_vmtrap_newptb:
	trap1(#H2_VMTRAP_NEWPTB)
	jumpr r31

h2_vmtrap_clrmap:
	trap1(#H2_VMTRAP_CLRMAP)
	jumpr r31

