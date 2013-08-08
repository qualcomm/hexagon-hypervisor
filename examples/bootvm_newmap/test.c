/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <h2_thread.h>
#include <h2_vmtraps.h>
#include <h2_common_linear.h>
#include <h2_common_pmap.h>

/* Assuming H2 default 4M page size here, and that everything fits in one page.
	 If we want to use small pages then the translations will have to be malloced
	 and set up at runtime */
H2K_linear_fmt_t trans[] = {
	MEMORY_MAP(	(((H2K_BOOTVM_OFFSET >> 22) + 0) << 10),	URWX,		L1WT_L2C,	SIZE_4M,	(((H2K_BOOTVM_OFFSET >> 22) + 0) << 10))
	{ .raw = 0 },
};

volatile int flag = 0;

void check () {
	
	flag = 1;
}

int main() 
{

	/* Set up translations with write-through L1 */
	h2_vmtrap_newmap(trans, H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_TRUE);

	check();

	h2_thread_stop(0);
	return 0;
}

