/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_COPROC_H
#define H2_COMMON_COPROC_H 1

typedef enum {
	CFG_TYPE_VXU0 = 0,
	CFG_TYPE_MAX
} h2_coproc_type_t;

typedef enum {
	CFG_SUBTYPE_VXU0 = 0,
	CFG_SUBTYPE_MAX
} h2_coproc_subtype_t;

/* symbolic entry names for all unit types */
typedef enum {
	CFG_UNIT_ID,
	CFG_UNIT_SUBID,
	CFG_UNIT_NEXT,
	CFG_VXU_UNIT_ID,
	CFG_HVX_CONTEXTS,
	CFG_HLX_CONTEXTS,
	CFG_HMX_CONTEXTS,
	CFG_HVX_VEC_LENGTH,
	CFG_HLX_REG_LENGTH,
	CFG_VTCM_BASE,
	CFG_VTCM_SIZE,
	CFG_MAX
} h2_cfg_unit_entry;

#endif
