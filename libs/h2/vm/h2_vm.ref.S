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
	.global h2_vmboot
h2_vmboot:
	trap0(#H2_TRAP_VMBOOT)
	jumpr r31

	.global h2_vmstatus
h2_vmstatus:
	trap0(#H2_TRAP_VMSTATUS)
	jumpr r31

