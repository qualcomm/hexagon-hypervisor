/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <intconfig.h>
#include <resched.h>
#include <max.h>
#include <hexagon_protos.h>
#include <thread.h>
#include <globals.h>
#include <hw.h>
#include <vmipi.h>
#include <timer.h>
#include <idtype.h>
#include <intcontrol.h>
#include <passthru.h>

/* FIXME: get these from allocator? */
H2K_fastint_context H2K_fastint_contexts[MAX_HTHREADS];

#define FASTINT_TRAPMASK 0xb /* ANGEL | FUTEX_RESUME | THREAD_ID */

void H2K_fastint();

void H2K_register_fastint(u32_t whatint, int (*fastint_handler)(u32_t x), H2K_thread_context *me)
{
	H2K_inthandler_t tmp;
	int i;
	if (fastint_handler == NULL) { /* deregister */
		H2K_intcontrol_disable(whatint);
		H2K_gp->inthandlers[whatint].raw = 0;
	} else {
		/* write atomically */
		tmp.param = fastint_handler;
		tmp.handler = H2K_fastint;
		H2K_gp->inthandlers[whatint].raw = tmp.raw;
		H2K_intcontrol_enable(whatint);
		H2K_gp->fastint_gp = (u32_t)(me->gp);
		H2K_gp->fastint_ssr= (u32_t)(me->ssr & 0x00007F00); /* Set ASID field */
		H2K_gp->fastint_ssr |= 0x01800000; /* Turn on CE etc */
		for (i = 0; i < MAX_HTHREADS; i++) {
			H2K_fastint_contexts[i].context.vmblock = me->vmblock;
		}
	}
}

void H2K_register_passthru(u32_t phys_int, H2K_id_t id, u32_t virt_int) {

	H2K_inthandler_t tmp;

	if (id.vmidx == 0) { /* deregister */
		H2K_intcontrol_disable(phys_int);
		H2K_gp->inthandlers[phys_int].raw = 0;
	} else {
		/* write atomically */
		if (virt_int >= 0x1 << H2K_ID_VINT_BITS) { // too big
			id.reserved = 0; // look in vmblock; it will have been set up there
		} else {
			id.reserved = virt_int;
		}
		tmp.param = (void *)id.raw;
		tmp.handler = H2K_passthru;
		H2K_gp->inthandlers[phys_int].raw = tmp.raw;
		H2K_intcontrol_enable(phys_int);
	}
}

#if H2K_L2_CONTROL && ! defined NO_DEVICES
static void H2K_intconfig_l2_init(ssbase)
{

	int i;
	volatile unsigned int *intbase = H2K_gp->l2_int_base;

	/* EJP: FIXME: hard coded values from qurt_config for 8909.  Need to put in api, I guess? */
	const unsigned int l2_edgevals[] = {
		0xf8ffe3f3L,
		0x1f8fd1ffL,
		0xee3fd21fL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0x1ff87c0L,
		0x5fc33e00L,
		0xffffffb7L,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL,
		0xffffffffL};

	for (i = 0; i < ((MAX_INTERRUPTS-32)/32); i++) {
		intbase[(0x100/4) + i] = 0x0; 			/* DISABLED */
		intbase[(0x280/4) + i] = l2_edgevals[i];	/* EDGE/level TRIGGERED */
		intbase[(0x300/4) + i] = 0x0;			/* Rising Edge / Level High */
		intbase[(0x400/4) + i] = 0xFFFFFFFF;		/* Interrupt Clear */
	}
	ciad(0x80000000);				/* Enable L2 Interrupts */
}
#else
static inline void H2K_intconfig_l2_init(u32_t ssbase) {}
#endif

void H2K_intconfig_l2vic_crash()
{
	int i;
	volatile unsigned int *intbase = H2K_gp->l2_int_base;
	for (i = 0; i < MAX_L2_INTERRUPTS/32; i++) {
		H2K_gp->crash_l2vic_enabled[i] = intbase[(0x100/4) + i];
		H2K_gp->crash_l2vic_pending[i] = intbase[(0x500/4) + i];
	}
}

void H2K_intconfig_init(u32_t ssbase)
{
	int i;
	H2K_thread_context *tmp;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->inthandlers[i].raw = 0;
	}
	H2K_gp->inthandlers[RESCHED_INT].handler = H2K_resched;
	H2K_gp->inthandlers[VM_IPI_INT].handler = H2K_vm_ipi_do;
	H2K_gp->inthandlers[H2K_gp->timer_intnum].handler = H2K_timer_int;
	for (i = 0; i < H2K_gp->hthreads; i++) {
		tmp = &H2K_fastint_contexts[i].context;
		H2K_thread_context_clear(tmp);
		tmp->hthread = i;
		tmp->trapmask = FASTINT_TRAPMASK;
		tmp->id.raw = i;
	}
	H2K_intconfig_l2_init(ssbase);
}

