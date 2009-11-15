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

void BLASTK_dosched(BLASTK_thread_context *me,u32_t hthread)
{
	BLASTK_thread_context *new;
	new = BLASTK_ready_getbest();
	if (new == NULL) {
		/* GO TO SLEEP */
		BLASTK_raise_lowprio();
		BLASTK_priomask |= 1<<hthread;
		BLASTK_wait_mask |= 1<<hthread;
		BLASTK_switch(me,NULL);
		/* EJP: should never get here! */
		BLASTK_check_sanity_unlock(0);
		return;
	}
	if ((BLASTK_wait_mask == 0) && (new->prio IS_WORSE_THAN BLASTK_runlist_worst_prio())) {
		if ((BLASTK_priomask & (1<<hthread)) != 0) {
			/* I am the new low priority thread */
			BLASTK_raise_lowprio();
			BLASTK_priomask |= 1<<hthread;
			lowprio_imask(hthread);
		}
	}
#if 0
	/* EJP: is this not needed? */
else {
		if ((BLASTK_priomask & (1<<hthread)) == 0) {
			highprio_imask(hthread);
		}
	}
#endif
	BLASTK_runlist_push(new);
	BLASTK_switch(me,new);
	/* EJP: should never get here! */
	BLASTK_check_sanity_unlock(0);
}

