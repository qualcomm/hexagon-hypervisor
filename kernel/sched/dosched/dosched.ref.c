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
#include <globals.h>
#include <dosched.h>

void H2K_dosched(H2K_thread_context *me,u32_t hthread)
{
	H2K_thread_context *new;
	new = H2K_ready_getbest();
	if (new == NULL) {
		/* GO TO SLEEP */
		H2K_raise_lowprio();
		H2K_switch(me,NULL);
		/* EJP: should never get here! */
	}
	if ((H2K_gp->wait_mask == 0) && (new->prio IS_WORSE_THAN H2K_runlist_worst_prio())) {
		/* If no threads are waiting and this new priority is worse than everyone else... */
		if ((H2K_gp->priomask & (1<<hthread)) == 0) {
			/* And I am not already marked as the lowest priority thread... */
			/* I am the new low priority thread */
			H2K_raise_lowprio();
			H2K_gp->priomask |= 1<<hthread;
			lowprio_imask(hthread);
		}
	} else {
		if ((H2K_gp->priomask & (1<<hthread)) != 0) {
			H2K_gp->priomask = Q6_R_clrbit_RR(H2K_gp->priomask,hthread);
			highprio_imask(hthread);
		}
	}
	new->hthread = hthread;
	H2K_runlist_push(new);
	//if (new->vmstatus & H2K_VMSTATUS_VMWORK) H2K_vm_do_work(new);
	H2K_switch(me,new);
	/* EJP: should never get here! */
}

