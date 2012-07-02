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

H2K_translation_t H2K_vm_translate(u32_t addr, H2K_vmblock_t *vmblock) {

	u32_t mask;
	H2K_translation_t ret;

	ret.raw = 0;

	if (vmblock->pmap_type == H2K_ASID_TRANS_TYPE_OFFSET) { // check fence params
		ret.addr = addr + (vmblock->phys_offset.pages << PAGE_BITS);

		mask = 0xffffffff << (PAGE_BITS + (vmblock->phys_offset.size * 2));
		if (vmblock->fence_lo <= ((ret.addr & mask) >> PAGE_BITS)
				&& ((ret.addr & mask) >> PAGE_BITS) <= vmblock->fence_hi) {

			ret.size = vmblock->phys_offset.size;
			ret.cccc = vmblock->phys_offset.cccc;
			ret.xwru = vmblock->phys_offset.xwru;
			ret.valid = 1;
		}
		return ret;
	}

	return H2K_translate(addr, vmblock);
}
