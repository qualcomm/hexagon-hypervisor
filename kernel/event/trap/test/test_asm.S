/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <vmdefs.h>
#include <max.h>

/* Set up the environment and jump to the trap0 handler.  ELR will point to code that returns. */
	.text
	.global call_trap0
call_trap0:
	r10 = ssr
	r10 = insert(r0,#8,#0)
	ssr = r10
#if ARCHV <= 3
	sgp = r1
#else
	sgp0 = r1
#endif
	r11.h = #hi(1f)
	r11.l = #lo(1f)
	elr = r11
	/* other stuff here */
	jump H2K_handle_trap0
1:
	jumpr r31

/* Go in and out of guest/user mode */
	.global guest_mode
guest_mode:
	r10 = ssr
	r10 = setbit(r10,#SSR_GUEST_BIT)
	ssr = r10
	jumpr r31

	.global user_mode
user_mode:
	r10 = ssr
	r10 = clrbit(r10,#SSR_GUEST_BIT)
	ssr = r10
	jumpr r31

/* Guest vector table */
	.global TH_vectors
TH_vectors:
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_guest_trap
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	jump TH_bad_vec
	

/* Stack area  */
	.data
	.global H2K_stacks
	.p2align 5
stacks_start:
	.space 128
H2K_stacks:
	.space 128
	.space 128
	.space 128
	.space 128
	.space 128

// Definitions for certain kernel implementations
	.global H2K_futex_wait
H2K_futex_wait:
	r0 = #2
	jumpr r31

	.global H2K_futex_resume
H2K_futex_resume:
	r0 = #3
	jumpr r31

	.global H2K_futex_lock_pi
H2K_futex_lock_pi:
	r0 = #20
	jumpr r31

	.global H2K_futex_unlock_pi
H2K_futex_unlock_pi:
	r0 = #21
	jumpr r31

