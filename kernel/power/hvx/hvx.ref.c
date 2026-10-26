/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <globals.h>
#include <hvx.h>
#include <atomic.h>
#include <hw.h>
#include <tmpmap.h>

void H2K_hvx_poweron(void) {
#ifndef NO_DEVICES
#ifdef HAVE_EXTENSIONS
	if (!H2K_gp->info_boot_flags.boot_have_hvx) {
		return;
	}

	volatile u32_t delay;
	u32_t va = H2K_tmpmap_add_and_lock((pa_t)H2K_gp->ssbase - H2K_gp->ss_pub_offset, DEVICE_TYPE, DEVICE_PAGE_SIZE);
	u32_t volatile *hvx_clock = (u32_t *)(va + QDSP6SS_CP_CLK_CTL);

	BKL_LOCK();
	if (H2K_gp->hvx_state == H2K_HVX_STATE_ON) {  // already on
		BKL_UNLOCK();
		return;
	}

#if ARCHV < 66
	u32_t volatile *hvx_reset  = (u32_t *)(va + QDSP6SS_CP_RESET);
	u32_t volatile *hvx_power = (u32_t *)(va + QDSP6SS_CP_PWR_CTL);

#if ARCHV < 65
	/* From HPG 4.7.8 */

	*hvx_clock = QDSP6SS_CP_CLK_CTL_DISABLE;
	*hvx_reset = QDSP6SS_CP_RESET_ASSERT;
	if (H2K_gp->arch >= CORE_V62) {
		*hvx_power = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V62;
	} else {
		*hvx_power = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V60;
	}
	delay = 500;
	while (--delay) asm volatile ("nop");  // spec says wait for 1(TBD?) microsecond
	if (H2K_gp->arch >= CORE_V62) {
		*hvx_power = QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V62;
	} else {
		*hvx_power = QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V60;
	}
	*hvx_reset = QDSP6SS_CP_RESET_DEASSERT;
	*hvx_clock = QDSP6SS_CP_CLK_CTL_ENABLE;

#else

	// v65
	u32_t val;

	u32_t volatile *hvx_bhs_status = (u32_t *)(va + QDSP6SS_CP_BHS_STATUS);
	u32_t volatile *hvx_bhs_cfg = (u32_t *)(va + QDSP6SS_CP_BHS_CFG);
	u32_t volatile *hvx_bhs_cmd = (u32_t *)(va + QDSP6SS_CP_BHS_CMD);
	u32_t volatile *hvx_cpmem_cfg = (u32_t *)(va + QDSP6SS_CPMEM_CFG);
	u32_t volatile *hvx_cpmem_cmd = (u32_t *)(va + QDSP6SS_CPMEM_CMD);
	u32_t volatile *hvx_cpmem_status = (u32_t *)(va + QDSP6SS_CPMEM_STATUS);

	/* From HPG 4.8.9 */

	*hvx_clock = QDSP6SS_CP_CLK_CTL_DISABLE;
	*hvx_reset = QDSP6SS_CP_RESET_ASSERT;
	delay = 1000;
	while ((delay--) &&
				 ((*hvx_bhs_status) & (0x1 << QDSP6SS_CP_BHS_STATUS_BHS_ON_BIT))) {
		asm volatile ("nop");
	}
	*hvx_bhs_cfg |= (0x1 << QDSP6SS_CP_BHS_CFG_BHS_ON_BIT);
	*hvx_bhs_cmd |= (0x1 << QDSP6SS_CP_BHS_CMD_UPDATE_BIT);
	delay = 1000;
	while ((delay--) &&
				 !((*hvx_bhs_status) & (0x1 << QDSP6SS_CP_BHS_STATUS_BHS_ON_BIT))) {
		asm volatile ("nop");
	}
	*hvx_power &= ~(((u32_t)(0x1)) << QDSP6SS_CP_PWR_CTL_CLAMP_IO_BIT);
	*hvx_power &= ~(((u32_t)(0x1)) << QDSP6SS_CP_PWR_CTL_CLAMP_QMC_MEM_BIT);
	*hvx_cpmem_cfg = QDSP6SS_CPMEM_CFG_VTCM_POWER_ON;
	*hvx_cpmem_cmd |= (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_NRET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 !((*hvx_cpmem_status) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT))) {
		asm volatile ("nop");
	}
	*hvx_cpmem_cmd |= (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_RET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 !((*hvx_cpmem_status) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) {
		asm volatile ("nop");
	}
	*hvx_reset = QDSP6SS_CP_RESET_DEASSERT;
	*hvx_clock = QDSP6SS_CP_CLK_CTL_ENABLE;
	val = *hvx_clock;  // sync

#endif

#else

	// v66
	u32_t val;
	u32_t volatile *hvx_cpmem_cfg = (u32_t *)(va + QDSP6SS_CPMEM_CFG);
	u32_t volatile *hvx_cpmem_cmd = (u32_t *)(va + QDSP6SS_CPMEM_CMD);
	u32_t volatile *hvx_cpmem_status = (u32_t *)(va + QDSP6SS_CPMEM_STATUS);

	/* From HPG 4.8.X */

	val = *hvx_cpmem_status &
		((0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT) | (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT));

	if (val != ((0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT) | (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) {
		*hvx_cpmem_cfg = QDSP6SS_CPMEM_CFG_VTCM_POWER_ON;
		*hvx_cpmem_cmd = (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_NRET_N_BIT);
		delay = 1000;
		while ((delay--) &&
					 !((*hvx_cpmem_status) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT))) {
			asm volatile ("nop");
		}
		*hvx_cpmem_cmd = (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_RET_N_BIT);
		delay = 1000;
		while ((delay--) &&
					 !((*hvx_cpmem_status) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) {
			asm volatile ("nop");
		}
	}

	*hvx_clock = QDSP6SS_CP_CLK_CTL_ENABLE;
	val = *hvx_clock;  // sync

#endif

	H2K_gp->hvx_state = H2K_HVX_STATE_ON;
	BKL_UNLOCK();
	H2K_tmpmap_remove_and_unlock();
#endif
#endif
}

void H2K_hvx_poweroff(void) {
#ifndef NO_DEVICES
#ifdef HAVE_EXTENSIONS
	if (!H2K_gp->info_boot_flags.boot_have_hvx) {
		return;
	}

	u32_t va = H2K_tmpmap_add_and_lock((pa_t)H2K_gp->ssbase - H2K_gp->ss_pub_offset, DEVICE_TYPE, DEVICE_PAGE_SIZE);
	u32_t volatile *hvx_clock = (u32_t *)(va + QDSP6SS_CP_CLK_CTL);

	BKL_LOCK();

	if (H2K_gp->hvx_state == H2K_HVX_STATE_OFF) {  // already off
		BKL_UNLOCK();
		return;
	}

#if ARCHV < 66
	u32_t volatile *hvx_reset  = (u32_t *)(va + QDSP6SS_CP_RESET);
	u32_t volatile *hvx_power = (u32_t *)(va + QDSP6SS_CP_PWR_CTL);
#if ARCHV < 65
	volatile u32_t delay = 500;

	/* From HPG 4.7.8 */

	*hvx_clock = QDSP6SS_CP_CLK_CTL_DISABLE;
	if (H2K_gp->arch >= CORE_V62) {
		*hvx_power = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V62;
	} else {
		*hvx_power = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V60;
	}
	while (--delay) asm volatile ("nop");  // spec says wait for 1(TBD?) microsecond
	if (H2K_gp->arch >= CORE_V62) {
		*hvx_power = QDSP6SS_CP_PWR_CTL_POWER_OFF_V62;
	} else {
		*hvx_power = QDSP6SS_CP_PWR_CTL_POWER_OFF_V60;
	}

#else

	// v65
	volatile u32_t delay = 65536;
	u32_t volatile *hvx_bhs_status = (u32_t *)(va + QDSP6SS_CP_BHS_STATUS);
	u32_t volatile *hvx_bhs_cfg = (u32_t *)(va + QDSP6SS_CP_BHS_CFG);
	u32_t volatile *hvx_bhs_cmd = (u32_t *)(va + QDSP6SS_CP_BHS_CMD);
	u32_t volatile *hvx_cpmem_cfg = (u32_t *)(va + QDSP6SS_CPMEM_CFG);
	u32_t volatile *hvx_cpmem_cmd = (u32_t *)(va + QDSP6SS_CPMEM_CMD);
	u32_t volatile *hvx_cpmem_status = (u32_t *)(va + QDSP6SS_CPMEM_STATUS);

	/* From HPG 4.8.9 */

	H2K_syncht(); //sync
	while ((delay--) &&
				 (*hvx_cpmem_cfg) & QDSP6SS_CPMEM_CFG_VTCM_POWER_ON) {
		asm volatile ("nop");
	}
	H2K_set_ssr(H2K_get_ssr() & ~(((u32_t)(0x1)) << SSR_XE_BIT));
	*hvx_clock = QDSP6SS_CP_CLK_CTL_DISABLE;
	*hvx_power |= (((u32_t)(0x1)) << QDSP6SS_CP_PWR_CTL_CLAMP_IO_BIT);
	*hvx_power |= (((u32_t)(0x1)) << QDSP6SS_CP_PWR_CTL_CLAMP_QMC_MEM_BIT);
	*hvx_cpmem_cfg &= ~QDSP6SS_CPMEM_CFG_VTCM_POWER_ON;
	*hvx_cpmem_cmd &= ~(0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_RET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 ((*hvx_cpmem_status) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) {
		asm volatile ("nop");
	}
	*hvx_cpmem_cmd &= ~(0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_NRET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 ((*hvx_cpmem_status) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT))) {
		asm volatile ("nop");
	}
	*hvx_bhs_cfg &= ~(0x1 << QDSP6SS_CP_BHS_CFG_BHS_ON_BIT);
	*hvx_bhs_cmd |= (0x1 << QDSP6SS_CP_BHS_CMD_UPDATE_BIT);
	delay = 1000;
	while ((delay--) &&
				 ((*hvx_bhs_status) & (0x1 << QDSP6SS_CP_BHS_STATUS_BHS_ON_BIT))) {
		asm volatile ("nop");
	}

#endif

#else
	// v66
	volatile u32_t delay = 500;
	u32_t volatile *hvx_cpmem_cfg = (u32_t *)(va + QDSP6SS_CPMEM_CFG);
	u32_t volatile *hvx_cpmem_cmd = (u32_t *)(va + QDSP6SS_CPMEM_CMD);
	u32_t volatile *hvx_cpmem_status = (u32_t *)(va + QDSP6SS_CPMEM_STATUS);

	/* From HPG 4.8.X */

	H2K_syncht(); //sync
	while ((delay--) &&
				 (*hvx_cpmem_cfg) & QDSP6SS_CPMEM_CFG_VTCM_POWER_ON) {
		asm volatile ("nop");
	}
	H2K_set_ssr(H2K_get_ssr() & ~(((u32_t)(0x1)) << SSR_XE_BIT));
	*hvx_clock = QDSP6SS_CP_CLK_CTL_DISABLE;
	*hvx_cpmem_cfg &= ~QDSP6SS_CPMEM_CFG_VTCM_POWER_ON;
	*hvx_cpmem_cmd = (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_RET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 ((*hvx_cpmem_status) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) {
		asm volatile ("nop");
	}
	*hvx_cpmem_cmd = (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_NRET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 ((*hvx_cpmem_status) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT))) {
		asm volatile ("nop");
	}
#endif

	H2K_gp->hvx_state = H2K_HVX_STATE_OFF;

	BKL_UNLOCK();
	H2K_tmpmap_remove_and_unlock();
#endif
#endif
}
