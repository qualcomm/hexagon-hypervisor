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
	volatile u32_t delay = 500;
	BKL_LOCK();
	if (H2K_gp->hvx_state == H2K_HVX_STATE_ON) {  // already on
		BKL_UNLOCK();
		return;
	}

	/* From HPG 4.7.8 */

	*((u32_t volatile *)(H2K_gp->hvx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
	*((u32_t volatile *)(H2K_gp->hvx_reset)) = QDSP6SS_CP_RESET_ASSERT;
	if (H2K_gp->arch >= CORE_V62) {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V62;
	} else {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V60;
	}
	while (--delay) asm volatile ("nop");  // spec says wait for 1(TBD?) microsecond
	if (H2K_gp->arch >= CORE_V62) {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V62;
	} else {
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V60;
	}
	*((u32_t volatile *)(H2K_gp->hvx_reset)) = QDSP6SS_CP_RESET_DEASSERT;
	*((u32_t volatile *)(H2K_gp->hvx_clock)) = QDSP6SS_CP_CLK_CTL_ENABLE;

	H2K_gp->hvx_state = H2K_HVX_STATE_ON;

	BKL_UNLOCK();
#endif
}

void H2K_hvx_poweroff(void) {

#ifdef HAVE_EXTENSIONS

	volatile u32_t delay = 500;

	BKL_LOCK();

	if (H2K_gp->hvx_state == H2K_HVX_STATE_OFF) {  // already off
		BKL_UNLOCK();
		return;
	}

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

	H2K_gp->hvx_state = H2K_HVX_STATE_OFF;

	BKL_UNLOCK();
#endif
}
