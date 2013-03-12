/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_ASID_H
#define H2_COMMON_ASID_H 1

typedef enum {
	H2K_ASID_TRANS_TYPE_LINEAR,
	H2K_ASID_TRANS_TYPE_TABLE,
	H2K_ASID_TRANS_TYPE_XXX_LAST,
	H2K_ASID_TRANS_TYPE_OFFSET
} translation_type;

typedef enum {
	H2K_ASID_TLB_INVALIDATE_FALSE,
	H2K_ASID_TLB_INVALIDATE_TRUE,
	H2K_ASID_TLB_INVALIDATE_XXX_LAST
} tlb_invalidate_flag;

#endif
