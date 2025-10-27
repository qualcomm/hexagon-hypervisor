/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CFG_TABLE_H
#define H2K_CFG_TABLE_H 1

#include <max.h>
#include <physread.h>
#include <h2_common_defs.h>
#include <h2_common_info.h>
#include <h2_common_coproc.h>

#include<log.h>

extern const u32_t H2K_unit_offsets[CFG_TYPE_MAX][CFG_SUBTYPE_MAX][CFG_MAX];

static inline u32_t H2K_cfg_table(u32_t entry) {
	u32_t cfgbase;
	cfgbase = H2K_get_cfgbase();
	return H2K_mem_physread_word((cfgbase << CFG_TABLE_SHIFT) + entry);
}

static inline u32_t H2K_cfg_table_unit_entry(u32_t unit, h2_cfg_unit_entry entry) {
	u32_t cfgbase;
	u32_t type;
	u32_t subtype;
	u32_t ret;

	//	H2K_log("unit 0x%08x  entry  %d\n", unit, entry);
	
	cfgbase = H2K_get_cfgbase();
	type = H2K_mem_physread_word((cfgbase << CFG_TABLE_SHIFT) + unit + CFG_TABLE_UNIT_ID_OFFSET);
	subtype = H2K_mem_physread_word((cfgbase << CFG_TABLE_SHIFT) + unit + CFG_TABLE_UNIT_SUBID_OFFSET);

	ret = H2K_mem_physread_word((cfgbase << CFG_TABLE_SHIFT) + unit + H2K_unit_offsets[type][subtype][entry]);
	//	H2K_log("ret 0x%08x\n", ret);

	return ret;
}

#endif
