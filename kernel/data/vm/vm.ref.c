/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <vm.h>

void H2K_vmblock_clear(H2K_vmblock_t *vmblock) {
	u32_t i;
	u64_t *x = (u64_t *)vmblock;
	for (i = 0; i < (sizeof(*vmblock)/sizeof(*x)); i++) {
		x[i] = 0;
	}
}
