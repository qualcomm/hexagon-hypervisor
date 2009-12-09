/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <hw.h>
#include <checker_help.h>
#include <checker_ring.h>

void checker_ring(H2K_ringnode_t *x)
{
	u32_t i = 0;
	H2K_ringnode_t *start, *tmp;
	start = tmp = x;
	do {
		if (++i == 1000) {
			FAIL("Could not find ring start");
		}
		if ((tmp->next == NULL) || (tmp->prev == NULL)) {
			FAIL("NULL pointer in ring");
		}
		if (tmp->next->prev != tmp) {
			FAIL("Next->prev invalid");
		}
		if (tmp->prev->next != tmp) {
			FAIL("prev->next invalid");
		}
		tmp = tmp->next;
	} while (tmp != start);
}

