/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* resched.v3opt.S
 * This contains fast implementations of resched.ref.c function
 * for V3.
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

/*----------------------------------------------------------------------*/
/* H2K_resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum)      */
/*----------------------------------------------------------------------*/
/* Register Usage: R0-R11, R17-R20                                      */
/*----------------------------------------------------------------------*/

FUNC_START H2K_resched  .text.core.resched  .p2align 3
	{
		r0.h = #hi(RESCHED_INT_INTMASK)
	}
	{
		r0.l = #lo(RESCHED_INT_INTMASK)
	}
	{
		ciad(r0)                                        // clear IAD
	}
	{
		k0lock                                          // BKL_LOCK(&H2K_bkl)
	}
	{
		p0 = cmp.eq(r1,#0)                              // me == NULL
		if (!p0.new) jump:t .H2K_resched_NOTWAIT        // if (me != NULL) then JUMP
		if (p0.new) r3 = memw(H2K_GP+#KG_wait_mask)     // H2K_gp->wait_mask
	}
	{
		r3 = clrbit(r3,r2)                              // clrbit(H2K_gp->wait_mask,hwtnum)
	}
	{
		memw(H2K_GP+#KG_wait_mask) = r3                 // H2k_gp_wait_mask =clrbit(H2K_gp->wait_mask,hwtnum)
		r1:0 = combine(r2,r1)                           // func call:
		call H2K_dosched                                //
	}

	.falign
.H2K_resched_NOTWAIT:
	{
		r6 = memub(r1+#CONTEXT_prio)                    // me->prio
		r20 = add(H2K_GP,#KG_ready_valids)              // &H2K_gp->ready_valids[0]
		r5 = add(H2K_GP,#KG_runlist)                    // &H2K_gp->runlist[0]
		r0 = add(H2K_GP,#KG_ready)                      // &H2K_gp->ready[0]
	}
	{
		r20 = tableidxw(r6,#3,#5)                       // &((u32_t *) H2K_gp->ready_valids)[prio >> 5]
		r7 = and(r6,#0x1f)                              // prio & 0x1f
		r4 = add(H2K_GP,#KG_runlist_prios)              // &H2K_gp->runlist_prios[0]
	}
	{
		r5 += asl(r2,#2)                                // &H2K_gp->runlist[hwtnum]
		r0 += asl(r6,#2)                                // func call: r0 = &H2K_gp->ready[prio]
		r11:10 = combine(#1,#0)
		r17 = memw(r20)                                 // ((u32_t *) H2K_gp->ready_valids)[prio >> 5]
	}
	{
		memw(r5) = r10                                  // H2K_gp->runlist[hwtnum] = NULL
		r17 = setbit(r17,r7)                            // ((u32_t *) H2K_gp->ready_valids)[prio >> 5] | 1 << (prio & 0x1f)
		r4 += asl(r2,#1)                                // &H2K_gp->runlist_prios[hwtnum]
		r7 = #-1
	}
	{
		memh(r4) = r7                                   // H2K_gp->runlist_prios[hwtnum] = -1
		r7 = #STATUS_READY
	}
	{
		memb(r1+#CONTEXT_status) = r7                   // me->status = H2K_STATUS_READY
		                                                // func call: r1 = me
		r19:18 = combine(r2,r1)                         // save hwtnum : me in regs
		call H2K_ring_append_real                       //
	}
	{
		memw(r20) = r17                                 // ((u32_t *) H2K_gp->ready_valids)[prio >> 5] |= 1 << (prio & 0x1f)
		r1:0 = r19:18                                   // func call:
		call H2K_dosched                                //
	}
FUNC_END H2K_resched
