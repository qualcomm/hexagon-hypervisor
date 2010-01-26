/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* rings.v3opt.S
 * This contains fast implementations of most rings.c functions
 * for V3.
 */

#include <asm_std.h>

#define NEXT 0
#define PREV 4

	/* Notes about fastrings:
	 * Only clobber r0-r7, p0-p3
	 * fused addremove uses r8-r9 also
	 * Assumes small/empty rings common
	 * Assumes ring aligned
	 */

// BLASTK_ring_remove_real(BLASTK_ringnode_t **ring, BLASTK_ringnode_t *node)
FUNC_START BLASTK_ring_remove_real  .text.h2.crit.ring  .p2align  3
	{
		r7 = memw(r0)
		r3:2 = memd(r1) // prev:next
	}
	{
		memw(r3+#NEXT) = r2 // prev->next = next
		p0 = cmp.eq(r7,r1)	// removing head?
		p1 = cmp.eq(r1,r2)	// node == node->next (only thing in list?)
	}
	{
		memw(r2+#PREV) = r3 // next->prev = prev
		if (!p0) jumpr r31  // didn't remove head, nothing to do
		if (p1) r2 = #0  // choose whether next or NULL
	}
	{
		memw(r0) = r2		// set head
		jumpr r31		// return
	}
FUNC_END BLASTK_ring_remove_real

// void BLASTK_ring_insert_real(BLASTK_ringnode_t **ring, BLASTK_ringnode_t *node)

	.falign
FUNC_START BLASTK_ring_insert_real .text.h2.crit.ring .falign
	{
		r7 = memw(r0)
		r5:4 = combine(r1,r1) // for case of empty 
	}
	{
		p0 = cmp.eq(r7,#0)
		if (!p0.new) r5:4 = memd(r7) // get *ring if not null
		memd(r1) = r5:4      // store in case empty -- dummy if not null
	}
	{
		memw(r0) = r1 // now points to inserted value
		if (p0) jumpr r31     // ring was empty, return
		if (!p0) r3:2 = combine(r5,r7) // *ring->prev,*ring
	}
	{
		memd(r1) = r3:2		// node->prev,node->next
	}
	{
		memw(r5+#NEXT) = r1		// *ring->prev->next = node
	}
	{
		memw(r7+#PREV) = r1		// *ring->prev = node
		jumpr r31
	}
FUNC_END BLASTK_ring_insert_real

// void BLASTK_ring_append_real(BLASTK_ringnode_t **ring, BLASTK_ringnode_t *node)

FUNC_START BLASTK_ring_append_real .falign
	{
		r7 = memw(r0)
		r5:4 = combine(r1,r1) // for case of empty 
	}
	{
		p0 = cmp.eq(r7,#0)
		if (!p0.new) r5:4 = memd(r7) // get *ring if not null
		memd(r1) = r5:4      // store in case empty -- dummy if not null
	}
	{
		if (p0) memw(r0) = r1 // if ring was empty, now points to inserted value
		if (p0) jumpr r31     // ring was empty, return
		if (!p0) r3:2 = combine(r5,r7) // *ring->prev,*ring
	}
	{
		memd(r1) = r3:2		// node->prev,node->next
	}
	{
		memw(r5+#NEXT) = r1		// *ring->prev->next = node
	}
	{
		memw(r7+#PREV) = r1		// *ring->prev = node
		jumpr r31
	}
FUNC_END BLASTK_ring_append_real

FUNC_START BLASTK_ring_remove_append_real .falign
	{	// 2 loads
		r3 = memw(r1)		// get *toring
		r9:8 = memd(r2)		// get *node
		r5:4 = combine(r2,r2) // for case of empty 
		p2 = or(p0,!p0)		// set p1 to true incase we skip it by emptying list
	}
	{	// 2 loads
		p0 = cmp.eq(r3,#0)
		p3 = cmp.eq(r2,r8)	// if node == node->next, only element in fromring
		r6 = memw(r0)
		if (!p0.new) r5:4 = memd(r3) // get **toring if not null
	}
	{	// store
		p2 = cmp.eq(r6,r2)	// does node == head?  If only element, it'd better...
		if (p0) memw(r1) = r2 // if toring was empty, now points to inserted value
		if (p3) r8 = #0
		if (p3) jump 1f		// fromring empty... don't need to update next/prev...
	}
	{	// store
		if (!p3) memw(r9+#NEXT) = r8	// update pointers in fromring
	}
	{	// store
		if (!p3) memw(r8+#PREV) = r9	// update pointers in fromring
	}
	.falign
1:
	{	// store
		if (!p0) r5:4 = combine(r5,r3) // *ring->prev,*ring
		if (p2) memw(r0) = r8
	}
	{	// store
		memd(r2) = r5:4		// node->prev,node->next
		if (p0) jumpr r31     // ring was empty, return
	}
	{	// store
		memw(r5+#NEXT) = r2		// *ring->prev->next = node
	}
	{	// store
		memw(r4+#PREV) = r2		// *ring->prev = node
		jumpr r31
	}
FUNC_END BLASTK_ring_remove_append_real

