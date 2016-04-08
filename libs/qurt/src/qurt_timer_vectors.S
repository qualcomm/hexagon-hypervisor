/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_trap_constants.h>
#include <h2_common_error.h>
#include <h2_context_defs.h>

	.text

.global qurt_timer_vectors
qurt_timer_vectors:
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump interrupt

interrupt:
	{
		allocframe(#H2_CONTEXT_SIZE-8)
		memd(r29+#-16) = r29:28
		call h2_context_save
	}
	{
		r18 = r31
		call qurt_timer_interrupt
	}
	callr r18  // restore
	{
		r29:28 = memd(r29+#H2_CONTEXT_SIZE-16)
		r31:30 = memd(r29+#H2_CONTEXT_SIZE-8)
	}
	trap1(#H2_VMTRAP_RETURN)

