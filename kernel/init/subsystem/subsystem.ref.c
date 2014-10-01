/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <globals.h>
#include <hvx.h>

/* Power up HVX */
void H2K_hvx_init(u32_t devpage_offset) {

#ifdef HAVE_EXTENSIONS

	if (!H2K_gp->info_boot_flags.boot_have_hvx) {
		return;
	}

	H2K_gp->hvx_clock = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_CLK_CTL);
	H2K_gp->hvx_reset = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_RESET);
	H2K_gp->hvx_power = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_PWR_CTL);
	H2K_gp->hvx_state = H2K_HVX_STATE_OFF;

#endif
}
