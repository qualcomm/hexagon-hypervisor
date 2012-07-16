/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <linear.h>
#include <q6protos.h>
#include <tlbfmt.h>
#include <max.h>
#include <asid.h>
#include <physread.h>
#include <globals.h>

#if __QDSP6_ARCH__ <= 3

static inline H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_linear(H2K_linear_fmt_t linear, u32_t asid)
{
	H2K_mem_tlbfmt_t ret;
	u32_t xwru;
	ret.raw = 0;
	ret.ppn = linear.ppn;
	ret.ccc = linear.cccc;
	xwru = linear.xwru;
	ret.vpn = linear.vpn;
	ret.size = linear.size;
	ret.xwr = (xwru >> 1);
	ret.guestonly = ~(xwru & 1);
	ret.asid = asid;
	ret.valid = 1;
	return ret;
}

#else

static inline H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_linear(H2K_linear_fmt_t linear, u32_t asid)
{
	H2K_mem_tlbfmt_t ret;
	u32_t size = linear.size;
	u32_t mask = (0xFFFFFFFF) << (size*2);
	ret.raw = 0;
	ret.cccc = linear.cccc;
	ret.ppd = ((linear.ppn & mask) << 1) | (1<<size);
	ret.vpn = linear.vpn;
	ret.xwru = linear.xwru;
	ret.asid = asid;
	ret.valid = 1;
	return ret;
}

#endif

H2K_linear_fmt_t H2K_mem_lookup_linear(u32_t badva, u32_t list, H2K_vmblock_t *vmblock) {

	H2K_linear_fmt_t tmp;
	H2K_translation_t phys_translation;
	u32_t tvpn;
	u32_t mask;
	u32_t badvpn = badva >> PAGE_BITS; /* PAGEBITS or something */

	tmp = (H2K_linear_fmt_t)H2K_mem_physread_dword((u64_t)list);
	while (tmp.raw) {
		if (tmp.chain) {
			/* Pointer to next set of translations */
			list = tmp.low;
			if ((list & 0xff000000) == 0xff000000) { // FIXME: remove this once guests no longer in monitor space
				list -= H2K_gp->phys_offset;
			} else if (vmblock != NULL) {
				phys_translation =	H2K_vm_translate(list, vmblock);
				if (!phys_translation.valid) {
					tmp.raw = 0;
					return tmp;
				}
				list = phys_translation.addr;
			}
			tmp = (H2K_linear_fmt_t)H2K_mem_physread_dword((u64_t)list);
			continue;
		}
		tvpn = tmp.vpn;
		mask = 0xffffffff << (tmp.size*2);
		if ((tvpn & mask) == (badvpn & mask)) {
			/* match */
			return tmp;
		}
		list += sizeof(H2K_linear_fmt_t);
		/* FIXME: check that new list addr is in bounds of VM */
		tmp = (H2K_linear_fmt_t)H2K_mem_physread_dword((u64_t)list);
	}
	tmp.raw = 0;
	return tmp;
}

H2K_mem_tlbfmt_t H2K_mem_get_linear(u32_t badva, H2K_thread_context *me)
{
	H2K_linear_fmt_t tmp;
	u32_t list;
	H2K_mem_tlbfmt_t ret;

	ret.raw = 0;

	list = H2K_mem_asid_table[me->ssr_asid & (MAX_ASIDS-1)].ptb;
	if (list == 0) {
		return ret;
	}

	tmp = H2K_mem_lookup_linear(badva, list, me->vmblock);
	if (tmp.raw) {
		return H2K_mem_tlbfmt_from_linear(tmp,me->ssr_asid);
	}
	return ret;
}
