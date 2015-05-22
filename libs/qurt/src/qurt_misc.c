/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>

unsigned int QURT_MAX_HTHREADS __attribute__((section(".sdata"))) = 6;
unsigned int QURTK_MAX_HTHREADS __attribute__((section(".sdata"))) = 6;

extern char start;
extern char end;
unsigned int image_vstart = (unsigned int)(&start);
unsigned int image_pstart = (unsigned int)(&start);
unsigned int image_vend = (unsigned int)(&end);

int qurt_sysenv_get_app_heap(qurt_sysenv_app_heap_t *aheap )
{
	/* EJP: FIXME: put real values here from config? */
	aheap->heap_base = ((unsigned int)0x8C000000);
	aheap->heap_limit = aheap->heap_base + (0x800000);
	return QURT_EOK;
}

/* If you want to cheat, I'll cheat too */

void qurt_bad_symbol() {
	UNSUPPORTED;
}

