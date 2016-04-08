/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>

static inline h2_u64_t qurt_tlb_from_entry(qurt_addr_t vaddr, qurt_paddr_64_t paddr_64, unsigned int size, qurt_mem_cache_mode_t cache_attribs, qurt_perm_t perms)
{
	h2_u64_t entry;
	entry = 0x80000000 | (vaddr >> 12);
	entry <<= 32;
	entry |= ((h2_u64_t)perms) << 29;
	entry |= (1) << 28; // U bit
	entry |= cache_attribs << 24;
	entry |= ((paddr_64 >> 11) & (0x00FFFFFF) & (-2 << (size*2)));
	entry |= (1<<(size));
	return entry;
}

static inline int qurt_tlb_size_to_sizeid(qurt_size_t size)
{
	switch(size) {
	case 0x00001000: return 0;
	case 0x00004000: return 1;
	case 0x00010000: return 2;
	case 0x00040000: return 3;
	case 0x00100000: return 4;
	case 0x00400000: return 5;
	case 0x01000000: return 6;
	default: return -1;
	}
}

int qurt_tlb_entry_create_64 (unsigned int *entry_id, qurt_addr_t vaddr, qurt_paddr_64_t paddr_64, qurt_size_t size, qurt_mem_cache_mode_t cache_attribs, qurt_perm_t perms, int asid __attribute__((unused)))
{
	int ret;
	int sizeid;
	if ((sizeid = qurt_tlb_size_to_sizeid(size)) < 0) return QURT_ETLBCREATESIZE;
	if (vaddr & (size-1)) return QURT_ETLBCREATEUNALIGNED;
	if (paddr_64 & (size-1)) return QURT_ETLBCREATEUNALIGNED;
	ret = h2_tlb_alloc(qurt_tlb_from_entry(vaddr,paddr_64,sizeid,cache_attribs,perms));
	if (ret >= 0) {
		*entry_id = ret;
		return QURT_EOK;
	} else {
		return QURT_EFATAL;
	}
}

