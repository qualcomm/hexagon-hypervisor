/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

void BLASTK_dosched(BLASTK_thread_context *me,u32_t hthread)
{
	BLASTK_thread_context *new;
	new = ready_getbest();
	if (new == NULL) {
		/* GO TO SLEEP */
		raise_lowprio();
		BLASTK_priomask |= 1<<hthread;
		BLASTK_wait_mask |= 1<<hthread;
		BLASTK_thread_switch(me,NULL);
		BLASTK_check_sanity();
		BKL_UNLOCK(&BLASTK_bkl);
		return;
	}
	if ((BLASTK_wait_mask == 0) && (new->prio > get_worst_running_prio())) {
		if ((BLASTK_priomask & (1<<hthread)) != 0) {
			/* I am the new low priority thread */
			raise_lowprio();
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
	runlist_push(new);
	BLASTK_thread_switch(me,new);
	BLASTK_check_sanity();
	BKL_UNLOCK(&BLASTK_bkl);
}

