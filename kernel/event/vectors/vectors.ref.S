/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>

#if ARCHV < 5 
#define VEC_P2ALIGN 12
#else
#define VEC_P2ALIGN 8
#endif

FUNC_START H2K_event_vectors .text.core.vectors .p2align VEC_P2ALIGN
	jump H2K_handle_reset
	jump H2K_handle_nmi
	jump H2K_handle_error
	jump H2K_handle_rsvd
	jump H2K_handle_tlbmissx
	jump H2K_handle_rsvd
	jump H2K_handle_tlbmissrw
#if ARCHV < 73
	jump H2K_handle_rsvd
#else
	jump H2K_handle_error // DMA TLB, pass to guest
#endif
	jump H2K_handle_trap0
	jump H2K_handle_trap1
	jump H2K_handle_rsvd /* 10 */
	jump H2K_handle_error /* 11 */ /* FP TRAP */
	jump H2K_handle_debug /* 12 */
	jump H2K_handle_rsvd /* 13 */
	jump H2K_handle_rsvd /* 14 */
	jump H2K_handle_rsvd /* 15 */
	jump H2K_handle_int  /* 0 */
	jump H2K_handle_int  /* 1 */
	jump H2K_handle_int  /* 2 */
	jump H2K_handle_int /* 3  -- for resched */
	jump H2K_handle_int  // 4
	jump H2K_handle_int // 5
	jump H2K_handle_int // 6 
	jump H2K_handle_int // 7
	jump H2K_handle_int // 8
	jump H2K_handle_int // 9
	jump H2K_handle_int /* 10? */
	jump H2K_handle_int // 11
	jump H2K_handle_int // 12
	jump H2K_handle_int // 13
	jump H2K_handle_int // 14
	jump H2K_handle_int // 15
	jump H2K_handle_int // 16
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int
	jump H2K_handle_int /* 31 */
FUNC_END H2K_event_vectors
//FUNC_START H2K_event_vectors_end .text.core.vectors
//FUNC_END H2K_event_vectors_end

