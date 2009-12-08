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
		H2K_priomask |= 1<<hthread;
		H2K_wait_mask |= 1<<hthread;
		H2K_switch(me,NULL);
		/* EJP: should never get here! */
		H2K_check_sanity_unlock(0);
		return;
	}
	if ((H2K_wait_mask == 0) && (new->prio IS_WORSE_THAN H2K_runlist_worst_prio())) {
		if ((H2K_priomask & (1<<hthread)) != 0) {
			/* I am the new low priority thread */
			H2K_raise_lowprio();
			H2K_priomask |= 1<<hthread;
			lowprio_imask(hthread);
		}
	}
#if 0
	/* EJP: is this not needed? */
else {
		if ((H2K_priomask & (1<<hthread)) == 0) {
			highprio_imask(hthread);
		}
	}
#endif
	H2K_runlist_push(new);
	H2K_switch(me,new);
	/* EJP: should never get here! */
	H2K_check_sanity_unlock(0);
}

