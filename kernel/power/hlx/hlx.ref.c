/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

//TODO:REDO for HLX
#include <c_std.h>
#include <max.h>
#include <globals.h>
#include <hlx.h>
#include <atomic.h>
#include <hw.h>

#define HLX_POWER_FUNCTIONAL 0

void H2K_hlx_poweron(void) {
#if HLX_POWER_FUNCTIONAL
#ifndef NO_DEVICES
#ifdef HAVE_HLX
	volatile u32_t delay;

	if (!H2K_gp->info_boot_flags.boot_have_hlx) {
		return;
	}

	BKL_LOCK();
	if (H2K_gp->hlx_state == H2K_HLX_STATE_ON) {  // already on
		BKL_UNLOCK();
		return;
	}

#if ARCHV < 66

#if ARCHV < 65
	/* From HPG 4.7.8 */

	*((u32_t volatile *)(H2K_gp->hlx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
	*((u32_t volatile *)(H2K_gp->hlx_reset)) = QDSP6SS_CP_RESET_ASSERT;
	if (H2K_gp->arch >= CORE_V62) {
		*((u32_t volatile *)(H2K_gp->hlx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V62;
	} else {
		*((u32_t volatile *)(H2K_gp->hlx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V60;
	}
	delay = 500;
	while (--delay) asm volatile ("nop");  // spec says wait for 1(TBD?) microsecond
	if (H2K_gp->arch >= CORE_V62) {
		*((u32_t volatile *)(H2K_gp->hlx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V62;
	} else {
		*((u32_t volatile *)(H2K_gp->hlx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V60;
	}
	*((u32_t volatile *)(H2K_gp->hlx_reset)) = QDSP6SS_CP_RESET_DEASSERT;
	*((u32_t volatile *)(H2K_gp->hlx_clock)) = QDSP6SS_CP_CLK_CTL_ENABLE;

#else

	// v65
	u32_t val;

	/* From HPG 4.8.9 */

	*((u32_t volatile *)(H2K_gp->hlx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
	*((u32_t volatile *)(H2K_gp->hlx_reset)) = QDSP6SS_CP_RESET_ASSERT;
	delay = 1000;
	while ((delay--) &&
				 (*((u32_t volatile *)(H2K_gp->hlx_bhs_status))) & (0x1 << QDSP6SS_CP_BHS_STATUS_BHS_ON_BIT)) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hlx_bhs_cfg)) |= (0x1 << QDSP6SS_CP_BHS_CFG_BHS_ON_BIT);
	*((u32_t volatile *)(H2K_gp->hlx_bhs_cmd)) |= (0x1 << QDSP6SS_CP_BHS_CMD_UPDATE_BIT);
	delay = 1000;
	while ((delay--) &&
				 !((*((u32_t volatile *)(H2K_gp->hlx_bhs_status))) & (0x1 << QDSP6SS_CP_BHS_STATUS_BHS_ON_BIT))) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hlx_power)) &= ~(((u32_t)(0x1)) << QDSP6SS_CP_PWR_CTL_CLAMP_IO_BIT);
	*((u32_t volatile *)(H2K_gp->hlx_power)) &= ~(((u32_t)(0x1)) << QDSP6SS_CP_PWR_CTL_CLAMP_QMC_MEM_BIT);
	*((u32_t volatile *)(H2K_gp->hlx_cpmem_cfg)) = QDSP6SS_CPMEM_CFG_VTCM_POWER_ON;
	*((u32_t volatile *)(H2K_gp->hlx_cpmem_cmd)) |= (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_NRET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 !((*((u32_t volatile *)(H2K_gp->hlx_cpmem_status))) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT))) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hlx_cpmem_cmd)) |= (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_RET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 !((*((u32_t volatile *)(H2K_gp->hlx_cpmem_status))) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hlx_reset)) = QDSP6SS_CP_RESET_DEASSERT;
	*((u32_t volatile *)(H2K_gp->hlx_clock)) = QDSP6SS_CP_CLK_CTL_ENABLE;
	val = *((u32_t volatile *)(H2K_gp->hlx_clock));  // sync

#endif

#else

	// v66
	u32_t val;

	/* From HPG 4.8.X */

	val = *((u32_t volatile *)(H2K_gp->hlx_cpmem_status)) &
		((0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT) | (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT));

	if (val != ((0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT) | (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) {
		*((u32_t volatile *)(H2K_gp->hlx_cpmem_cfg)) = QDSP6SS_CPMEM_CFG_VTCM_POWER_ON;
		*((u32_t volatile *)(H2K_gp->hlx_cpmem_cmd)) = (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_NRET_N_BIT);
		delay = 1000;
		while ((delay--) &&
					 !((*((u32_t volatile *)(H2K_gp->hlx_cpmem_status))) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT))) asm volatile ("nop");
		*((u32_t volatile *)(H2K_gp->hlx_cpmem_cmd)) = (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_RET_N_BIT);
		delay = 1000;
		while ((delay--) &&
					 !((*((u32_t volatile *)(H2K_gp->hlx_cpmem_status))) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) asm volatile ("nop");
	}

	*((u32_t volatile *)(H2K_gp->hlx_clock)) = QDSP6SS_CP_CLK_CTL_ENABLE;
	val = *((u32_t volatile *)(H2K_gp->hlx_clock));  // sync

#endif

	H2K_gp->hlx_state = H2K_HLX_STATE_ON;
	BKL_UNLOCK();
#endif
#endif
#endif
}

void H2K_hlx_poweroff(void) {
#if HLX_POWER_FUNCTIONAL
#ifndef NO_DEVICES
#ifdef HAVE_HLX

	if (!H2K_gp->info_boot_flags.boot_have_hlx) {
		return;
	}

	BKL_LOCK();

	if (H2K_gp->hlx_state == H2K_HLX_STATE_OFF) {  // already off
		BKL_UNLOCK();
		return;
	}

#if ARCHV < 66

#if ARCHV < 65
	volatile u32_t delay = 500;

	/* From HPG 4.7.8 */

	*((u32_t volatile *)(H2K_gp->hlx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
	if (H2K_gp->arch >= CORE_V62) {
		*((u32_t volatile *)(H2K_gp->hlx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V62;
	} else {
		*((u32_t volatile *)(H2K_gp->hlx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V60;
	}
	while (--delay) asm volatile ("nop");  // spec says wait for 1(TBD?) microsecond
	if (H2K_gp->arch >= CORE_V62) {
		*((u32_t volatile *)(H2K_gp->hlx_power)) = QDSP6SS_CP_PWR_CTL_POWER_OFF_V62;
	} else {
		*((u32_t volatile *)(H2K_gp->hlx_power)) = QDSP6SS_CP_PWR_CTL_POWER_OFF_V60;
	}

#else

	// v65
	volatile u32_t delay = 65536;

	/* From HPG 4.8.9 */

	H2K_syncht(); //sync
	while ((delay--) &&
				 (*((u32_t volatile *)(H2K_gp->hlx_cpmem_cfg))) & QDSP6SS_CPMEM_CFG_VTCM_POWER_ON) asm volatile ("nop");
	H2K_set_ssr(H2K_get_ssr() & ~(((u32_t)(0x1)) << SSR_XE_BIT));
	*((u32_t volatile *)(H2K_gp->hlx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
	*((u32_t volatile *)(H2K_gp->hlx_power)) |= (((u32_t)(0x1)) << QDSP6SS_CP_PWR_CTL_CLAMP_IO_BIT);
	*((u32_t volatile *)(H2K_gp->hlx_power)) |= (((u32_t)(0x1)) << QDSP6SS_CP_PWR_CTL_CLAMP_QMC_MEM_BIT);
	*((u32_t volatile *)(H2K_gp->hlx_cpmem_cfg)) &= ~QDSP6SS_CPMEM_CFG_VTCM_POWER_ON;
	*((u32_t volatile *)(H2K_gp->hlx_cpmem_cmd)) &= ~(0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_RET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 ((*((u32_t volatile *)(H2K_gp->hlx_cpmem_status))) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hlx_cpmem_cmd)) &= ~(0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_NRET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 ((*((u32_t volatile *)(H2K_gp->hlx_cpmem_status))) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT))) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hlx_bhs_cfg)) &= ~(0x1 << QDSP6SS_CP_BHS_CFG_BHS_ON_BIT);
	*((u32_t volatile *)(H2K_gp->hlx_bhs_cmd)) |= (0x1 << QDSP6SS_CP_BHS_CMD_UPDATE_BIT);
	delay = 1000;
	while ((delay--) &&
				 ((*((u32_t volatile *)(H2K_gp->hlx_bhs_status))) & (0x1 << QDSP6SS_CP_BHS_STATUS_BHS_ON_BIT))) asm volatile ("nop");

#endif

#else

	// v66
	volatile u32_t delay = 500;

	/* From HPG 4.8.X */

	H2K_syncht(); //sync
	while ((delay--) &&
				 (*((u32_t volatile *)(H2K_gp->hlx_cpmem_cfg))) & QDSP6SS_CPMEM_CFG_VTCM_POWER_ON) asm volatile ("nop");
	H2K_set_ssr(H2K_get_ssr() & ~(((u32_t)(0x1)) << SSR_XE_BIT));
	*((u32_t volatile *)(H2K_gp->hlx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
	*((u32_t volatile *)(H2K_gp->hlx_cpmem_cfg)) &= ~QDSP6SS_CPMEM_CFG_VTCM_POWER_ON;
	*((u32_t volatile *)(H2K_gp->hlx_cpmem_cmd)) = (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_RET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 ((*((u32_t volatile *)(H2K_gp->hlx_cpmem_status))) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hlx_cpmem_cmd)) = (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_NRET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 ((*((u32_t volatile *)(H2K_gp->hlx_cpmem_status))) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT))) asm volatile ("nop");

#endif

	H2K_gp->hlx_state = H2K_HLX_STATE_OFF;

	BKL_UNLOCK();
#endif
#endif
#endif
}
