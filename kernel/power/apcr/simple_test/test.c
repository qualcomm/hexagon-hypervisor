/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef RTL
#define ITERS 1
#ifndef INTERRUPT_NUM
#define INTERRUPT_NUM 3
#endif
#else
#define ITERS 2
#define HAVE_TIMER
#endif

#ifdef HAVE_TIMER
#define SLEEP (1000*1000*5)
#endif

#define PASSFAIL_VA (H2K_GUEST_START + 0x01000000)

int main()
{
	int i;
	unsigned int *sigil = (void *)(PASSFAIL_VA);

	for (i = 0; i < ITERS; i++) {
#ifdef HAVE_TIMER
		h2_nanosleep(SLEEP);
#else
		h2_intwait(INTERRUPT_NUM);
#endif
#ifndef RTL
		*sigil = i + 1;
#endif
	}
	*sigil = 0xe0f0beef;
	h2_dccleana(sigil);
#ifndef RTL
	printf("TEST PASSED\n");
	exit(0);
#endif
	h2_thread_stop_trap(0);
	return 0;
}