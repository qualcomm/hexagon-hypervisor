/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_trap_constants.h>
#include <h2_common_error.h>
#include <h2_context_defs.h>

	.text
	// h2_handle_errors(int exit_flag)
	.global h2_handle_errors
	.global h2_error_event
h2_handle_errors:
	allocframe(#0)
	r1 = ##do_exit
	memw(r1) = r0
	call h2_debug_mutex_init
	r0 = ##vectors
	trap1(#H2_VMTRAP_SETVEC)
	dealloc_return

vectors:
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_interrupt_event
h2_error_event:
	# Allocate stack, save context, print and maybe exit
	{
		allocframe(#H2_CONTEXT_SIZE-8)
		memd(r29+#-16) = r29:28
		call h2_context_save
	}
	{
		r0 = r29
		call h2_debug_context_handle
	}
	{
		r0 = memw(##do_exit)
	}
	{
		p0 = cmp.eq(r0,#0)
		if (!p0.new) jump:nt .Ltrap_exit
	}
	{
		r0 = ##H2_THREAD_FATAL_ERROR
		call h2_thread_stop
	}
.Ltrap_exit:
	{
		r0 = #1
		call exit
	}
	// normally shouldn't reach here
	{
		r0 = ##H2_THREAD_FATAL_ERROR
		call h2_thread_stop
	}

h2_interrupt_event:	// interrupt
	# Just ignore all interrupts
	trap1(#H2_VMTRAP_RETURN)
	

	.data
	.p2align 2

do_exit:
	.word 0x0

