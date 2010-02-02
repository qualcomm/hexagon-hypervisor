/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* resched.v3opt.S 
 * This contains fast implementations of resched.cc function
 * for V3.
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

/*----------------------------------------------------------------------*/
/* H2K_resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum)      */
/*----------------------------------------------------------------------*/
/* Register Usage: R0-R6, R17-R19                                       */
/*----------------------------------------------------------------------*/

FUNC_START H2K_resched  .text.h2.crit.resched  .p2align 3
    {
        r0.h = #HI(RESCHED_INT_INTMASK)                 // RESCHED_INIT_INTMASK
    }
    {
        r0.l = #LO(RESCHED_INT_INTMASK)                 // 
    }
    {
        ciad(r0)                                        // clear IAD 
    }
    {
        k0lock                                          // BKL_LOCK(&H2K_bkl)
    }
    {
        p0 = cmp.eq(r1,#0)                              // if(me!=NULL) 
        if (!p0.new) jump:t .H2K_resched_NOTWAIT        // then jump                                    
        if ( p0.new) r3 = memw(r16+#KG_wait_mask)       // otherwise load H2K_gp->wait_mask
    }
    {
        r3=clrbit(r3,r2)                                // clrbit(H2K_gp->wait_mask,hwtnum)
    }
    {
        memw(r16+#KG_wait_mask) = r3                    // save H2K_gp->wait_mask
        r1:0 = combine(r2,r1)                           // func call: r0 = me, r1=hwtnum
        call H2K_dosched                                // H2K_dosched(me,hwtnum)
        NOP
    }
    // return 
    
.H2K_resched_NOTWAIT:
    {
        r17 = memub(r1+#CONTEXT_prio)                   // me->prio
        r5 = add(r16,#KG_runlist)                       // &H2K_gp->runlist[0]
        r0 = add(r16,#KG_ready)                         // &H2K_gp->ready[0]
    }
    { 
        r5 += asl(r17,#2)                               // &H2K_gp->runlist[prio]
        r0 += asl(r17,#2)                               // func call: r0 = &H2K_gp->ready[prio]
    }
    {
        r4 = memw(r5)                                   // H2K_gp->runlist[prio]
        r3 = memw(r1+#CONTEXT_next)                     // me->next
        r5 = add(r5,#-CONTEXT_next)                     // &H2K_gp->runlist[prio] (-CONTEXT_next)
    }
    {
        p0 = cmp.eq(r4,r1)                              // if(H2K_gp->runlist[prio]==me)
        if ( p0.new) r4 = r3                            // then update H2K_gp->runlist[prio] in reg
        if ( p0.new) jump:nt .H2K_resched_srchrunlistEnd// then skip runlist search
        if (!p0.new) r6 = r4                            // else tmp = H2K_gp->runlist[prio]
    }
.H2K_resched_srchrunlist:
    {
        r5 = r6                                         // tmp = tmp->next
        r6 = memw(r6+#CONTEXT_next)                     // tmp->next
    }
    {
        p0 = cmp.eq(r6,r1)                              // if(tmp->next != me)
        if (!p0.new) jump:t .H2K_resched_srchrunlist    // then continue look for thread in list
    }

.H2K_resched_srchrunlistEnd:
    {
        memw(r5+#CONTEXT_next) = r3                     // H2K_gp->runlist[prio] or tmp->next = me->next
        if (r4!=#0) jump:nt .H2K_resched_ready_append   // if (H2K_gp->runlist[prio] !=NULL) then end runlist_remove()
        r6 = #STATUS_READY
    }
    {
        r3 = memw(r16+#KG_runlist_valids)               // H2K_gp->runlist_valids ^= 1<<prio
    }
    {
        r3 = togglebit(r3,r17)                          //
    }
    {
        memw(r16+#KG_runlist_valids) = r3               //
        NOP; NOP
    }

.H2K_resched_ready_append:
    {
        memb(r1+#CONTEXT_status) = r6                   // me->status = STATUS_READY
        r19:18 = combine(r2,r1)                         // save hwtnum : me in regs
        call H2K_ring_append_real                       // H2K_ring_append( &H2K_gp->ready[prio],me)
    }
    {
        r0 = memw(r16+#KG_ready_valids)                 // H2K_gp->ready_valids |= 1<<prio
    }
    {
        r0 = setbit(r0,r17)                             //
    }
    {
        memw(r16+#KG_ready_valids) = r0                 //
        r1:0 = r19:18                                   // func call: r0=me, r1= hwtnum
        call H2K_dosched                                // H2K_dosched(me, hwtnum)
    }
FUNC_END H2K_resched

