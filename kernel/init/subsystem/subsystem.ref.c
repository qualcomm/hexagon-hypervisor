/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <globals.h>
#include <hvx.h>
#include <hmx.h>
#include <hlx.h>
#include <tmpmap.h>
#include <h2_common_pmap.h>

/* Power up HMX */
void H2K_hmx_init(u32_t devpage_offset) {

#ifdef HAVE_EXTENSIONS

	if (!H2K_gp->info_boot_flags.boot_have_hmx) {
		return;
	}

#if ARCHV >= 68
	H2K_gp->hmx_rsc_seq_busy_drv0 = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_RSCC_RSC_SEQ_BUSY_DRV0);
	H2K_gp->hmx_rsc_seq_override_trigger_drv0 = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_DRV0);
	H2K_gp->hmx_rsc_seq_override_trigger_start_addr_drv0 = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_START_ADDR_DRV0);
	H2K_gp->hmx_state = H2K_HMX_STATE_OFF;
#endif

#endif

}

//TODO: HLX, what has to be done here?
void H2K_hlx_init(u32_t devpage_offset) {

	if (!H2K_gp->info_boot_flags.boot_have_hlx) {
		return;
	}

	H2K_gp->hlx_state = H2K_HLX_STATE_OFF;
}
