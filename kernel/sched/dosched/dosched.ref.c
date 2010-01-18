/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <lowprio.h>
#include <readylist.h>
#include <runlist.h>
#include <check_sanity.h>
#include <switch.h>

void H2K_dosched(H2K_thread_context *me,u32_t hthread)
{
	H2K_thread_context *new;
	new = H2K_ready_getbest();
	if (new == NULL) {
		/* GO TO SLEEP */
		H2K_raise_lowprio();
		/* EJP: is this done in switch now?!?!? */
		//H2K_priomask |= 1<<hthread;
		//H2K_wait_mask |= 1<<hthread;
		/* EJP: Partial sanity check: reraise resched for scheduler
 		 * mask if further boot-offs needed */
		H2K_check_sched_mask();
		H2K_switch(me,NULL);
		/* EJP: should never get here! */
		return;
	}
	if ((H2K_wait_mask == 0) && (new->prio IS_WORSE_THAN H2K_runlist_worst_prio())) {
		/* If no threads are waiting and this new priority is worse than everyone else... */
		if ((H2K_priomask & (1<<hthread)) == 0) {
			/* And I am not already marked as the lowest priority thread... */
			/* I am the new low priority thread */
			H2K_raise_lowprio();
			H2K_priomask |= 1<<hthread;
			lowprio_imask(hthread);
		}
	} else {
		if ((H2K_priomask & (1<<hthread)) != 0) {
			H2K_priomask = Q6_R_clrbit_RR(H2K_priomask,hthread);
			highprio_imask(hthread);
		}
	}
	H2K_runlist_push(new);
	H2K_switch(me,new);
	/* EJP: should never get here! */
	return;
}

