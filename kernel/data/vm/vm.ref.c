/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <vm.h>
#include <translate.h>

void H2K_vmblock_clear(H2K_vmblock_t *vmblock) {
	u32_t i;
	u64_t *x = (u64_t *)vmblock;
	for (i = 0; i < (sizeof(*vmblock)/sizeof(*x)); i++) {
		x[i] = 0;
	}
}

s32_t H2K_vm_translate(u32_t addr, H2K_vmblock_t *vmblock, u32_t *result) {

	u32_t ret;

	if (vmblock->pmap_type == H2K_ASID_TRANS_TYPE_OFFSET) { // check fence params
		ret = addr + (vmblock->phys_offset.pages << PAGE_BITS);

		if (vmblock->fence_lo <= ret && ret <= vmblock->fence_hi) {
			*result = ret;
			return 0;
		}
		else {
			return -1;
		}
	}

	return H2K_translate(addr, vmblock->pmap, vmblock->pmap_type, result);
}
