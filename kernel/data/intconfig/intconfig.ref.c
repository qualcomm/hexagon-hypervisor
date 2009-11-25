/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <intconfig.h>
#include <resched.h>
#include <max.h>
#include <q6protos.h>
#include <hw.h>

void *BLASTK_fastint_funcptrs[MAX_INTERRUPTS];
void *BLASTK_inthandlers[MAX_INTERRUPTS];
u32_t BLASTK_fastint_mask;

void BLASTK_fastint();

void BLASTK_register_fastint(u32_t whatint, void (*fastint_handler)(u32_t x), BLASTK_thread_context *me)
{
	BLASTK_fastint_funcptrs[whatint] = fastint_handler;
	BLASTK_inthandlers[whatint] = BLASTK_fastint;
	BLASTK_fastint_mask |= 1<<(31-whatint);
	ciad(Q6_R_brev_R(1<<whatint));
}

void BLASTK_intconfig_init()
{
	int i;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		BLASTK_inthandlers[i] = NULL;
		BLASTK_fastint_funcptrs[i] = NULL;
	}
	BLASTK_inthandlers[RESCHED_INT] = BLASTK_reschedule_from_lowprio;
}

