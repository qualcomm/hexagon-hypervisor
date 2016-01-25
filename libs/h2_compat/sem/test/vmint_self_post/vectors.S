/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_trap_constants.h>
#include <h2_common_error.h>
#include <h2_context_defs.h>

	.text

sem_test_vectors:
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump sem_do_interrupt

	.global sem_test_vecsetup
sem_test_vecsetup:
	allocframe(#0)
	r0 = ##sem_test_vectors
	trap1(#H2_VMTRAP_SETVEC)
	dealloc_return

sem_do_interrupt:
	# Allocate stack, save context, print and maybe exit
	{
		allocframe(#H2_CONTEXT_SIZE-8)
		memd(r29+#-16) = r29:28
		call h2_context_save
	}
	{
		r18 = r31
		call timer_handler
	}
	callr r18
	{
		r29:28 = memd(r29+#H2_CONTEXT_SIZE-16)
		r31:30 = memd(r29+#H2_CONTEXT_SIZE-8)
	}
	trap1(#H2_VMTRAP_RETURN)

