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
	 * Only clobber r0-r5, p0-p3
	 * fused addremove uses r6-r7 also
	 * Assumes small/empty rings common
	 * Assumes ring aligned
	 */

// H2K_ring_remove_real(H2K_ringnode_t **ring, H2K_ringnode_t *node)
FUNC_START H2K_ring_remove_real  .text.core.ring  .p2align  3
	{
		r4 = memw(r0)
		r3:2 = memd(r1) // prev:next
	}
	{
		memw(r3+#NEXT) = r2 	// prev->next = next
		memw(r2+#PREV) = r3 	// next->prev = prev
		p0 = cmp.eq(r4,r1)	// removing head?
		p1 = cmp.eq(r1,r2)	// node == node->next (only thing in list?)
	}
	{
		r2 = mux(p1,#0,r2)	// choose whether next or NULL
		if (p0) memw(r0+#0) = r2.new	// If head removed, update
		jumpr r31		// Done?
	}
FUNC_END H2K_ring_remove_real

// void H2K_ring_insert_real(H2K_ringnode_t **ring, H2K_ringnode_t *node)

	.falign
FUNC_START H2K_ring_insert_real .text.core.ring .falign
	{
		r2 = memw(r0)
		r5:4 = combine(r1,r1) // for case of empty 
	}
	{
		p0 = cmp.eq(r2,#0)
		memd(r1) = r5:4      // store in case empty -- dummy if not null
		memw(r0) = r1 // now points to inserted value
	}
	{
		if (!p0) r3 = memw(r2+#PREV) // get ring->prev if not null
		if (p0) jumpr r31     // ring was empty, return
		if (!p0) memw(r2+#PREV) = r1	// *ring->prev = node
	}
	{
		memw(r3+#NEXT) = r1		// *ring->prev->next = node
		memd(r1) = r3:2		// node->prev,node->next
		jumpr r31
	}
FUNC_END H2K_ring_insert_real

// void H2K_ring_append_real(H2K_ringnode_t **ring, H2K_ringnode_t *node)

FUNC_START H2K_ring_append_real .text.core.ring .falign
	{
		r2 = memw(r0)
		r5:4 = combine(r1,r1) // for case of empty 
	}
	{
		p0 = cmp.eq(r2,#0)
		memd(r1) = r5:4      // store in case empty -- dummy if not null
		if (!p0.new) r3 = memw(r2+#PREV)	// get *ring->prev
	}
	{
		if (p0) jumpr r31     // ring was empty, return
		if (!p0) memw(r2+#PREV) = r1	// *ring->prev = node
		if (p0) memw(r0) = r1 // if ring was empty, now points to appended value
	}
	{
		memd(r1) = r3:2		// node->prev,node->next
		memw(r3+#NEXT) = r1		// *ring->prev->next = node
		jumpr r31
	}
FUNC_END H2K_ring_append_real

FUNC_START H2K_ring_remove_append_real .text.core.ring .falign
	{	// 2 loads
		r6 = memw(r1)		// get *toring
		r5:4 = memd(r2)		// get *node
	}
	{	// 2 loads
		p0 = cmp.eq(r6,#0)	// toring empty?
		p3 = cmp.eq(r2,r4)	// if node == node->next, only element in fromring
		r3 = memw(r0)		// get *fromring
		if (!p0.new) r7 = memw(r6+#PREV) // get *toring->next if not null
	}
	{
		p2 = cmp.eq(r3,r2)	// does node == head?  If only element, it'd better...
		if (p0) memw(r1) = r2	// if toring was empty, now points to inserted value
		if (p3) r4 = #0		// fromring empty, use NULL instead of NEXT
		if (p0) r7:6 = combine(r2,r2)	// toring empty, next/prev = self
	}
	{	// 2 stores
		if (!p3) memw(r5+#NEXT) = r4	// update pointers in fromring if not empty
		if (!p3) memw(r4+#PREV) = r5	// update pointers in fromring if not empty
	}
	{	// 2 stores
		if (p2) memw(r0) = r4	// if fromring was pointing to me, store new head
		memd(r2) = r7:6		// node->prev,node->next
		if (p0) jumpr r31     // toring was empty, return
	}
	{	// 2 stores
		memw(r7+#NEXT) = r2		// *toring->prev->next = node
		memw(r6+#PREV) = r2		// *toring->prev = node
		jumpr r31
	}
FUNC_END H2K_ring_remove_append_real

