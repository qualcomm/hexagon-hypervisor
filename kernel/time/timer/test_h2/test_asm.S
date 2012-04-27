/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <h2_trap_constants.h>
	.text
	.global TH_vectors

TH_vectors:
TH_bad_vec:
	jump TH_bad_vec		/* 0 */
	jump TH_bad_vec		/* 1 */
	jump TH_bad_vec		/* 2 */
	jump TH_bad_vec		/* 3 */
	jump TH_bad_vec		/* 4 */
	jump TH_trap_event	/* 5 */
	jump TH_bad_vec		/* 6 */
	jump TH_interrupt	/* 7 */
	jump TH_bad_vec		/* 8 */
	jump TH_bad_vec		/* 9 */
	jump TH_bad_vec		/* a */
	jump TH_bad_vec		/* b */
	jump TH_bad_vec		/* c */
	jump TH_bad_vec		/* d */
	jump TH_bad_vec		/* e */
	jump TH_bad_vec		/* f */

TH_interrupt:
	g3 = r0
	r0 = ##saw_interrupt
	memw(r0) = #1
	r0 = g3
TH_trap_event:
	trap1(#H2_VMTRAP_RETURN)

	.global h2_time_get_time
h2_time_get_time:
	allocframe(#0)
	r0 = #3
	trap0(#22)
	deallocframe
	jumpr r31

	.global h2_time_set_timeout
h2_time_set_timeout:
	allocframe(#0)
	r3:2 = r1:0
	r0 = #5
	trap0(#22)
	deallocframe
	jumpr r31

	.global set_vectors
set_vectors:
	r0 = ##TH_vectors
	jump h2_vmtrap_setvec

