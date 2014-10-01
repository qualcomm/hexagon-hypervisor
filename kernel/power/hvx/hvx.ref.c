/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <globals.h>
#include <hvx.h>
#include <atomic.h>

void H2K_hvx_poweron(void) {

#ifdef HAVE_EXTENSIONS

	volatile u32_t delay = 500;

	if (H2K_atomic_compare_swap(&H2K_gp->hvx_state, H2K_HVX_STATE_OFF, H2K_HVX_STATE_ON) == H2K_HVX_STATE_OFF) {  // if power was off

		/* From HPG 4.7.8 */

		*((u32_t volatile *)(H2K_gp->hvx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
		*((u32_t volatile *)(H2K_gp->hvx_reset)) = QDSP6SS_CP_RESET_ASSERT;
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON;
		while (--delay);  // spec says wait for 1(TBD?) microsecond
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF;
		*((u32_t volatile *)(H2K_gp->hvx_reset)) = QDSP6SS_CP_RESET_DEASSERT;
		*((u32_t volatile *)(H2K_gp->hvx_clock)) = QDSP6SS_CP_CLK_CTL_ENABLE;
	}
#endif
}

void H2K_hvx_poweroff(void) {

#ifdef HAVE_EXTENSIONS

	volatile u32_t delay = 500;

	if (H2K_atomic_compare_swap(&H2K_gp->hvx_state, H2K_HVX_STATE_ON, H2K_HVX_STATE_OFF) == H2K_HVX_STATE_ON) {  // if power was on

		/* From HPG 4.7.8 */

		*((u32_t volatile *)(H2K_gp->hvx_clock)) = QDSP6SS_CP_CLK_CTL_DISABLE;
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON;
		while (--delay);  // spec says wait for 1(TBD?) microsecond
		*((u32_t volatile *)(H2K_gp->hvx_power)) = QDSP6SS_CP_PWR_CTL_POWER_OFF;
	}
#endif
}
