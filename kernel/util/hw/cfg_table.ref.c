/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <cfg_table.h>

/* entry offsets for unit blocks in cfg_table, [CONFIG_ID][CONFIG_SUB_ID][ENTRY] */
const u32_t H2K_unit_offsets[CFG_TYPE_MAX][CFG_SUBTYPE_MAX][CFG_MAX] = {
	// ID 0x0
	{
		// SUBID 0x0
		{
			[CFG_UNIT_ID] = 0x0,
			[CFG_UNIT_SUBID] = 0x4,
			[CFG_UNIT_NEXT] = 0x8,
			[CFG_VXU_UNIT_ID] = 0xc,
			[CFG_HVX_CONTEXTS] = 0x10,
			[CFG_HLX_CONTEXTS] = 0x14,
			[CFG_HMX_CONTEXTS] = 0x18,
			[CFG_HVX_VEC_LENGTH] = 0x1c,
			[CFG_HLX_REG_LENGTH] = 0x20,
			[CFG_VTCM_BASE] = 0x30,
			[CFG_VTCM_SIZE] = 0x34
		}
	}
};
