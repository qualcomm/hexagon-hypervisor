/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* resched.v4opt.S
 * This contains fast implementations of resched.cc function
 * for V4.
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

/*----------------------------------------------------------------------*/
/* H2K_resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum)      */
/*----------------------------------------------------------------------*/
/* Register Usage: R0-R6, R17-R19                                       */
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
		memw(H2K_GP+#KG_wait_mask) &= r3                // H2k_gp_wait_mask =clrbit(H2K_GP->wait_mask,hwtnum)
		r1:0 = combine(r2,r1)                           // func call:
		call H2K_dosched                                //
	}

	.falign
.H2K_resched_NOTWAIT:
	{
		r6 = memub(r1+#CONTEXT_prio)                    // me->prio
		r5 = add(H2K_GP,#KG_runlist)                    // &H2K_GP->runlist[0]
		r0 = add(H2K_GP,#KG_ready)                      // &H2K_GP->ready[0]
		r4 = add(H2K_GP,#KG_runlist_prios)              // &H2K_GP->runlist_prios[0]
	}
	{
		r5 += asl(r2,#2)                                // &H2K_GP->runlist[hwtnum]
		r0 += asl(r6,#2)                                // func call: r0 = &H2K_GP->ready[pio]
	}
	{
		memw(r5) = #0                                   // H2K_GP->runlist[hwtnum] = NULL
		r17 = lsl(#1,r6)                                // 1<<prio
		r4 += asl(r2,#1)                                // &H2K_GP->runlist_prios[hwtnum]
	}
	{
		memh(r4) = #-1                                  // H2K_GP->runlist_prios[hwtnum] = -1
		memb(r1+#CONTEXT_status) = #STATUS_READY        // me->status = H2K_STATUS_READY
		                                                // func call: r1 = me
		r19:18 = combine(r2,r1)                         // save hwtnum : me in regs
		call H2K_ring_append_real                       //
	}
	{
		memw(H2K_GP+#KG_ready_valids) |= r17            // H2K_GP->ready_valids|= 1<<prio
		r1:0 = r19:18                                   // func call:
		call H2K_dosched                                //
	}
FUNC_END H2K_resched
