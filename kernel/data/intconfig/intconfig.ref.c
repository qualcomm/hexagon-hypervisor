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

//void *H2K_fastint_funcptrs[MAX_INTERRUPTS] IN_SECTION(".data.event.interrupt");
//void *H2K_inthandlers[MAX_INTERRUPTS] IN_SECTION(".data.event.interrupt");
//u32_t H2K_fastint_mask IN_SECTION(".data.event.interrupt");
//u32_t H2K_fastint_gp IN_SECTION(".data.event.interrupt");

H2K_fastint_context H2K_fastint_contexts[MAX_HTHREADS];

#define FASTINT_TRAPMASK 0x9 /* ANGEL | FUTEX_RESUME */

void H2K_fastint();

void H2K_register_fastint(u32_t whatint, void (*fastint_handler)(u32_t x), H2K_thread_context *me)
{
	H2K_gp->fastint_funcptrs[whatint] = fastint_handler;
	H2K_gp->inthandlers[whatint] = H2K_fastint;
	H2K_gp->fastint_mask |= 1<<whatint;
	ciad(Q6_R_brev_R(1<<whatint));
	H2K_gp->fastint_gp = (u32_t)(me->ugpgp);
}

void H2K_intconfig_init()
{
	int i;
	H2K_thread_context *tmp;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->inthandlers[i] = NULL;
		H2K_gp->fastint_funcptrs[i] = NULL;
	}
	H2K_gp->fastint_mask = 0;
	H2K_gp->inthandlers[RESCHED_INT] = H2K_resched;
	for (i = 0; i < MAX_HTHREADS; i++) {
		tmp = &H2K_fastint_contexts[i].context;
		H2K_thread_context_clear(tmp);
		tmp->hthread = i;
		tmp->trapmask = FASTINT_TRAPMASK;
	}
}

