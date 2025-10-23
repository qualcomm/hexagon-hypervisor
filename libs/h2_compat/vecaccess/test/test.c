/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>
#include <h2_vecaccess.h>

void FAIL(const char *str)
{
	puts("FAIL");
	h2_printf(str);
	exit(1);
}

int main() {

	h2_vecaccess_state_t vacc;
	h2_vecaccess_ret_t ret;
	unsigned int x;

	h2_vecaccess_unit_init(&vacc, H2_VECACCESS_HVX_128, CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HVX_CONTEXTS, 0x1);
	ret = h2_vecaccess_acquire(&vacc);
	printf("ret.idx %d\n", ret.idx);
	ret = h2_vecaccess_acquire(&vacc);
	printf("ret.idx %d\n", ret.idx);
	asm volatile ("%0 = ssr \n" : "=r"(x));  // crash

	printf("HVX 0 %d\n", h2_coproc_count(CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HVX_CONTEXTS, 0x1));
	printf("HVX 1 %d\n", h2_coproc_count(CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HVX_CONTEXTS, 0x2));
	printf("HVX all %d\n", h2_coproc_count(CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HVX_CONTEXTS, -1));
	printf("HVX none %d\n", h2_coproc_count(CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HVX_CONTEXTS, 0x0));

	printf("HMX 0 %d\n", h2_coproc_count(CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HMX_CONTEXTS, 0x1));
	printf("HMX 1 %d\n", h2_coproc_count(CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HMX_CONTEXTS, 0x2));
	printf("HMX all %d\n", h2_coproc_count(CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HMX_CONTEXTS, -1));

	

	printf("TEST PASSED\n");
	return 0;
}

