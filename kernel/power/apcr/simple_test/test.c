/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
/* #include <h2_cache.h> */
/* #include <h2_intwait.h> */
/* #include <h2_thread.h> */

#ifndef DEBUG
#define ITERS 1
#ifndef INTERRUPT_NUM
#define INTERRUPT_NUM 2
#endif
#else
#define ITERS 2
#ifndef INTERRUPT_NUM
#define INTERRUPT_NUM 5
#endif
#endif

#define PASSFAIL_VA 0x01000000

int main() 
{
	int i;
	unsigned int *sigil = (void *)(PASSFAIL_VA);

	for (i = 0; i < ITERS; i++) {
		h2_intwait(INTERRUPT_NUM);

#ifdef DEBUG
		*sigil = i + 1;
#endif
	}
	*sigil = 0xe0f0beef;
	h2_dccleana(sigil);
	h2_thread_stop_trap(0);
	return 0;
}

