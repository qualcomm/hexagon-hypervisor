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
	jump TH_debug_event	/* 3 */
	jump TH_bad_vec		/* 4 */
	jump TH_bad_vec		/* 5 */
	jump TH_bad_vec		/* 6 */
	jump TH_bad_vec		/* 7 */
	jump TH_bad_vec		/* 8 */
	jump TH_bad_vec		/* 9 */
	jump TH_bad_vec		/* a */
	jump TH_bad_vec		/* b */
	jump TH_bad_vec		/* c */
	jump TH_bad_vec		/* d */
	jump TH_bad_vec		/* e */
	jump TH_bad_vec		/* f */

TH_debug_event:
	r0 = #1
	r1 = r29
	call TH_debug_check
	r0 = ##debug_event_str
	call FAIL

	.global TH_do_debug
TH_do_debug:
	r0 = ##1f
	r1 = g1
	r1 = setbit(r1,#29)
	g1 = r1
	g0 = r0
	trap1(#1)
1:
	nop
	nop
	nop
	r0 = ##do_debug_str
	jump FAIL

	.global TH_usermode
TH_usermode:
	r0 = ##1f
	r1 = ssr
	r1 = setbit(r1,#16)
	r1 = setbit(r1,#17)
	ssr = r1
	elr = r0
	rte
1:
	nop
	jumpr r31

	.global set_vectors
set_vectors:
	r0 = ##TH_vectors
	jump h2_vmtrap_setvec

	.data
do_debug_str:
	.ascii "do_debug"
	.byte 0
debug_event_str:
	.ascii "debug_event"
	.byte 0

