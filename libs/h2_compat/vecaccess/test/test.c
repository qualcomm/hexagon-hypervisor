/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>

void FAIL(const char *str)
{
	puts("FAIL");
	h2_printf(str);
	exit(1);
}

int main() {
	h2_coproc_init();

	printf("HMX 0 %d\n", h2_coproc_count(CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HMX_CONTEXTS, 0x1));
	printf("HMX 1 %d\n", h2_coproc_count(CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HMX_CONTEXTS, 0x2));
	printf("HMX all %d\n", h2_coproc_count(CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HMX_CONTEXTS, -1));

	printf("TEST PASSED\n");
	return 0;
}

