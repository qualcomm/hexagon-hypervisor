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
#include <id.h>

void H2K_interrupt_restore();

IN_SECTION(".text.misc.create") s32_t H2K_thread_create_no_squash(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me)
{       
	H2K_thread_context *tmp;
	u32_t bestprio;
	u32_t trapmask;
	u32_t ptb;
	translation_type type;
	s32_t asid;
	void *guest_evb;

	bestprio = vmblock->bestprio;
	trapmask = vmblock->trapmask;

	if (vmblock->num_cpus == 0) { // starting first cpu
		guest_evb = NULL;
		type = vmblock->pmap_type;
		if (type == H2K_ASID_TRANS_TYPE_OFFSET) { // use vmblock as "ptb"
			ptb = (u32_t)vmblock;
		} else {
			ptb = vmblock->pmap; 				/* initial page tables == pmap */
		}
	} else { // inherit
		guest_evb = me->gevb;
		ptb = H2K_mem_asid_table[me->ssr_asid].ptb;
		type = H2K_mem_asid_table[me->ssr_asid].fields.transtype;
	}

	if (prio > MAX_PRIO) return -1;        // bad prio
	if (prio < bestprio) return -1;	       // priority better than allowed

	if ((sp & 7) != 0) return -1;          // bad stack pointer alignment
	if ((pc & 3) != 0) return -1;          // bad pc alignment

	BKL_LOCK(&H2K_bkl);
	if (vmblock->free_threads == NULL) {
		BKL_UNLOCK(&H2K_bkl);
		return -1;
	}
	tmp = vmblock->free_threads;
	vmblock->free_threads = vmblock->free_threads->next;
	tmp->base_prio = tmp->prio = prio;
	tmp->gp = H2K_get_gp();
	tmp->usr = me->usr;
	tmp->ssr = me->ssr;
	tmp->elr = pc;
	tmp->r29 = sp;
	tmp->r0100 = arg1;
	tmp->ccr = H2K_get_ccr();
	tmp->trapmask = trapmask;
	tmp->continuation = H2K_interrupt_restore;
	tmp->vmstatus = 0x0;            // all clear
	tmp->gevb = guest_evb;

	asid = H2K_asid_table_inc(ptb, type, H2K_ASID_TLB_INVALIDATE_FALSE, NULL);

	if (asid == -1) { 						// can't allocate asid
		vmblock->free_threads = tmp; // return to free list
		BKL_UNLOCK(&H2K_bkl);
		return -1;
	}

	tmp->ssr_guest = 1;  // start in guest mode
	tmp->ssr_um = 1;
	tmp->ssr_asid = asid;
#ifdef HAVE_EXTENSIONS
  tmp->ssr_xa = 0; // no ext active
  tmp->ssr_xe = 0; // ext disabled to cause exception
#endif

	vmblock->num_cpus++;
	tmp->vmblock = vmblock;

	H2K_ready_append(tmp);
	return H2K_check_sanity_unlock(H2K_id_from_context(tmp).raw);
}

IN_SECTION(".text.misc.create") s32_t H2K_thread_create(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me)
{       
	return H2K_thread_create_no_squash(pc, sp, arg1, prio, me->vmblock, me);
}
