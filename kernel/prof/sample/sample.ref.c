/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include  <globals.h>
#include <fatal.h>
#include <atomic.h>
#include <vmipi.h>
#include <hw.h>
#include <interrupt.h>
#include <switch.h>

void H2K_sample(u32_t hthread, H2K_thread_context *me) {

	if (H2K_atomic_clrbit(&H2K_gp->prof_sample_pending, hthread)) { // not done
		if (NULL != me) {
			H2K_gp->prof_sample[hthread] = ((u64_t)(me->id.raw) << 32) | H2K_get_elr();
		} else {
			H2K_gp->prof_sample[hthread] = 0ULL;
		}
	}
}

u32_t H2K_sample_start(H2K_thread_context *me) {
	u32_t i;
	u32_t volatile *pending = &H2K_gp->prof_sample_pending;

	if (H2K_spinlock_trylock(&H2K_gp->prof_sample_lock)) {  // another sample in progress
		return 0;
	}

	/* Tell all the other hw threads to sample */
	H2K_gp->prof_sample_pending = H2K_gp->hthreads_mask & ~(1 << me->hthread);
	H2K_vm_ipi_send_mask(H2K_gp->prof_sample_pending);

	while (*pending);  // wait for all to finish

	for (i = 0; i < MAX_HTHREADS; i++) {
		if (H2K_gp->hthreads_mask & (1 << i)) {
			switch (i) {  // can't assume a particular reg order in the context
			case 0:
				me->r0302 = H2K_gp->prof_sample[i];
				break;
			case 1:
				me->r0504 = H2K_gp->prof_sample[i];
				break;
			case 2:
				me->r0706 = H2K_gp->prof_sample[i];
				break;
			case 3:
				me->r0908 = H2K_gp->prof_sample[i];
				break;
			case 4:
				me->r1110 = H2K_gp->prof_sample[i];
				break;
			case 5:
				me->r1312 = H2K_gp->prof_sample[i];
				break;
			default:
				/* So far we have at most 6 hw threads */
				H2K_fatal_kernel(0, me, 0, 0, me->hthread);
			}
		}
	}
	H2K_spinlock_unlock(&H2K_gp->prof_sample_lock);

	me->continuation = H2K_interrupt_restore;
	me->r0100 = H2K_gp->hthreads_mask & ~(1 << me->hthread);
	H2K_switch(me, me);

	/* not reached */
	return H2K_gp->hthreads_mask & ~(1 << me->hthread);
}

void H2K_sample_init(void) {
	H2K_spinlock_init(&H2K_gp->prof_sample_lock);
}
