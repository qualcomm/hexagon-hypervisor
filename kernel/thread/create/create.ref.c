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

s32_t BLASTK_thread_create(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t asid, u32_t trapmask, BLASTK_thread_context *me)
{       
	u32_t i;
	BLASTK_thread_context *tmp;
	if (prio > MAX_PRIOS) return -1;        // bad prio
	if (asid > MAX_ASIDS) return -1;        // bad asid
	if ((sp & 7) != 0) return -1;           // bad stack pointer alignment
	do {    
		for (i = 0; i < MAX_THREADS; i++) {
			tmp = (void *)((char *)(&BLASTK_thread_contexts) + (i*THREAD_CONTEXT_TOTALSIZE));
			if (tmp->valid == 0) break;
		}
		if (i >= MAX_THREADS) return -1;
		BKL_LOCK(&BLASTK_bkl);
		if (tmp->valid) {
			/* Someone else claimed it just now */
			BKL_UNLOCK(&BLASTK_bkl);
			continue;
		} else {
			break;
		}
	} while (1);
	tmp->valid = 1;
	tmp->prio = prio;
	if (me) tmp->ugpgp = me->ugpgp;
	tmp->ssrelr = (((unsigned long long int)(SSR_DEFAULT | (asid << 8))) << 32)
			| ((unsigned long long int)pc);
	tmp->r2928 = ((unsigned long long int)sp) << 32;
	tmp->r0100 = arg1;

        ready_append(tmp);
        if (me) return BLASTK_check_sanity_unlock((u32_t)tmp); // otherwise we're starting up the boot thread, don't wake everyone
        return (s32_t)tmp;
}

