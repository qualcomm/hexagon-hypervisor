/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <linear.h>
#include <q6protos.h>
#include <tlbfmt.h>

H2K_mem_tlbfmt_t H2K_mem_translate_linear(u32_t badva, H2K_thread_context *me)
{
	H2K_linear_fmt_t *list, tmp;
	u32_t tvpn;
	u32_t mask;
	u32_t badvpn = badva >> 12; /* PAGEBITS or something */
	H2K_mem_tlbfmt_t ret;
	list = (H2K_linear_fmt_t *)me->gptb;
	tmp = *list;
	while (tmp.raw) {
		if (tmp.chain) {
			/* Pointer to next set of translations */
			list = (H2K_linear_fmt_t *)tmp.low;
			tmp = *list;
			continue;
		}
		tvpn = tmp.vpn;
		mask = 0xffffffff << (tmp.size*2);
		if ((tvpn & mask) == (badvpn & mask)) {
			/* match */
			return H2K_mem_tlbfmt_from_linear(tmp,me->ssr_asid);
		}
		tmp = *(++list);
	}
	ret.raw = 0;
	return ret;
}

