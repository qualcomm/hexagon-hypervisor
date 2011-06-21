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
#include <vm.h>

void H2K_interrupt_restore();

IN_SECTION(".text.misc.create") s32_t H2K_thread_create_no_squash(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me)
{       
	H2K_thread_context *tmp;
	u32_t bestprio;
	u32_t trapmask;
	u32_t ptb;
	translation_type type;
	s32_t asid;

	/* get priority, trapmask, and ptb from VM block if valid */
	if (vmblock) {
		bestprio = vmblock->bestprio;
		trapmask = vmblock->trapmask;
		ptb = vmblock->pmap; 				/* initial page tables == pmap */
		type = vmblock->pmap_type;
	}
	else {
		bestprio = me->base_prio;
		trapmask = me->trapmask;
		ptb = H2K_mem_asid_table[me->ssr_asid].ptb;
		type = H2K_mem_asid_table[me->ssr_asid].transtype;
	}

	if (prio > MAX_PRIO) return -1;        // bad prio
#warning this is broken but required for audio
	//if (prio < bestprio) return -1;	       // priority better than allowed

	if ((sp & 7) != 0) return -1;          // bad stack pointer alignment
	if ((pc & 3) != 0) return -1;          // bad pc alignment

	BKL_LOCK(&H2K_bkl);
	if (H2K_gp->free_threads == NULL) {
		BKL_UNLOCK(&H2K_bkl);
		return -1;
	}
	tmp = H2K_gp->free_threads;
	H2K_gp->free_threads = H2K_gp->free_threads->next;
	tmp->base_prio = tmp->prio = prio;
	tmp->ugpgp = me->ugpgp;
	tmp->sr = me->sr;
	tmp->ssr = me->ssr;
	tmp->elr = pc;
	tmp->r29 = sp;
	tmp->r0100 = arg1;
	tmp->ccr = me->ccr;
	tmp->trapmask = trapmask;
	tmp->continuation = H2K_interrupt_restore;

	asid = H2K_asid_table_inc(ptb, type);

	if (vmblock) {
		/* only need to check asid if we have a vmblock, else it's inherited */
		if (asid == -1) { 						// can't allocate asid
			H2K_gp->free_threads = tmp; // return to free list
			BKL_UNLOCK(&H2K_bkl);
			return -1;
		}
		if (vmblock->num_cpus == vmblock->max_cpus) {  // no more vcpus
			H2K_asid_table_dec(asid); // was bogus
			H2K_gp->free_threads = tmp; // return to free list
			BKL_UNLOCK(&H2K_bkl);
			return -1;
		}

		tmp->ssr_asid = asid;
		tmp->vmcpu = vmblock->num_cpus++;
		tmp->vmstatus = 0x0;            // all clear
		tmp->vmblock = vmblock;

		vmblock->cpu_contexts[tmp->vmcpu] = tmp;
	}
	H2K_ready_append(tmp);
	return H2K_check_sanity_unlock((u32_t)tmp);
}

IN_SECTION(".text.misc.create") s32_t H2K_thread_create(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me)
{       
	return H2K_thread_create_no_squash(pc, sp, arg1, prio, NULL, me);
}
