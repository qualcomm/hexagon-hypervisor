/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <dosched.h>
#include <readylist.h>
#include <runlist.h>
#include <context.h>
#include <setjmp.h>
#include <lowprio.h>
#include <stdio.h>
#include <stdlib.h>
#include <checker_runlist.h>
#include <checker_ready.h>
#include <globals.h>

H2K_thread_context l,m,h;
H2K_thread_context l2,m2,h2;

void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static u32_t TB_saw_switch = 0;

static jmp_buf env;

H2K_thread_context *TB_me;
H2K_thread_context *TB_to;

/*
 * Define H2K_switch so that we don't actually switch
 */
void H2K_switch(H2K_thread_context *from, H2K_thread_context *to)
{
	TB_saw_switch++;
	if (from != TB_me) FAIL("switch not from TB_me");
	TB_to = to;
	longjmp(env,1);
}

typedef enum {
	SETUP,
	CHECK
} phase_t;

typedef void (* testsetup_t)(phase_t phase);

/* Reset state */
void TB_reset()
{
	TB_saw_switch = 0;
	H2K_runlist_init();
	H2K_readylist_init();
	H2K_lowprio_init();
}

void TB_setup_common()
{
	/* lock kernel */
}

/* Check ready and run lists for sanity */
void TB_check_common()
{
	if (TB_saw_switch == 0) FAIL("No switch?");
	checker_ready();
	checker_runlist();
}

/* Do call to dosched, longjmp can bring us back */
void TB_do_call()
{
	if (setjmp(env) == 0) {
		H2K_dosched(TB_me, 0);
	} else {
		/* longjump return */
	}
}

/* 
 * OK, each test below has two phases.  The first phase sets up the test, and
 * the second phase checks that the behavior was expected 
 */

void TB_fiddle_prio_low_to_low(phase_t phase)
{
	if (phase == SETUP) {
		/* Setup for call */
		if (H2K_gp->wait_mask == 0) {
			H2K_gp->priomask = H2K_gp->wait_mask;
		} else {
			H2K_gp->priomask = 0x1;
		}
		H2K_runlist_push(&h);
		H2K_ready_append(&m);
		TB_me = &l;
	} else {
		/* Check expected values */
		if (TB_to != &m) FAIL("Unexpected thread scheduled");
		if (H2K_gp->wait_mask == 0) {
			if ((H2K_gp->priomask & 1) == 0) FAIL("low_to_low did not switch to lowprio");
		}
	}
}

void TB_fiddle_prio_low_to_high(phase_t phase)
{
	if (phase == SETUP) {
		/* Setup for call */
		if (H2K_gp->wait_mask == 0) {
			H2K_gp->priomask = H2K_gp->wait_mask;
		} else {
			H2K_gp->priomask = 0x1;
		}
		H2K_runlist_push(&m);
		H2K_ready_append(&h);
		TB_me = &l;
	} else {
		/* Check expected values */
		if (TB_to != &h) FAIL("Unexpected thread scheduled");
		if ((H2K_gp->priomask & 1) == 1) FAIL("low_to_high did not switch from lowprio");
	}
}

void TB_fiddle_prio_high_to_low(phase_t phase)
{
	if (phase == SETUP) {
		/* Setup for call */
		if (H2K_gp->wait_mask == 0) {
			H2K_gp->priomask = H2K_gp->wait_mask;
		} else {
			H2K_gp->priomask = 0x2;
		}
		H2K_runlist_push(&m);
		H2K_ready_append(&l);
		TB_me = &h;
	} else {
		/* Check expected values */
		if (TB_to != &l) FAIL("Unexpected thread scheduled");
		if (H2K_gp->wait_mask == 0) {
			if ((H2K_gp->priomask & 1) == 0) FAIL("high_to_low did not switch to lowprio");
		}
	}
}

void TB_fiddle_prio_high_to_high(phase_t phase)
{
	if (phase == SETUP) {
		/* Setup for call */
		if (H2K_gp->wait_mask == 0) {
			H2K_gp->priomask = H2K_gp->wait_mask;
		} else {
			H2K_gp->priomask = 0x2;
		}
		H2K_runlist_push(&l);
		H2K_ready_append(&m);
		H2K_ready_append(&h);
		TB_me = &h;
	} else {
		if (TB_to != &h) FAIL("Unexpected thread scheduled");
		if ((H2K_gp->priomask & 1) != 0) FAIL( " Unexpected switch to lowprio");
	}
}

void TB_fiddle_prio_high_to_wait(phase_t phase)
{
	if (phase == SETUP) {
		/* Setup for call */
		if (H2K_gp->wait_mask == 0) {
			H2K_gp->priomask = H2K_gp->wait_mask;
		} else {
			H2K_gp->priomask = 0x2;
		}
		TB_me = &h;
	} else {
		/* Check expected values */
		if (TB_to != NULL) FAIL( " Unexpected thread scheduled");
	}
}

void TB_fiddle_prio_low_to_wait(phase_t phase)
{
	if (phase == SETUP) {
		/* Setup for call */
		if (H2K_gp->wait_mask == 0) {
			H2K_gp->priomask = H2K_gp->wait_mask;
		} else {
			H2K_gp->priomask = 0x1;
		}
		TB_me = &l;
	} else {
		/* Check expected values */
		if (TB_to != NULL) FAIL( " Unexpected thread scheduled");
	}
}

void TB_fiddle_prio_wait_to_high(phase_t phase)
{
	if (phase == SETUP) {
		/* Setup for call */
		TB_me = NULL;
		H2K_runlist_push(&l);
		H2K_ready_append(&h);
		if (H2K_gp->wait_mask == 0) {
			H2K_gp->priomask = 0;
			H2K_lowprio_notify();
		} else {
			H2K_gp->priomask = H2K_gp->wait_mask;
		}
	} else {
		/* Check expected values */
		if (H2K_gp->ready_valids != 0) FAIL("didn't find ready thread");
		if ((H2K_gp->runlist_valids & (1<<h.prio)) == 0) FAIL("Didn't insert h thread");
		if (H2K_gp->wait_mask == 0) {
			if (H2K_gp->priomask == 0x1) FAIL("set myself to lowprio?");
		}
	}
}

void TB_fiddle_prio_wait_to_low(phase_t phase)
{
	/* highprio thread already running, we come out of sleep */
	if (phase == SETUP) {
		/* Setup for call */
		TB_me = NULL;
		H2K_runlist_push(&h);
		H2K_ready_append(&l);
		if (H2K_gp->wait_mask == 0) {
			H2K_gp->priomask = 0;
			H2K_lowprio_notify();
		} else {
			H2K_gp->priomask = H2K_gp->wait_mask;
		}
	} else {
		/* Check expected values */
		if (H2K_gp->ready_valids != 0) FAIL("didn't find ready thread");
		if ((H2K_gp->runlist_valids & (1<<l.prio)) == 0) FAIL("Didn't insert l thread");
		if (H2K_gp->wait_mask == 0) {
			if (H2K_gp->priomask != 0x1) FAIL("Didn't set myself to lowprio");
		}
	}
}

void TB_fiddle_prio_wait_to_wait(phase_t phase)
{
	if (phase == SETUP) {
		/* Setup for call */
		TB_me = NULL;
	} else {
		/* Check expected values */
		if (TB_to != NULL) FAIL("Unexpected thread scheduled");
	}
}

/* Array of tests to do */
testsetup_t TB_fiddle_prio[] = {
	TB_fiddle_prio_low_to_low,
	TB_fiddle_prio_low_to_high,
	TB_fiddle_prio_high_to_low,
	TB_fiddle_prio_high_to_high,
	TB_fiddle_prio_high_to_wait,
	TB_fiddle_prio_low_to_wait,
	TB_fiddle_prio_wait_to_high,
	TB_fiddle_prio_wait_to_low,
	TB_fiddle_prio_wait_to_wait,
	NULL
};

/* Set up or check wait mask */
void TB_fiddle_wait_mask_zero(phase_t phase)
{
	if (phase == SETUP) {
		/* Setup for call */
		H2K_gp->wait_mask = 0;
	} else {
		/* Check expected values */
		if (H2K_gp->wait_mask != 0x0) FAIL("wait mask bit set");
	}
}

void TB_fiddle_wait_mask_nonzero(phase_t phase)
{
	if (phase == SETUP) {
		/* Setup for call */
		H2K_gp->wait_mask = 0x4;	/* Mark another thread as asleep */
	} else {
		/* Check expected values */
		if (H2K_gp->wait_mask != 0x4) FAIL("wait mask bit cleared");
	}
}

/* Array of tests to do */
testsetup_t TB_fiddle_wait_mask[] = {
	TB_fiddle_wait_mask_zero,
	TB_fiddle_wait_mask_nonzero,
	NULL
};

H2K_kg_t H2K_kg;

int main()
{
	int i,j;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));

	/* Set up some threads */
	l.prio = 20;
	l.hthread = 1;
	m.prio = 10;
	m.hthread = 1;
	h.prio =  2;
	h.hthread = 1;

	l2.prio = 20;
	l2.hthread = 1;
	m2.prio = 10;
	m2.hthread = 1;
	h2.prio =  2;
	h2.hthread = 1;

	/* For each wait mask type... */
	for (i = 0; TB_fiddle_wait_mask[i] != NULL; i++) {
		/* For each change type... */
		for (j = 0; TB_fiddle_prio[j] != NULL; j++) {
			/* Reset the TB */
			TB_reset();
			/* setup the wait mask */
			TB_fiddle_wait_mask[i](SETUP);
			/* setup scheduler state */
			TB_fiddle_prio[j](SETUP);
			/* Setup other stuff */
			TB_setup_common();
			/* Call the scheduler */
			TB_do_call();
			/* Check normal things */
			TB_check_common();
			/* Check the wait mask behavior */
			TB_fiddle_wait_mask[i](CHECK);
			/* Check the scheduler behavior */
			TB_fiddle_prio[j](CHECK);
		}
	}
	puts("TEST PASSED\n");
	return 0;
}

