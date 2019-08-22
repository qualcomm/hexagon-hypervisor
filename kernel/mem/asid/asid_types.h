/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_ASID_TYPES_H
#define H2K_ASID_TYPES_H 1

#include <c_std.h>
#include <h2_common_asid.h>

/* For call to H2K_atomic_*_mask  */
#define H2K_ASID_ENTRY_COUNT_BITS 12
#define H2K_ASID_ENTRY_VMID_BITS 6
#define H2K_ASID_ENTRY_TYPE_BITS 3
#define H2K_ASID_ENTRY_LOG_MAXHOPS_BITS 3
#define H2K_ASID_ENTRY_EXTRA_BITS 8

#define H2K_ASID_ENTRY_COUNT_POS (0)
#define H2K_ASID_ENTRY_VMID_POS (H2K_ASID_ENTRY_COUNT_POS + H2K_ASID_ENTRY_COUNT_BITS)
#define H2K_ASID_ENTRY_TYPE_POS (H2K_ASID_ENTRY_VMID_POS + H2K_ASID_ENTRY_VMID_BITS)
#define H2K_ASID_ENTRY_LOG_MAXHOPS_POS (H2K_ASID_ENTRY_TYPE_POS + H2K_ASID_ENTRY_TYPE_BITS)
#define H2K_ASID_ENTRY_EXTRA_POS (H2K_ASID_ENTRY_LOG_MAXHOPS_POS + H2K_ASID_ENTRY_LOG_MAXHOPS_BITS)

#define H2K_ASID_ENTRY_COUNT_MASK (((1U<<H2K_ASID_ENTRY_COUNT_BITS)-1) << H2K_ASID_ENTRY_COUNT_POS)
#define H2K_ASID_ENTRY_VMID_MASK (((1U<<H2K_ASID_ENTRY_VMID_BITS)-1) << H2K_ASID_ENTRY_VMID_POS)
#define H2K_ASID_ENTRY_TYPE_MASK (((1U<<H2K_ASID_ENTRY_TYPE_BITS)-1) << H2K_ASID_ENTRY_TYPE_POS)
#define H2K_ASID_ENTRY_LOG_MAXHOPS_MASK (((1U<<H2K_ASID_ENTRY_LOG_MAXHOPS_BITS)-1) << H2K_ASID_ENTRY_LOG_MAXHOPS_POS)
#define H2K_ASID_ENTRY_EXTRA_MASK (((1U<<H2K_ASID_ENTRY_EXTRA_BITS)-1) << H2K_ASID_ENTRY_EXTRA_POS)

#if (32 != (H2K_ASID_ENTRY_COUNT_BITS + H2K_ASID_ENTRY_VMID_BITS + H2K_ASID_ENTRY_TYPE_BITS + H2K_ASID_ENTRY_LOG_MAXHOPS_BITS + H2K_ASID_ENTRY_EXTRA_BITS))
#error bits don't add up
#endif

typedef	union {
	u32_t raw;
	struct {
		u32_t count:H2K_ASID_ENTRY_COUNT_BITS;
		u32_t vmid:H2K_ASID_ENTRY_VMID_BITS;
		u32_t type:H2K_ASID_ENTRY_TYPE_BITS;
		u32_t log_maxhops:H2K_ASID_ENTRY_LOG_MAXHOPS_BITS;
		u32_t extra:H2K_ASID_ENTRY_EXTRA_BITS;
	};
} H2K_asid_entry_fields_t;

/*
 * EJP: FIXED: need to point to next level of translation somehow. Now have VMID
 * EJP: FIXME?: rename vmid to vmidx for consistency
 * EJP: FIXED: need to distinguish between two guests with same ptb address, or different types @ same PTB.
 * EJP: FIXME: PTB should be physical for speed? Sometimes yes, sometimes no.  Let walker decide, need extra setup func.
 * EJP: FIXME: >32-bit PA space safe.  Currently can only register 32bit ptb.  Probably need to support extra field, match with it, and plumb through.
 * ... and somehow still fit in a 64 bit entry? going to 16 bytes would be +1KB data size.
 *
 * 32 bits for "PTB"
 * 8 bits for "extra" (could be less, extra PA bits or VA size or ... ) ?
 * 3 bits for type
 * 3 bits for maxhops (1<<val is actual maxhops?)
 * 6 bits for VM ID
 * 12 bits for refcount
 * 
 * refcount overflow could just allocate another ASID with same type/vmid/refcount
 */
typedef union {
	u64_t raw;
	struct {
		u32_t ptb;
		H2K_asid_entry_fields_t fields;
	};
} H2K_asid_entry_t;

#endif
