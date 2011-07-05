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
#include <vmipi.h>

H2K_fastint_context H2K_fastint_contexts[MAX_HTHREADS];

#define FASTINT_TRAPMASK 0x9 /* ANGEL | FUTEX_RESUME */

void H2K_fastint();

#ifdef H2K_L2_CONTROL
static void H2K_fastint_enable_l2(u32_t whatint)
{
	volatile unsigned int *l2_enable = (void *)H2K_gp->l2_ack_base;
	whatint -= 32;
	l2_enable[(whatint)/32] = (1 << (whatint & 0x1f));
}

static void H2K_fastint_disable_l2(u32_t whatint)
{
	volatile unsigned int *l2_disable = (void *)(((unsigned int)H2K_gp->l2_int_base) + 0x180);
	whatint -= 32;
	l2_disable[(whatint)/32] = (1 << (whatint & 0x1f));
}

#else
static inline void H2K_fastint_enable_l2(int whatint) {}
static inline void H2K_fastint_disable_l2(int whatint) {}
#endif

static void H2K_fastint_disable(u32_t whatint)
{
	u32_t siad_intmask;
	siad_intmask = 1<<whatint;
	if (whatint < 32) {
#if __QDSP6_ARCH__ >= 4
		siad(siad_intmask);
#endif
		return;
	} else H2K_fastint_disable_l2(whatint);
}

static void H2K_fastint_enable(u32_t whatint)
{
	u32_t ciad_intmask;
	if (whatint < 32) {
		ciad_intmask = 1<<whatint;
#if __QDSP6_ARCH__ <= 3
		ciad_intmask = Q6_R_brev_R(ciad_intmask);
#endif
		ciad(ciad_intmask);
	} else H2K_fastint_enable_l2(whatint);
}

void H2K_register_fastint(u32_t whatint, int (*fastint_handler)(u32_t x), H2K_thread_context *me)
{
	if (fastint_handler == NULL) { /* deregister */
		H2K_fastint_disable(whatint);
		H2K_gp->inthandlers[whatint] = NULL;
		H2K_gp->fastint_funcptrs[whatint] = NULL;
	}
	else {
		H2K_gp->fastint_funcptrs[whatint] = fastint_handler;
		H2K_gp->inthandlers[whatint] = H2K_fastint;
		H2K_fastint_enable(whatint);

		H2K_gp->fastint_gp = (u32_t)(me->gp);
	}
}

#ifdef H2K_L2_CONTROL
static void H2K_intconfig_l2_init()
{
	int i;
	volatile unsigned int *intbase = H2K_gp->l2_int_base;
	for (i = 0; i < ((MAX_INTERRUPTS-32)/32); i++) {
		intbase[(0x100/4) + i] = 0x0; 		/* DISABLED */
		intbase[(0x280/4) + i] = 0xff57f7fd;	/* EDGE/level TRIGGERED */
		intbase[(0x300/4) + i] = 0x0;		/* Rising Edge / Level High */
		intbase[(0x400/4) + i] = 0xffffffff;	/* Interrupt Clear */
	}
	ciad(0x80000000);				/* Enable L2 Interrupts */
}
#else
static inline void H2K_intconfig_l2_init() {}
#endif

void H2K_intconfig_init()
{
	int i;
	H2K_thread_context *tmp;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->inthandlers[i] = NULL;
		H2K_gp->fastint_funcptrs[i] = NULL;
	}
	H2K_gp->inthandlers[RESCHED_INT] = H2K_resched;
	H2K_gp->inthandlers[VM_IPI_INT] = H2K_vm_ipi_do;
	for (i = 0; i < MAX_HTHREADS; i++) {
		tmp = &H2K_fastint_contexts[i].context;
		H2K_thread_context_clear(tmp);
		tmp->hthread = i;
		tmp->trapmask = FASTINT_TRAPMASK;
	}
	H2K_intconfig_l2_init();
}

