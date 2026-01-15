/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_info_imp.ref.c
 * 
 * @brief Get system configuration info - Implementation
 */

#include "h2_info.h"

int h2_info(info_type type) {
	return h2_info_trap(type, 0, (h2_cfg_unit_entry)0);
}

int h2_info_unit(unsigned int unit, h2_cfg_unit_entry entry) {
	return h2_info_trap(INFO_UNIT_ENTRY, unit, entry);
}
