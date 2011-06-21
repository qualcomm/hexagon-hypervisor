/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <intconfig.h>
#include <resched.h>
#include <max.h>
#include <q6protos.h>
#include <thread.h>
#include <hw.h>
#include <globals.h>

H2K_fastint_context H2K_fastint_contexts[MAX_HTHREADS];

#define FASTINT_TRAPMASK 0x9 /* ANGEL | FUTEX_RESUME */

void H2K_fastint();

void H2K_register_fastint(u32_t whatint, int (*fastint_handler)(u32_t x), H2K_thread_context *me)
{
	u32_t ciad_intmask;
	if (fastint_handler == NULL) { /* deregister */
		H2K_gp->inthandlers[whatint] = NULL;
		H2K_gp->fastint_funcptrs[whatint] = NULL;
	}
	else {
		H2K_gp->fastint_funcptrs[whatint] = fastint_handler;
		H2K_gp->inthandlers[whatint] = H2K_fastint;
		ciad_intmask = 1<<whatint;
#if __QDSP6_ARCH__ <= 3
		ciad_intmask = Q6_R_brev_R(ciad_intmask);
#endif
		ciad(ciad_intmask);
		H2K_gp->fastint_gp = (u32_t)(me->ugpgp);
	}
}

void H2K_intconfig_init()
{
	int i;
	H2K_thread_context *tmp;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->inthandlers[i] = NULL;
		H2K_gp->fastint_funcptrs[i] = NULL;
	}
	H2K_gp->inthandlers[RESCHED_INT] = H2K_resched;
	for (i = 0; i < MAX_HTHREADS; i++) {
		tmp = &H2K_fastint_contexts[i].context;
		H2K_thread_context_clear(tmp);
		tmp->hthread = i;
		tmp->trapmask = FASTINT_TRAPMASK;
	}
}

