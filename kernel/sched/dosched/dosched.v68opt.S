/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * Note: This function does not return.  Always calls switch(), which doesn't
 * return
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

/* number of u64 dwords in ready_valids (one bit per priority) */
#define READY_VALID_DWORDS ((MAX_PRIOS)/64)

#if CLUSTER_SCHED

#define NEW r0			/* H2K_ready_getbest_asm() return value */
#define ME r6			/* me */
#define HTHREAD r7		/* hthread */
#define HTHREAD_ME r7:6

FUNC_START H2K_dosched  .text.core.dosched  .p2align  3
	{
		allocframe(#8)
		r0 = r1					// arg0 = hthread
		memd(r29+#-16) = r1:0
		call H2K_ready_getbest_asm
	}
	{
		p0 = cmp.eq(NEW,#0)
		HTHREAD_ME = memd(r29+#0)
		deallocframe
	}
	{
		if (!p0) memb(NEW+#CONTEXT_hthread) = HTHREAD		// new->hthread = (u8_t)hthread
		if (!p0) memb(NEW+#CONTEXT_status) = #STATUS_RUNNING	// new->status = H2K_STATUS_RUNNING
		r1:0 = combine(NEW,ME)				// r0 = me, r1 = new/null
		jump H2K_switch					// H2K_switch(me,new)
	}
FUNC_END H2K_dosched

#undef HTHREAD_ME
#undef NEW
#undef ME
#undef HTHREAD

#else

#define HTHREAD r1		/* arg1: hthread */
#define PRIO r4			/* best priority so far */
#define DWORD r5			/* u64 dword of ready_valids that holds PRIO (0..3) */
#define DWORD_PRIO r5:4
#define READY r9		/* &ready[0] */
#define EVEN_VALIDS r3:2	/* ready_valids[0], then [2] */
#define EVEN_CT0 r2		/* ct0(ready_valids[2]) */
#define ODD_VALIDS r15:14	/* ready_valids[1], then [3] */
#define ODD_CT0 r14		/* ct0(ready_valids[1] / [3]) */

FUNC_START H2K_dosched .text.core.dosched .p2align 2

/* --- Phase 1: H2K_ready_best_prio() --------------------------------------- */
	/* Dwords 0/1, then 2/3, are loaded in pairs.  ct0 = 64 means the dword is empty. */
	{
		DWORD_PRIO = combine(#0,#0x3f)			// DWORD = 0; PRIO = 63 (compare value)
		READY = add(H2K_GP,#KG_ready)
		EVEN_VALIDS = memd(H2K_GP+#KG_ready_valids+0x0)
		ODD_VALIDS = memd(H2K_GP+#KG_ready_valids+0x8)
	}
	{
		PRIO = ct0(EVEN_VALIDS)
		ODD_CT0 = ct0(ODD_VALIDS)
		if (!cmp.gtu(PRIO.new,PRIO)) jump:t .Lfound_prio	// dword 0 non-empty
	}
	{
		p0 = cmp.gtu(ODD_CT0,#0x3f)
		PRIO = add(ODD_CT0,PRIO)
		if (!p0.new) jump:t .Lfound_prio			// dword 1 non-empty
		DWORD = add(DWORD,#1)
	}
	{
		EVEN_VALIDS = memd(H2K_GP+#KG_ready_valids+0x10)
		ODD_VALIDS = memd(H2K_GP+#KG_ready_valids+0x18)
	}
	{
		EVEN_CT0 = ct0(EVEN_VALIDS)
		ODD_CT0 = ct0(ODD_VALIDS)
		DWORD = add(DWORD,#1)
	}
	{
		p0 = cmp.gtu(EVEN_CT0,#0x3f)
		if (!p0.new) jump:t .Lfound_prio			// dword 2 non-empty
		PRIO = add(PRIO,EVEN_CT0)
		p1 = cmp.gtu(ODD_CT0,#0x3f)				// p1: dword 3 empty as well
	}
	{
		PRIO = add(ODD_CT0,PRIO)
		if (!p1) DWORD = add(DWORD,#1)
		if (p1) jump:nt H2K_switch				// nothing ready: H2K_switch(me,NULL)
		if (p1) r1 = #0x0
	}

#undef DWORD_PRIO
#undef EVEN_VALIDS
#undef EVEN_CT0
#undef ODD_VALIDS
#undef ODD_CT0

/* --- Phase 2: new = ready[prio], inlined H2K_ring_remove_real ------------ */

#define NEW r7			/* new = ready[PRIO], always the ring head */
#define RING r9			/* &ready[PRIO] (was READY) */
#define NEXT r2			/* new->next */
#define PREV r3			/* new->prev */
#define PREV_NEXT r3:2
#define WIDX r6			/* PRIO >> 5: u32 word of ready_valids */
#define BITNO r8		/* PRIO & 31: bit in that u32 word */
#define COUNT r15		/* rescan loop count: dwords DWORD..READY_VALID_DWORDS-1 */
#define BASE r14		/* rescan: priority of bit 0 of the scanned dword */

.Lfound_prio:
	{
		COUNT = sub(#READY_VALID_DWORDS,DWORD)
		BASE = asl(DWORD,#0x6)
		NEW = memw(READY+PRIO<<#0x2)			// new = ready[prio]
		RING = addasl(READY,PRIO,#0x2)
	}
#undef READY
	/*
	 * new is the head, so the head is always rewritten:
	 *   ring had >1 thread: ready[prio] = new->next    ring stays NON-EMPTY
	 *   ring had 1 thread:  ready[prio] = NULL          ring becomes EMPTY
	 */
	{
		PREV_NEXT = memd(NEW+#CONTEXT_next)
		WIDX = lsr(PRIO,#0x5)
		BITNO = and(PRIO,#0x1f)
		memb(NEW+#CONTEXT_status) = #STATUS_RUNNING	// new->status = RUNNING
	}
	{
		memw(PREV+#CONTEXT_next) = NEXT			// prev->next = next
		memw(NEXT+#CONTEXT_prev) = PREV			// next->prev = prev
		p1 = cmp.eq(NEW,NEXT)				// p1: new was alone -> ring EMPTY
		loop0(.Lrescan,COUNT)				// harmless if the NON-EMPTY path skips it
	}
#undef PREV_NEXT
#undef PREV
#undef COUNT

#define NEWHEAD r2		/* new ready[PRIO]: next, or NULL if EMPTY */
#define VALID32 r3		/* u32 ready_valids word that holds PRIO's bit */

	/*
	 * NON-EMPTY (!p1): PRIO's bit stays set, so PRIO is still the best
	 *   priority: bestwait = PRIO, skip the clear and the rescan.
	 * EMPTY (p1): ready[prio] = NULL; fall through to clear PRIO's bit.
	 */
	{
		if (!p1) jump:nt .Lset_bestwait
		NEWHEAD = mux(p1,#0x0,NEXT)
		memw(RING+#0x0) = NEWHEAD.new			// ready[prio] = new head
		if (p1) VALID32 = memw(H2K_GP+WIDX<<#0x2)
	}
#undef NEXT
#undef NEWHEAD
#undef RING

/* --- Phase 3 (ring EMPTY only): clear PRIO's bit, rescan for bestwait ----- */

#define VALIDS_PTR r12		/* &ready_valids[DWORD], post-incremented */

	/*
	 * H2K_ready_clear_prio(prio).  Bit prio&63 of u64 dword prio>>6 is bit
	 * prio&31 of u32 word prio>>5 (little-endian), so one memw is enough.
	 */
	{
		VALID32 = clrbit(VALID32,BITNO)
		memw(H2K_GP+WIDX<<#0x2) = VALID32.new
		VALIDS_PTR = addasl(H2K_GP,DWORD,#0x3)
	}
#undef VALID32
#undef WIDX
#undef BITNO
#undef DWORD

#define CT0_MAX r15		/* constant 63; ct0 above this means an empty dword */
#define VALIDS r11:10		/* ready_valids dword being rescanned */
#define CT0 r13			/* ct0(VALIDS) */

	/*
	 * Rescan H2K_ready_best_prio() from dword DWORD: earlier dwords were empty,
	 * and only dword DWORD changed.  The first load is in its own packet so
	 * that it sees the clrbit store.
	 */
	{
		BASE = add(BASE,#-0x40)				// pre-bias: the loop adds 64 first
		CT0_MAX = #0x3f
		VALIDS = memd(VALIDS_PTR++#0x8)
	}
.Lrescan:
	{
		CT0 = ct0(VALIDS)
		BASE = add(BASE,#0x40)
		if (!cmp.gtu(CT0.new,CT0_MAX)) jump:nt .Lfound_next	// found a set bit
	}
	/* preload the next dword; on the last pass this reads past ready_valids, unused */
	{
		VALIDS = memd(VALIDS_PTR++#0x8)
	} :endloop0
	/* loop ran out: CT0 = 64, so PRIO = 256 = nothing ready */
.Lfound_next:
	{
		PRIO = add(BASE,CT0)
	}
#undef VALIDS_PTR
#undef CT0_MAX
#undef VALIDS
#undef CT0
#undef BASE

/* --- Done: bestwait, fill in new, switch ---------------------------------- */

.Lset_bestwait:
	{
		bestwait = PRIO
		memb(NEW+#CONTEXT_hthread) = HTHREAD		// new->hthread = hthread (r1 before r1 = new)
		r1 = NEW					// H2K_switch(me, new)
		jump H2K_switch
	}
FUNC_END H2K_dosched

#undef HTHREAD
#undef PRIO
#undef NEW

#endif
