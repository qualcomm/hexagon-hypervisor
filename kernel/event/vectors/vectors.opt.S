/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>

FUNC_START BLASTK_event_vectors .text.event.vectors .p2align 12
	jump BLASTK_handle_reset
	jump BLASTK_handle_nmi
	jump BLASTK_handle_error
	jump BLASTK_handle_rsvd
	jump BLASTK_handle_tlbmissx
	jump BLASTK_handle_rsvd
	jump BLASTK_handle_tlbmissrw
	jump BLASTK_handle_rsvd
	jump BLASTK_handle_trap0
	jump BLASTK_handle_trap1
	jump BLASTK_handle_rsvd /* 10 */
        jump BLASTK_handle_rsvd /* 11 */
        jump BLASTK_handle_rsvd /* 12 */
        jump BLASTK_handle_rsvd /* 13 */
        jump BLASTK_handle_rsvd /* 14 */
        jump BLASTK_handle_rsvd /* 15 */
        jump BLASTK_handle_int  /* 0 */
        jump BLASTK_handle_int  /* 1 */
        jump BLASTK_handle_int  /* 2 */
        jump BLASTK_handle_int /* 3  -- for resched */
        jump BLASTK_handle_int  // 4
        jump BLASTK_handle_int // 5
        jump BLASTK_handle_int // 6 
        jump BLASTK_handle_int // 7
        jump BLASTK_handle_int // 8
        jump BLASTK_handle_int // 9
        jump BLASTK_handle_int /* 10? */
        jump BLASTK_handle_int // 11
        jump BLASTK_handle_int // 12
        jump BLASTK_handle_int // 13
        jump BLASTK_handle_int // 14
        jump BLASTK_handle_int // 15
        jump BLASTK_handle_int // 16
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int
        jump BLASTK_handle_int /* 31 */
FUNC_END BLASTK_event_vectors

