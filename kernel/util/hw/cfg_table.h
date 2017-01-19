/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CFG_TABLE_H
#define H2K_CFG_TABLE_H 1

#include <max.h>
#include <physread.h>

/* EJP: we can just move this into hw.h... */

static inline u32_t H2K_cfg_table(u32_t entry) {
	u32_t cfgbase;
	cfgbase = H2K_get_cfgbase();
	return H2K_mem_physread_word((cfgbase << CFG_TABLE_SHIFT) + entry);
}

#endif
