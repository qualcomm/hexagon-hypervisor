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

void *BLASTK_fastint_funcptrs[MAX_INTERRUPTS] IN_SECTION(".data.event.interrupt");
void *BLASTK_inthandlers[MAX_INTERRUPTS] IN_SECTION(".data.event.interrupt");
u32_t BLASTK_fastint_mask IN_SECTION(".data.event.interrupt");
u32_t BLASTK_fastint_gp IN_SECTION(".data.event.interrupt");

BLASTK_fastint_context BLASTK_fastint_contexts[MAX_HTHREADS];

#define FASTINT_TRAPMASK 0x9 /* ANGEL | FUTEX_RESUME */

void BLASTK_fastint();

void BLASTK_register_fastint(u32_t whatint, void (*fastint_handler)(u32_t x), BLASTK_thread_context *me)
{
	BLASTK_fastint_funcptrs[whatint] = fastint_handler;
	BLASTK_inthandlers[whatint] = BLASTK_fastint;
	BLASTK_fastint_mask |= 1<<(31-whatint);
	ciad(Q6_R_brev_R(1<<whatint));
	BLASTK_fastint_gp = (u32_t)(me->ugpgp);
}

void BLASTK_intconfig_init()
{
	int i;
	BLASTK_thread_context *tmp;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		BLASTK_inthandlers[i] = NULL;
		BLASTK_fastint_funcptrs[i] = NULL;
	}
	BLASTK_fastint_mask = 0;
	BLASTK_inthandlers[RESCHED_INT] = BLASTK_resched;
	for (i = 0; i < MAX_HTHREADS; i++) {
		tmp = &BLASTK_fastint_context[i].context;
		BLASTK_thread_context_clear(tmp);
		tmp->hthread = i;
		tmp->trapmask = FASTINT_TRAPMASK;
	}
}

