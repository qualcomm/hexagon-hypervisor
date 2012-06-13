/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_ASID_TYPES_H
#define H2K_ASID_TYPES_H 1

#include <c_std.h>

/* For call to H2K_atomic_*_mask  */
#define H2K_ASID_ENTRY_COUNT_POS (offsetof(H2K_asid_entry_fields_t, count) * 8)
#define H2K_ASID_ENTRY_MAXHOPS_POS (offsetof(H2K_asid_entry_fields_t, maxhops) *8)

typedef	union {
	u32_t raw;
	struct {
		u16_t count;
		u8_t maxhops;
		/* FIXME: make transtype a bit field so there's room for a flag to
			 indicate that we should look in a second table for the next-level
			 ptb */
		u8_t transtype;
	};
} H2K_asid_entry_fields_t;

typedef union {
	u64_t raw;
	struct {
		u32_t ptb;
		H2K_asid_entry_fields_t fields;
	};
} H2K_asid_entry_t;

typedef enum {
	H2K_ASID_TRANS_TYPE_LINEAR,
	H2K_ASID_TRANS_TYPE_TABLE,
	H2K_ASID_TRANS_TYPE_XXX_LAST
} translation_type;

typedef enum {
	H2K_ASID_TLB_INVALIDATE_FALSE,
	H2K_ASID_TLB_INVALIDATE_TRUE,
	H2K_ASID_TLB_INVALIDATE_XXX_LAST
} tlb_invalidate_flag;

#endif
