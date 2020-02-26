/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <offset.h>
#include <vmblock.h>
#include <h2_common_vmblock.h>
#include <globals.h>

H2K_translation_t H2K_offset_translate(H2K_translation_t in, H2K_asid_entry_t info)
{
	H2K_vmblock_t *vmblock;
	H2K_offset_t offset;
	H2K_translation_t out;

	offset.raw = info.ptb;
	in.pn += offset.pages;
	if (in.size > offset.size) in.size = offset.size;
	in.xwru &= offset.xwru;
	if (in.cccc > 0xF) in.cccc = offset.cccc;

	vmblock = H2K_gp->vmblocks[info.fields.vmid];
	if (vmblock->guestmap.raw) {
		out = H2K_translate(in,vmblock->guestmap);
	}	else {
		out = in;
	}
	if ((out.pn < vmblock->fence_lo) || (out.pn > ((vmblock->fence_hi) + (0x1 << (out.size * 2))))) {
		out.raw = 0;
	}
	return out;
}

