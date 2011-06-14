/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast.h>
#include <h2.h>
#include <blast_tls.h>
#include <blast_thread.h>

#include <string.h>

/* mostly from blast_thread.c */
int blast_thread_initial_setup (struct BLAST_ugp_ptr *pUgp)
{
   /* Set UGP Value */

	__asm__ __volatile__ (
												"ugp = %0\n"
												:
												:"r"(pUgp)
												);

   /* Set thread_id */
	/* also cause ugp to be saved in context */
   pUgp->utcb.thread_id = h2_thread_get_tid();

   /* Initialize the anysignal */
   blast_anysignal_init (&pUgp->utcb.anysignal);

   return 0;
}

/* from blast_thread.c */
void blast_trampoline (void *arg)
{
   struct BLAST_ugp_ptr *pUgp;

   pUgp = (struct BLAST_ugp_ptr *)arg;

   /* Initialize UGP and UTCB variables */
   blast_thread_initial_setup (pUgp);

   /* Enter thread entry */
   pUgp->utcb.entrypoint (pUgp->utcb.arg);
}

/* FIXME: hackage */
int blast_thread_create(void *pc, void *stack, void *arg, 
	unsigned int prio, unsigned int asid, unsigned int hw_bitmask)
{
	/*
	 * H2's thread create prototype is:
	 * H2K_thread_create(u32_t pc, u32_t sp, u32_t arg, u32_t prio, u32_t trapmask,
	 *    H2K_thread_context *me)
	 * The trap automatically fills in the *me pointer, so it's really just that
	 * ASID thing that needs changing.  Just drop that on the floor.
	 * drop the hw_bitmask on the floor
	 * might as well fudge the priority too 
	 */

	struct BLAST_ugp_ptr *pUgp;

	/* from blast_thread.c */
	pUgp = (struct BLAST_ugp_ptr *)blast_malloc (sizeof (struct BLAST_ugp_ptr));
	assert(pUgp);
	memset (pUgp, 0, sizeof (struct BLAST_ugp_ptr));
	pUgp->utcb.entrypoint = pc;
	pUgp->utcb.arg = arg;
	pUgp->utcb.stack_ptr = (unsigned int *)stack;

	//	return h2_thread_create(pc,stack,arg,prio);
	return h2_thread_create((void *)blast_trampoline, stack, (void *)pUgp, prio);
}

