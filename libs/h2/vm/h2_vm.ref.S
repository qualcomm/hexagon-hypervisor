/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_vm.S
 * 
 * Boot a VM
 */

#include "h2_trap_constants.h"

	.text
	.p2align 3
	.global h2_vmop_trap
h2_vmop_trap:
	trap0(#H2_TRAP_VMOP)
	jumpr r31
