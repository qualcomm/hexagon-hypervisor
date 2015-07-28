/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"

#include <h2_trap_constants.h>
#include <h2_common_error.h>
#include <h2_context_defs.h>

	.text
	// h2_handle_errors(int exit_flag)
	.global h2_handle_errors
	.global h2_error_event
FUNC_START h2_handle_errors .text.h2.error .falign
	allocframe(#0)
	r1 = ##do_exit
	memw(r1) = r0
	call h2_debug_mutex_init
	r0 = ##h2_error_vectors
	trap1(#H2_VMTRAP_SETVEC)
	dealloc_return
FUNC_END h2_handle_errors

FUNC_START h2_error_vectors .text.h2.error .falign
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_error_event
	jump h2_interrupt_event
FUNC_END h2_error_vectors

FUNC_START h2_error_event .text.h2.error .falign
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
FUNC_END h2_error_event

FUNC_START h2_interrupt_event .text.h2.error .falign
	# Just ignore all interrupts
	trap1(#H2_VMTRAP_RETURN)
FUNC_END h2_interrupt_event
	

DATA_START do_exit .data.h2.error .p2align 2
	.word 0x0
DATA_END do_exit

