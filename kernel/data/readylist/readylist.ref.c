/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <readylist.h>

void H2K_readylist_init(void) 
{
	/* EJP: FIXME: global_init already zeroes. */
	u32_t i;
	for (i = 0; i < MAX_PRIOS; i++) {
		H2K_gp->ready[i] = NULL;
	}
	for (i = 0; i < MAX_PRIOS/64; i++) {
		H2K_gp->ready_valids[i] = 0;
	}
}
