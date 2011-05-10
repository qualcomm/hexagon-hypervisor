/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>

/* Cause an error by doing a misaligned load */

FUNC_START TH_do_error .text .p2align 2
	r0 = #0
	r6.h = #hi(0xdeadbeef)
	r6.l = #lo(0xdeadbeef)
	r6 = memw(r6)
FUNC_END TH_do_error

/* Guest vector table */

FUNC_START TH_guest_vectors .text .p2align 2
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_error_handler
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
	jump TH_bad_vector
FUNC_END TH_guest_vectors

/* Error handler that calls TH_error_check */

FUNC_START TH_error_handler .text .p2align 2
	r7.h = #hi(0xdeadbeef)
	r7.l = #lo(0xdeadbeef)
	p0 = cmp.eq(r7,r6)
	r1 = r29
	if (p0) r0 = #1
	jump TH_error_check
FUNC_END TH_do_error

#if __QDSP6_ARCH__ <= 3
#define SSR_BIT 13
#else
#define SSR_BIT 19
#endif

/* Go in and out of "Guest" vs "user" mode

FUNC_START TH_guestmode .text .p2align 2
	r0 = ssr
	r0 = setbit(r0,#SSR_BIT)
	ssr = r0
	jumpr r31
FUNC_END TH_guestmode

FUNC_START TH_usermode .text .p2align 2
	r0 = ssr
	r0 = clrbit(r0,#SSR_BIT)
	ssr = r0
	jumpr r31
FUNC_END TH_usermode

FUNC_START TH_clr_ex .text .p2align 2
	r0 = ssr
	r0 = clrbit(r0,#17)
	ssr = r0
	jumpr r31
FUNC_END TH_clr_ex

