/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <futex.h>
#include <hw.h>
#include <globals.h>
#include <ring.h>

/* note: futex_cancel must be called with bkl held */
void H2K_futex_cancel(H2K_thread_context *dest)
{
	u32_t hashval;
	hashval = FUTEX_HASHVAL(dest->futex_ptr);
	H2K_ring_remove(&H2K_gp->futexhash[hashval],dest);
	dest->r00 = -1;
}

void H2K_futex_init()
{
	int i;
	for (i = 0; i < FUTEX_HASHSIZE; i++) {
		H2K_gp->futexhash[i] = NULL;
	}
}

