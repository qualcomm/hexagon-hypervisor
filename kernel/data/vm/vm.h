/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VM_H
#define H2K_VM_H 1

#include<asid_types.h>
#include <vmblock.h>
#include <translate.h>

void H2K_vmblock_clear(H2K_vmblock_t *vmblock) IN_SECTION(".text.data.vm");
H2K_translation_t H2K_vm_translate(u32_t addr, H2K_vmblock_t *vmblock) IN_SECTION(".text.data.vm");

#if __QDSP6_ARCH__ <= 3

static inline H2K_mem_tlbfmt_t H2K_vm_get_offset(u32_t addr, H2K_thread_context *me) {

	H2K_mem_tlbfmt_t ret;
	u32_t mask;
	H2K_vmblock_t *vmblock = me->vmblock;
	H2K_offset_t offset = vmblock->phys_offset;
	u32_t xwru;

	ret.raw = 0;
	if (vmblock->pmap_type != H2K_ASID_TRANS_TYPE_OFFSET) { // woops
		return ret;
	}

	mask = (0xffffffff) << (offset.size * 2);
	xwru = offset.xwru;

	ret.ccc = offset.cccc;
	ret.xwr = xwru >> 1;
	ret.guestonly = ~(xwru & 1);
	ret.asid = me->ssr_asid;

	ret.vpn = addr & mask;
	ret.ppn = ret.vpn + (offset.pages << PAGE_BITS);
	ret.size = offset.size;
	ret.valid = 1;

	return ret;
}

#else

static inline H2K_mem_tlbfmt_t H2K_vm_get_offset(u32_t addr, H2K_thread_context *me) {

	H2K_mem_tlbfmt_t ret;
	u32_t mask, vpn, ppn;
	H2K_vmblock_t *vmblock = me->vmblock;
	H2K_offset_t offset = vmblock->phys_offset;

	ret.raw = 0;
	if (vmblock->pmap_type != H2K_ASID_TRANS_TYPE_OFFSET) { // woops
		return ret;
	}

	mask = (0xffffffff) << (offset.size * 2);

	vpn = (addr >> PAGE_BITS) & mask;
	ppn = vpn + offset.pages;
	if (me->vmblock->fence_lo <= ppn && ppn <= me->vmblock->fence_hi) {
		ret.cccc = offset.cccc;
		ret.xwru = offset.xwru;
		ret.asid = me->ssr_asid;

		ret.vpn = vpn;
		ret.ppd = (ppn << 1) | (1 << offset.size);
		ret.valid = 1;
	}
	return ret;
}

#endif

#endif

