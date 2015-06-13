/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_trap_constants.h>
#include <h2_common_error.h>
#include <h2_context_defs.h>

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
	jump h2_error_event

.global bootvm__Interrupt
bootvm__Interrupt:
	{
		allocframe(#H2_CONTEXT_SIZE-8)
		memd(r29+#-16) = r29:28
		call h2_context_save
	}
	{
		r18 = r31
		call booter_isr
	}
	callr r18  // restore
	{
		r29:28 = memd(r29+#H2_CONTEXT_SIZE-16)
		r31:30 = memd(r29+#H2_CONTEXT_SIZE-8)
	}
	trap1(#H2_VMTRAP_RETURN)
