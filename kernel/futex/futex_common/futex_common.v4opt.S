/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <futex.h>
#include <max.h>

/* H2K_futex_hash_add_ring(H2K_thread_context **ring, H2K_thread_context *me) */
FUNC_START H2K_futex_hash_add_ring  .text.core.futex  .p2align  5
#define RINGPTR r0
#define ME r1
#define CURHEAD r2
#define MYPRIO r3
#define PREV_NEXT r5:4
#define PREV r5
#define NEXT r4
#define TESTPRIO r6
	{
		CURHEAD = memw(RINGPTR)
		PREV_NEXT = combine(ME,ME)
		if (cmp.eq(CURHEAD.new,#0)) jump:nt .Ladd_empty
	}
	{
		MYPRIO = memub(ME+#CONTEXT_prio)
		TESTPRIO = memub(CURHEAD+#CONTEXT_prio)
	}
	{
		PREV = memw(CURHEAD+#CONTEXT_prev)
		p0 = cmp.gt(TESTPRIO,MYPRIO)
		if (p0.new) memw(RINGPTR) = ME
		if (p0.new) jump:nt .Ladd_found
		NEXT = CURHEAD
	}
	.falign
.Lnext:
	{
		TESTPRIO = memub(PREV+#CONTEXT_prio)
	}
	{
		p0 = cmp.gt(TESTPRIO,MYPRIO)
		if (p0.new) PREV = memw(PREV+#CONTEXT_prev)
		if (p0.new) jump:t .Lnext
		if (p0.new) NEXT = PREV
	}
	.falign
.Ladd_found:
	{
		memw(NEXT+#CONTEXT_prev) = ME
		memw(PREV+#CONTEXT_next) = ME
	}
	{
		memd(ME+#CONTEXT_next) = PREV_NEXT
		jumpr r31
	}
	.falign
.Ladd_empty:
	{
		memd(ME+#CONTEXT_next) = PREV_NEXT
		memw(RINGPTR) = ME
		jumpr r31
	}
#undef TESTPRIO
#undef NEXT
#undef PREV
#undef PREV_NEXT
#undef MYPRIO
#undef CURHEAD
#undef ME
#undef RINGPTR
FUNC_END H2K_futex_hash_add_ring

// H2K_futex_hash_remove_one(pa_t lock, H2K_thread_context **ring, H2K_thread_context **pos)
FUNC_START H2K_futex_hash_remove_one  .text.core.futex  .p2align  5
#define LOCK r0
#define RINGPTR r1
#define POSPTR r2
#define CUR r3
#define START r4
#define CURLOCK r5
#define PREV_NEXT r7:6
#define PREV r7
#define NEXT r6
#define READY r8
#define NEWPRIO r9
#define READYHEAD r10
#define READYVALIDS r11
#define WIDX_BITNO r13:12
#define WIDX r13
#define BITNO r12
	{
		CUR = memw(POSPTR)
		START = memw(RINGPTR)
		READY = ADD(H2K_GP,#KG_ready)
		READYVALIDS = add(H2K_GP,#KG_ready_valids)
	}
	{
		p0 = cmp.eq(START,#0)
		if (p0.new) jump:nt .Lrm_bad
		if (!p0.new) CURLOCK = memw(CUR+#CONTEXT_futex_ptr)
		if (!p0.new) NEXT = memw(CUR+#CONTEXT_next)
	}
	.falign
.Lrm_again:
	{
		CURLOCK = memw(CUR+#CONTEXT_futex_ptr)
		NEXT = memw(CUR+#CONTEXT_next)
	}
	{
		p0 = !cmp.eq(NEXT,START)			// if we're not back to start
		p0 = !cmp.eq(CURLOCK,LOCK)			// and we're not at the lock
		if (p0.new) jump:t .Lrm_again			// repeat
		if (p0.new) CUR = NEXT
	}
	{
		p1 = cmp.eq(CURLOCK,LOCK)			// Did we find the lock?
		if (!p1.new) jump:nt .Lrm_bad			// No, return fail
		memw(POSPTR) = NEXT				// save position
		PREV = memw(CUR+#CONTEXT_prev)			// Get prev
		p0 = cmp.eq(CUR,START)				// removing head?
	}
#undef POSPTR
#define TMP r2
	{
		memw(PREV+#CONTEXT_next) = NEXT
		memw(NEXT+#CONTEXT_prev) = PREV
		p1 = cmp.eq(NEXT,CUR)				// only thing in list?
		if (p1.new) NEXT = #0
	}
	{
		if (p0) memw(RINGPTR) = NEXT
		NEWPRIO = memub(CUR+#CONTEXT_prio)
		PREV_NEXT = combine(CUR,CUR)	// in case empty
	}
	{
		memb(CUR+#CONTEXT_status) = #STATUS_READY
		READYHEAD = memw(READY+NEWPRIO<<#2)
		WIDX_BITNO = bitsplit(NEWPRIO,#5)
	}
	{
		p0 = cmp.eq(READYHEAD,#0)
		if (!p0.new) PREV = memw(READYHEAD+#CONTEXT_prev)
		if (!p0.new) NEXT = READYHEAD
		TMP = memw(READYVALIDS+WIDX<<#2)
	}
	{
		if (p0) memw(READY+NEWPRIO<<#2) = CUR
		memd(CUR+#CONTEXT_next) = PREV_NEXT
		TMP = setbit(TMP,BITNO)
	}
	{
		if (!p0) memw(NEXT+#CONTEXT_prev) = CUR
		if (!p0) memw(PREV+#CONTEXT_next) = CUR
	}
	{
		memw(READYVALIDS+WIDX<<#2) = TMP
		r0 = CUR
		jumpr r31
	}
.Lrm_bad:
	{
		r0 = #0
		jumpr r31
	}
FUNC_END H2K_futex_hash_remove_one

