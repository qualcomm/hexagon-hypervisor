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
}

void H2K_fastint();

int main() 
{
	int i;
	u32_t oldmask;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	a.ugpgp = 0x12345678F0000000ULL;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->fastint_funcptrs[i] = BAD;
		H2K_gp->inthandlers[i] = BAD;
	}
	for (i = 0; i < MAX_HTHREADS; i++) {
		H2K_fastint_contexts[i].context.r0100 = 0xdeadbeefcafebabeULL;
		H2K_fastint_contexts[i].context.hthread = 0xf;
		H2K_fastint_contexts[i].context.trapmask = 0xdead;
	}
	H2K_intconfig_init();
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		if (i != RESCHED_INT) {
			if (H2K_gp->inthandlers[i] != NULL) FAIL("uninitialized handler");
		} else {
			if (H2K_gp->inthandlers[i] != H2K_resched) FAIL("wrong resched handler");
		}
		if (H2K_gp->fastint_funcptrs[i] != NULL) FAIL("uninitialized fastint ptr");
	}
	for (i = 0; i < MAX_HTHREADS; i++) {
		if (H2K_fastint_contexts[i].context.r0100) FAIL("Uninitialized fastint context");
		if (H2K_fastint_contexts[i].context.hthread != i) FAIL("Uninitialized fastint context");
		if (H2K_fastint_contexts[i].context.trapmask != TEST_FASTINT_TRAPMASK) FAIL("bad trapmask");
	}
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		if (i == RESCHED_INT) continue;
		H2K_register_fastint(i,TH_handler,&a);
		if (H2K_gp->inthandlers[i] != H2K_fastint) FAIL("fastint handler not set");
		if (H2K_gp->fastint_funcptrs[i] != TH_handler) FAIL("wrong handler func");
		if (H2K_gp->fastint_gp != 0xF0000000U) FAIL("fastint gp not set");
	}
	/* Lets try deregistering */
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		if (i == RESCHED_INT) continue;
		H2K_register_fastint(i,NULL,&a);
		if (H2K_gp->inthandlers[i]) FAIL("fastint handler not cleared");
		if (H2K_gp->fastint_funcptrs[i]) FAIL("handler func not cleared");
	}
	/* Lets deregister an already deregistered fastint */
	H2K_register_fastint(0,NULL,&a);
	if (H2K_gp->inthandlers[0]) FAIL("fastint handler did not stay cleared");
	if (H2K_gp->fastint_funcptrs[0]) FAIL("handler func did not stay cleared");
	puts("TEST PASSED\n");
	return 0;
}

