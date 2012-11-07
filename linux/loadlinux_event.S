/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_trap_constants.h>

	.text

	.global bootvm_vectors
	.p2align 12
bootvm_vectors:
	jump bootvm__Reset
	jump bootvm__MachineCheck
	jump bootvm__Error
	jump bootvm__Rsvd
	jump bootvm__Rsvd
	jump bootvm__Trap0
	jump bootvm__Rsvd
	jump bootvm__Interrupt

bootvm__Reset:
bootvm__MachineCheck:
bootvm__Rsvd:
bootvm__Rsvd:
bootvm__Rsvd:
bootvm__Error:
bootvm__Trap0:
1:
	jump 1b

bootvm__Interrupt:
	call handle_child_int
	trap1(#H2_VMTRAP_RETURN)
