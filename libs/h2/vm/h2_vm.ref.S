/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_vm.S
 * 
 * Boot a VM
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

FUNC_START h2_vmop_trap .text.h2.vmop
	trap0(#H2_TRAP_VMOP)
	jumpr r31
FUNC_END h2_vmop_trap
