/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* resched.v4opt.S
 * This contains fast implementations of resched.ref.c function
 * for V4.
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

/*----------------------------------------------------------------------*/
/* H2K_resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum)      */
/*----------------------------------------------------------------------*/
/* Register Usage: R0-R8, R17-R20                                       */
/*----------------------------------------------------------------------*/

FUNC_START H2K_resched  .text.core.resched  .p2align 3
	{
		r0 = #RESCHED_INT_INTMASK
		r3 = #-1                                        // 0xFFFFFFFF
	}
	{
		ciad(r0)                                        // clear IAD
	}
	{
		k0lock                                          // BKL_LOCK(&H2K_bkl)
	}
	{
		if (r1!=#0) jump:t .H2K_resched_NOTWAIT         // if (me != NULL) then JUMP
		r3 = clrbit(r3,r2)                              // mask = clrbit(-1, hwtnum)
	}
	{
		memw(H2K_GP+#KG_wait_mask) &= r3                // H2k_gp_wait_mask =clrbit(H2K_gp->wait_mask,hwtnum)
		r1:0 = combine(r2,r1)                           // func call:
		call H2K_dosched                                //
	}

	.falign
.H2K_resched_NOTWAIT:
	{
		r6 = memub(r1+#CONTEXT_prio)                    // me->prio
		r5 = add(H2K_GP,#KG_runlist)                    // &H2K_gp->runlist[0]
		r0 = add(H2K_GP,#KG_ready)                      // &H2K_gp->ready[0]
		r4 = add(H2K_GP,#KG_runlist_prios)              // &H2K_gp->runlist_prios[0]
	}
	{
		r5 += asl(r2,#2)                                // &H2K_gp->runlist[hwtnum]
		r0 += asl(r6,#2)                                // func call: r0 = &H2K_gp->ready[pio]
		r20 = add(H2K_GP,#KG_ready_valids)              // &((u32_t *) H2K_gp->ready_valids)[0]
		r8 = and(r6,#0x1f)                              // prio & 0x1f
	}
	{
		memw(r5) = #0                                   // H2K_gp->runlist[hwtnum] = NULL
		r17 = lsl(#1,r8)                                // 1 << (prio & 0x3f)
		r4 += asl(r2,#1)                                // &H2K_gp->runlist_prios[hwtnum]
		r19:18 = combine(r2,r1)                         // save hwtnum : me in regs
	}
	{
		r20 = tableidxw(r6,#3,#5)                       // &((u32_t *) H2K_gp->ready_valids)[prio >> 5]
		memh(r4) = #-1                                  // H2K_gp->runlist_prios[hwtnum] = -1
		memb(r1+#CONTEXT_status) = #STATUS_READY        // me->status = H2K_STATUS_READY
		                                                // func call: r1 = me
		call H2K_ring_append_real                       //
	}
	{
		memw(r20) |= r17                                // ((u32_t *) H2K_gp->ready_valids)[prio >> 5] |= 1 << (prio & 0x3f)
		r1:0 = r19:18                                   // func call:
		call H2K_dosched                                //
	}
FUNC_END H2K_resched
