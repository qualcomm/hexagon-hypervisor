/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */


#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

#define NEXT 0
#define PREV 4

#define INTERRUPTED_ME r1	/* arg1: interrupted thread */
#define INPUT_HTHREAD r2	/* arg2: hardware thread */
#define HTHREAD_ME r1:0	/* H2K_dosched() arguments */
#define INTMASK r3
#define READY r14		/* &H2K_gp->ready[0] */
#define READY_VALID r15		/* selected u32 word of ready_valids */
#define BIT r6			/* bit for PRIO in READY_VALID */

FUNC_START H2K_resched .text.core.resched .p2align 3
	{
		INTMASK = #RESCHED_INT_INTMASK
		HTHREAD_ME = combine(INPUT_HTHREAD,INTERRUPTED_ME)
		READY = add(H2K_GP,#KG_ready)
		READY_VALID = add(H2K_GP,#KG_ready_valids)
	}
#undef INTERRUPTED_ME
#undef INPUT_HTHREAD
	{
		BIT = #1
		ciad(INTMASK)
	}
#undef INTMASK

#define ME r0
#define HTHREAD r1
#define HEAD r2			/* ready-list head */
#define TAIL r3			/* ready-list tail */
#define TAIL_HEAD r3:2
#define NODE_LINKS r5:4	/* node->prev,node->next */
#define PRIO r10		/* me->prio */
#define WIDX r9			/* PRIO >> 5 */
#define BITIDX r8		/* PRIO & 31 */
#define WIDX_BITIDX r9:8
#define RING r13		/* &ready[PRIO] */

	ASM_BKL_LOCK(r5, r7, r8, p0)
.Lresched_locked:
	{
		NODE_LINKS = combine(ME,ME)	// for case of empty
		p0 = cmp.eq(ME,#0)
#if CLUSTER_SCHED
		if (p0.new) jump:t .Lwait
#else
		if (p0.new) jump:t H2K_dosched
#endif
		if (!p0.new) PRIO = memub(ME+#CONTEXT_prio)
		if (!p0.new) memb(ME+#CONTEXT_status) = #STATUS_READY
	}
	{
		WIDX_BITIDX = bitsplit(PRIO,#5)
		RING = addasl(READY,PRIO,#0x2)
	}
	{
		memd(ME) = NODE_LINKS		// store in case empty -- dummy if not null
		HEAD = memw(RING)
		BIT = asl(BIT,BITIDX)
		READY_VALID += asl(WIDX,#0x2)
	}
	{
		memw(READY_VALID) |= BIT
		p0 = cmp.eq(HEAD,#0)
		if (!p0.new) TAIL = memw(HEAD+#PREV)	// get *ring->prev
	}
	{
		if (p0) jump H2K_dosched
		if (!p0) memw(HEAD+#PREV) = ME	// *ring->prev = node
		if (p0) memw(RING) = ME		// if ring was empty, now points to appended value
	}
	{
		memd(ME) = TAIL_HEAD		// node->prev,node->next
		memw(TAIL+#NEXT) = ME		// *ring->prev->next = node
		jump H2K_dosched
	}
#if CLUSTER_SCHED
#define WAIT_MASK r2
.Lwait:
	{
		WAIT_MASK = memw(H2K_GP+#KG_wait_mask)
	}
	{
		WAIT_MASK = clrbit(WAIT_MASK,HTHREAD)
		memw(H2K_GP+#KG_wait_mask) = WAIT_MASK.new
		jump H2K_dosched
	}
#undef WAIT_MASK
#endif
FUNC_END H2K_resched

#undef NEXT
#undef PREV
#undef HTHREAD_ME
#undef READY
#undef READY_VALID
#undef BIT
#undef ME
#undef HTHREAD
#undef HEAD
#undef TAIL
#undef TAIL_HEAD
#undef NODE_LINKS
#undef PRIO
#undef WIDX
#undef BITIDX
#undef WIDX_BITIDX
#undef RING

#if CLUSTER_SCHED
#define CLUSTER_INTERRUPTED_ME r1	/* arg1: interrupted thread */
#define CLUSTER_INPUT_HTHREAD r2	/* arg2: hardware thread */
#define CLUSTER_HTHREAD_ME r1:0	/* H2K_dosched() arguments */
#define CLUSTER_INTMASK r3
#define CLUSTER_IASSIGN r4	/* packed interrupt number and hardware-thread mask */
#define CLUSTER_READY r14		/* &H2K_gp->ready[0] */
#define CLUSTER_READY_VALID r15	/* selected u32 word of ready_valids */
#define CLUSTER_BIT r6			/* bit for PRIO in READY_VALID */

FUNC_START H2K_resched_cluster .text.core.resched .p2align 3
	{
		CLUSTER_INTMASK = #CLUSTER_RESCHED_INT_INTMASK
		CLUSTER_HTHREAD_ME = combine(CLUSTER_INPUT_HTHREAD,CLUSTER_INTERRUPTED_ME)
		CLUSTER_READY = add(H2K_GP,#KG_ready)
		CLUSTER_READY_VALID = add(H2K_GP,#KG_ready_valids)
	}
	{
		CLUSTER_BIT = #1
		ciad(CLUSTER_INTMASK)
	}
	ASM_BKL_LOCK(r5, r7, r8, p0)
	{
		CLUSTER_IASSIGN = ##((CLUSTER_RESCHED_INT << 16) | 0xffff)
	}
	{
		iassignw(CLUSTER_IASSIGN)
	}
	jump .Lresched_locked
FUNC_END H2K_resched_cluster

#undef CLUSTER_INTERRUPTED_ME
#undef CLUSTER_INPUT_HTHREAD
#undef CLUSTER_HTHREAD_ME
#undef CLUSTER_INTMASK
#undef CLUSTER_IASSIGN
#undef CLUSTER_READY
#undef CLUSTER_READY_VALID
#undef CLUSTER_BIT
#endif