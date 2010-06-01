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
        memw(r16+#KG_wait_mask) &= r3                   // H2k_gp_wait_mask =clrbit(H2K_gp->wait_mask,hwtnum)
        r1:0 = combine(r2,r1)                           // func call:
        call H2K_dosched                                // 
    }

    .falign
.H2K_resched_NOTWAIT:
    {
        r6 = memub(r1+#CONTEXT_prio)                    // me->prio
        r5 = add(r16,#KG_runlist)                       // &H2K_gp->runlist[0]
        r0 = add(r16,#KG_ready)                         // &H2K_gp->ready[0]
        nop                                             // alignment
    }
    {
        r5 += asl(r6,#2)                                // &H2K_gp->runlist[prio]
        r0 += asl(r6,#2)                                // func call: r0 = &H2K_gp->ready[pio]
    }
    {
        r4 = memw(r5)                                   // H2K_gp->runlist[prio]
        r3 = memw(r1+#CONTEXT_next)                     // me->next
        r5 = ADD(r5,#-CONTEXT_next)                     // &H2K_gp->runlist[prio](- CONTEXT_next)
        r17 = lsl(#1,r6)                                // 1<<prio
    }
    {
        p0 = cmp.eq(r4,r1)                              // if(H2K_gp->runlist[prio]== me)
        if ( p0.new) r4 = r3                            // updated H2K_gp->runlist[prio] in r4
        if ( p0.new) jump:t .H2K_resched_srchrunlistEnd // and skip runlist search
        if (!p0.new) r6 = r4                            // else tmp = H2K_gp->runlist[prio]
    }

    .falign
.H2K_resched_srchrunlist:
    {
        r5 = r6                                         // tmp = tmp->next
        r6 = memw(r6+#CONTEXT_next)                     // tmp->next
        if (!cmp.eq(r6.new,r1)) jump:t .H2K_resched_srchrunlist    
                                                        // if(tmp->next != me) 
                                                        // then continue look for thread in list
    }
.H2K_resched_srchrunlistEnd:
    {   
        memw(r5+#CONTEXT_next) = r3                     // H2K_gp->runlist[prio] or tmp->next = me->next
        if (r4!=#0) jump:t .H2K_resched_ready_append    // if (H2K_gp->runlist[prio] != NULL) then end runlist_remove()
        r6 = not(r17)                                   // ~(1<<prio)
    }
    {
        memw(r16+#KG_runlist_valids) &= r6              // H2K_gp->runlist_valids &= (~(1<<prio))
                                                        // EP: clearbit instead toggle bit
    }
.H2K_resched_ready_append:
    {
        memb(r1+#CONTEXT_status) = #STATUS_READY    // me->status = H2K_STATUS_READY
                                                        // func call: r1 = me
        r19:18 = combine(r2,r1)                         // save hwtnum : me in regs
        call H2K_ring_append_real                       // 
    }
    {
        memw(r16+#KG_ready_valids) |= r17               // H2K_gp->ready_valids|= 1<<prio
        r1:0 = r19:18                                   // func call:
        call H2K_dosched                                // 
    }
FUNC_END H2K_resched
