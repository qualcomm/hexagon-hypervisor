/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>

#ifdef HAVE_EXTENSIONS
/* Power up HVX */
void H2K_hvx_init(u32_t devpage_offset) {

	u32_t volatile *clk = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_CLK_CTL);
	u32_t volatile *reset = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_RESET);
	u32_t volatile *pwr = (u32_t *)(Q6_SS_BASE_VA + devpage_offset + QDSP6SS_CP_PWR_CTL);

	volatile u32_t delay = 2000;

	/* From HPG 4.7.8 */
	*clk = QDSP6SS_CP_CLK_CTL_DISABLE;
	*reset = QDSP6SS_CP_RESET_ASSERT;
	*pwr = QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON;
	while (--delay);  // spec says wait for 1(TBD?) microsecond
	*pwr = QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF;
	*reset = QDSP6SS_CP_RESET_DEASSERT;
	*clk = QDSP6SS_CP_CLK_CTL_ENABLE;
}

#endif
