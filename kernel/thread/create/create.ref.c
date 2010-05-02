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
#include <asid.h>

void H2K_interrupt_restore();

s32_t H2K_thread_create(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t trapmask, H2K_thread_context *me)
{       
	H2K_thread_context *tmp;
	u32_t myssr;
	myssr = me->ssr;
	if (prio > MAX_PRIO) return -1;        // bad prio
	if ((sp & 7) != 0) return -1;           // bad stack pointer alignment
	if ((pc & 3) != 0) return -1;           // bad pc alignment
	if (prio < me->base_prio) return -1;	// can't spawn higher-priority children
	BKL_LOCK(&H2K_bkl);
	if (H2K_gp->free_threads == NULL) {
		BKL_UNLOCK(&H2K_bkl);
		return -1;
	}
	tmp = H2K_gp->free_threads;
	H2K_gp->free_threads = H2K_gp->free_threads->next;
	tmp->base_prio = tmp->prio = prio;
	tmp->ugpgp = me->ugpgp;
	tmp->ssr = myssr;
	tmp->elr = pc;
	tmp->r29 = sp;
	tmp->r0100 = arg1;
	H2K_asid_table_inc(H2K_mem_asid_table[me->ssr_asid].ptb);
	tmp->ccr = me->ccr;
	tmp->trapmask = trapmask & me->trapmask;
	tmp->continuation = H2K_interrupt_restore;
        H2K_ready_append(tmp);
	return H2K_check_sanity_unlock((u32_t)tmp);
}

