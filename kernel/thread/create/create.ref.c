/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * thread_create
 * Makes a new context 
 * Returns -1 on failure
 * returns threadid/threadptr on success
 */

#include <c_std.h>
#include <context.h>
#include <hw.h>
#include <readylist.h>
#include <thread.h>
#include <check_sanity.h>

s32_t BLASTK_thread_create(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t asid, u32_t trapmask, BLASTK_thread_context *me)
{       
	BLASTK_thread_context *tmp;
	u32_t myssr = (me->ssrelr >> 32);
	if (prio > MAX_PRIOS) return -1;        // bad prio
	if (asid > MAX_ASIDS) return -1;        // bad asid
	if ((sp & 7) != 0) return -1;           // bad stack pointer alignment
	BKL_LOCK(&BLASTK_bkl);
	if (BLASTK_free_threads == NULL) {
		BKL_UNLOCK(&BLASTK_bkl);
		return -1;
	}
	tmp = BLASTK_free_threads;
	BLASTK_free_threads = BLASTK_free_threads->next;
	tmp->valid = 1;
	tmp->prio = prio;
	if (me) tmp->ugpgp = me->ugpgp;
	tmp->ssrelr = (((u64_t)(myssr | (asid << 8))) << 32)
			| ((u64_t)pc);
	tmp->r2928 = ((u64_t)sp) << 32;
	tmp->r0100 = arg1;
	tmp->trapmask = trapmask & me->trapmask;
        BLASTK_ready_append(tmp);
        if (me) {
		return BLASTK_check_sanity_unlock((u32_t)tmp); // otherwise we're starting up the boot thread, don't wake everyone
	} else {
		BKL_UNLOCK(&BLASTK_bkl);
		return (s32_t)tmp;
	}
}

