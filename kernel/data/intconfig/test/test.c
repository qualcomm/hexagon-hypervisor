/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <asm_offsets.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <intconfig.h>
#include <setjmp.h>
#include <max.h>
#include <resched.h>
#include <globals.h>
#include <vmipi.h>
#include <timer.h>

#define BAD ((void *)(0xdeadbeef))

#define TEST_FASTINT_TRAPMASK 0x9

H2K_thread_context a;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

int TH_handler(u32_t x)
{
	return 0;
}

void H2K_fastint();

u32_t  TH_l2int_cfg[1024] __attribute__((aligned(4096))) ;

int main() 
{
	int i;
	u32_t timer_int;
	u32_t rev;

	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	__asm__ __volatile("%0 = rev " : "=r"(rev));

	rev &= 0xff;
	switch (rev) {
	case CORE_V4:
		timer_int = TIMER_INT_CORE_V4;
		break;
	case CORE_V5:
		timer_int = TIMER_INT_CORE_V5;
		break;
	case CORE_V61:
		timer_int = TIMER_INT_CORE_V61;
		break;
	default:
		timer_int = TIMER_INT_CORE_V60;
		break;
	}

#if ARCHV >= 4
	H2K_gp->l2_int_base = TH_l2int_cfg;
	H2K_gp->l2_ack_base = TH_l2int_cfg + 0x80;
#endif
	H2K_gp->timer_intnum = timer_int;

	a.gpugp = 0xF000000012345678ULL;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->inthandlers[i].handler = BAD;
		H2K_gp->inthandlers[i].param = BAD;
	}
	for (i = 0; i < H2K_gp->hthreads; i++) {
		H2K_fastint_contexts[i].context.r0100 = 0xdeadbeefcafebabeULL;
		H2K_fastint_contexts[i].context.hthread = 0xf;
		H2K_fastint_contexts[i].context.trapmask = 0xdead;
	}
#if ARCHV >= 5
	H2K_intconfig_init(QDSP6SS_PRIV_BASE_LPASS);
#else
	H2K_intconfig_init(0);
#endif
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		if (i == RESCHED_INT) {
			if (H2K_gp->inthandlers[i].handler != H2K_resched)
				FAIL("wrong resched handler");
		} else if (i == CLUSTER_RESCHED_INT) {
			if (H2K_gp->inthandlers[i].handler != H2K_resched_cluster)
				FAIL("wrong cluster resched handler");
		} else if (i == VM_IPI_INT) {
			if (H2K_gp->inthandlers[i].handler != H2K_vm_ipi_do)
				FAIL("wrong ipi handler");
		} else if (i == timer_int) {
			if (H2K_gp->inthandlers[i].handler != H2K_timer_int)
				FAIL("wrong timer int handler");
		} else {
			if (H2K_gp->inthandlers[i].handler != NULL) {
				FAIL("uninitialized handler");
			}
			if (H2K_gp->inthandlers[i].param != NULL) {
				FAIL("uninitialized param");
			}
		}
	}
	for (i = 0; i < H2K_gp->hthreads; i++) {
		if (H2K_fastint_contexts[i].context.r0100) FAIL("Uninitialized fastint context");
		if (H2K_fastint_contexts[i].context.hthread != i) FAIL("Uninitialized fastint context");
		if (H2K_fastint_contexts[i].context.trapmask != TEST_FASTINT_TRAPMASK) FAIL("bad trapmask");
	}
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		if (i == RESCHED_INT) continue;
		H2K_register_fastint(i,TH_handler,&a);
		if (H2K_gp->inthandlers[i].handler != H2K_fastint) FAIL("fastint handler not set");
		if (H2K_gp->inthandlers[i].param != TH_handler) FAIL("wrong handler func");
		if (H2K_gp->fastint_gp != 0xF0000000U) FAIL("fastint gp not set");
	}
	/* Lets try deregistering */
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		if (i == RESCHED_INT) continue;
		H2K_register_fastint(i,NULL,&a);
		if (H2K_gp->inthandlers[i].handler) FAIL("fastint handler not cleared");
		if (H2K_gp->inthandlers[i].param) FAIL("handler func not cleared");
	}
	/* Lets deregister an already deregistered fastint */
	H2K_register_fastint(0,NULL,&a);
	if (H2K_gp->inthandlers[0].handler) FAIL("fastint handler did not stay cleared");
	if (H2K_gp->inthandlers[0].param) FAIL("handler func did not stay cleared");
	puts("TEST PASSED\n");
	return 0;
}

