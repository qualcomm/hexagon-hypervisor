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

void H2K_hvx_poweron(void) {
#ifdef HAVE_EXTENSIONS
	volatile u32_t delay;

	BKL_LOCK();
	if (H2K_gp->hvx_state == H2K_HVX_STATE_ON) {  // already on
		BKL_UNLOCK();
		return;
	}

#if ARCHV < 65
	/* From HPG 4.7.8 */

	*((u32_t volatile *)(H2K_gp->hvx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
	*((u32_t volatile *)(H2K_gp->hvx_reset)) = QDSP6SS_CP_RESET_ASSERT;
	if (H2K_gp->arch >= CORE_V62) {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V62;
	} else {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V60;
	}
	delay = 500;
	while (--delay) asm volatile ("nop");  // spec says wait for 1(TBD?) microsecond
	if (H2K_gp->arch >= CORE_V62) {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V62;
	} else {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V60;
	}
	*((u32_t volatile *)(H2K_gp->hvx_reset)) = QDSP6SS_CP_RESET_DEASSERT;
	*((u32_t volatile *)(H2K_gp->hvx_clock)) = QDSP6SS_CP_CLK_CTL_ENABLE;

#else

	// v65
	u32_t val;

	/* From HPG 4.8.9 */

	*((u32_t volatile *)(H2K_gp->hvx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
	*((u32_t volatile *)(H2K_gp->hvx_reset)) = QDSP6SS_CP_RESET_ASSERT;
	delay = 1000;
	while ((delay--) &&
				 (*((u32_t volatile *)(H2K_gp->hvx_bhs_status))) & (0x1 << QDSP6SS_CP_BHS_STATUS_BHS_ON_BIT)) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hvx_bhs_cfg)) |= (0x1 << QDSP6SS_CP_BHS_CFG_BHS_ON_BIT);
	*((u32_t volatile *)(H2K_gp->hvx_bhs_cmd)) |= (0x1 << QDSP6SS_CP_BHS_CMD_UPDATE_BIT);
	delay = 1000;
	while ((delay--) &&
				 !((*((u32_t volatile *)(H2K_gp->hvx_bhs_status))) & (0x1 << QDSP6SS_CP_BHS_STATUS_BHS_ON_BIT))) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hvx_power)) &= ~(((u32_t)(0x1)) << QDSP6SS_CP_PWR_CTL_CLAMP_IO_BIT);
	*((u32_t volatile *)(H2K_gp->hvx_power)) &= ~(((u32_t)(0x1)) << QDSP6SS_CP_PWR_CTL_CLAMP_QMC_MEM_BIT);
	*((u32_t volatile *)(H2K_gp->hvx_cpmem_cfg)) = QDSP6SS_CPMEM_CFG_VTCM_POWER_ON;
	*((u32_t volatile *)(H2K_gp->hvx_cpmem_cmd)) |= (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_NRET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 !((*((u32_t volatile *)(H2K_gp->hvx_cpmem_status))) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT))) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hvx_cpmem_cmd)) |= (0x1 << QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_RET_N_BIT);
	delay = 1000;
	while ((delay--) &&
				 !((*((u32_t volatile *)(H2K_gp->hvx_cpmem_status))) & (0x1 << QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT))) asm volatile ("nop");
	*((u32_t volatile *)(H2K_gp->hvx_reset)) = QDSP6SS_CP_RESET_DEASSERT;
	*((u32_t volatile *)(H2K_gp->hvx_clock)) = QDSP6SS_CP_CLK_CTL_ENABLE;
	val = *((u32_t volatile *)(H2K_gp->hvx_clock));  // sync

#endif

	H2K_gp->hvx_state = H2K_HVX_STATE_ON;
	BKL_UNLOCK();
#endif
}

void H2K_hvx_poweroff(void) {

#ifdef HAVE_EXTENSIONS

	BKL_LOCK();

	if (H2K_gp->hvx_state == H2K_HVX_STATE_OFF) {  // already off
		BKL_UNLOCK();
		return;
	}

#if ARCHV < 65
	volatile u32_t delay = 500;

	/* From HPG 4.7.8 */

	*((u32_t volatile *)(H2K_gp->hvx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
	if (H2K_gp->arch >= CORE_V62) {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V62;
	} else {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V60;
	}
	while (--delay) asm volatile ("nop");  // spec says wait for 1(TBD?) microsecond
	if (H2K_gp->arch >= CORE_V62) {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_POWER_OFF_V62;
	} else {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_POWER_OFF_V60;
	}

#else

	// v65

#endif

	H2K_gp->hvx_state = H2K_HVX_STATE_OFF;

	BKL_UNLOCK();
#endif
}
