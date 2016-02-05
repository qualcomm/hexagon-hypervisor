/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <linear.h>
#include <hexagon_protos.h>
#include <tlbfmt.h>
#include <max.h>
#include <asid.h>
#include <physread.h>
#include <globals.h>

static inline H2K_translation_t H2K_linear_translate_update(H2K_translation_t in, H2K_linear_fmt_t entry)
{
	in.size = min(in.size,2*entry.size);
	in.pn = entry.ppn;
	if (in.cccc > 0xF) in.cccc = entry.cccc;
	in.xwru &= entry.xwru;
	return in;
}

H2K_translation_t H2K_linear_translate(H2K_translation_t in, H2K_asid_entry_t info)
{
	u32_t list = info.ptb;	/* FIXME: use extra info to allow extra bits? */
	u32_t list_ppn,list_gpn;
	u32_t last_gpn = ~0UL;
	u32_t badvpn;
	u32_t evpn;
	u32_t mask;
	pa_t list_pa;
	H2K_linear_fmt_t entry;
	H2K_vmblock_t *vmblock = H2K_gp->vmblocks[info.fields.vmid];
	H2K_translation_t tmp = H2K_translate_default(0);
	badvpn = in.pn;
	do {
		list_gpn = list >> PAGE_BITS;
		if (list_gpn != last_gpn) {
			/* Walked over a page boundary, retranslate entries */
			last_gpn = list_gpn;
			if (vmblock->guestmap.raw) {
				tmp.pn = list_gpn;
				tmp = H2K_translate(tmp,vmblock->guestmap);
				list_ppn = tmp.pn;
				if (tmp.raw == 0) break;
			} else {
				list_ppn = list_gpn;
			}
			list_pa = list_ppn;
			list_pa <<= PAGE_BITS;
			list_pa |= (list & ((1<<PAGE_BITS)-1));
		}
		entry.raw = H2K_mem_physread_dword(list_pa);
		if (entry.raw == 0) break;
		if (entry.chain) {
			list = entry.low & -8;
			list_pa &= (list_pa & -(1<<PAGE_BITS));
			list_pa |= list & ((1<<PAGE_BITS)-1);
			continue;
		}
		evpn = entry.vpn;
		mask = ~0UL << entry.size*2;
		if ((evpn & mask) == (badvpn & mask)) {
			/* REFINE PPN, PERMS */
			in = H2K_linear_translate_update(in,entry);
			if (vmblock->guestmap.raw) return H2K_translate(in,vmblock->guestmap);
			return in;
		}
		list += 8;
		list_pa += 8;
	} while (1);
	in.raw = 0;
	return in;
}

