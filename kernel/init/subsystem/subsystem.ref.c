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
/* Power up HVX */
void H2K_hvx_init(u32_t devpage_offset) {

#ifdef HAVE_EXTENSIONS

	if (!H2K_gp->info_boot_flags.boot_have_hvx) {
		return;
	}

	H2K_gp->hvx_clock = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_CLK_CTL);
	H2K_gp->hvx_reset = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_RESET);
	H2K_gp->hvx_power = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_PWR_CTL); 
#if ARCHV >= 65
	H2K_gp->hvx_bhs_status = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_BHS_STATUS); 
	H2K_gp->hvx_bhs_cfg = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_BHS_CFG); 
	H2K_gp->hvx_bhs_cmd = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_BHS_CMD); 
	H2K_gp->hvx_cpmem_cfg = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CPMEM_CFG); 
	H2K_gp->hvx_cpmem_cmd = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CPMEM_CMD); 
	H2K_gp->hvx_cpmem_status = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CPMEM_STATUS); 
#endif
	H2K_gp->hvx_state = H2K_HVX_STATE_OFF;

#endif
}

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
