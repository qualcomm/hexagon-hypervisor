/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <offset.h>
#include <vmblock.h>
#include <h2_common_vmblock.h>
#include <globals.h>
#include <log.h>

H2K_translation_t H2K_offset_translate(H2K_translation_t in, H2K_asid_entry_t info)
{
	H2K_vmblock_t *vmblock;
	H2K_offset_t offset;
	H2K_translation_t out;
	u32_t orig_in = in.pn;

	offset.raw = info.ptb;
	in.pn += offset.pages;
	if (in.size > offset.size) in.size = offset.size;
	in.xwru &= offset.xwru;
	if (in.weak_ccc) in.cccc = offset.cccc;
	in.weak_ccc = offset.weak_ccc;
	// in.shared unchanged, since this is meaningless for offset translations

	vmblock = H2K_gp->vmblocks[info.fields.vmid]; // parent VM has next-level translation
	if (!in.shared && vmblock->guestmap.raw) {
		out = H2K_translate(in,vmblock->guestmap);
	}	else { // lowest level
		/* Don't apply offset or check fences when mapping special address spaces */
		/* FIXME: We need to allocate ranges to guests and check access here */
		if ((H2K_gp->tcm_base <= orig_in && orig_in < H2K_gp->tcm_base + H2K_gp->tcm_size) ||
				(H2K_gp->vtcm_base <= orig_in &&  orig_in < H2K_gp->vtcm_base + H2K_gp->vtcm_size)) {
			in.pn = orig_in;
			out = in;
		} else {
			out = in;
			if ((out.pn < vmblock->fence_lo) || (out.pn >= ((vmblock->fence_hi) + (0x1 << (out.size * 2))))) {
				out.raw = 0;
			}
		}
	}
	return out;
}

