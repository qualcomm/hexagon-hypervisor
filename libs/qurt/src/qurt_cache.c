/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>

int qurt_mem_cache_clean(qurt_addr_t addr, qurt_size_t size, qurt_mem_cache_op_t opcode, qurt_mem_cache_type_t type)
{
	if (type == QURT_MEM_ICACHE) {
		switch (opcode) {
			case QURT_MEM_CACHE_FLUSH: return QURT_EOK;
			case QURT_MEM_CACHE_INVALIDATE: /* FALLTHROUGH h2_inv_icache_range(data,size); return QURT_EOK; */
			case QURT_MEM_CACHE_FLUSH_INVALIDATE: h2_cache_icinv_range((void *)addr,size); return QURT_EOK;
			default: return QURT_EVAL;
		}
	} else if (type == QURT_MEM_DCACHE) {
		switch (opcode) {
			case QURT_MEM_CACHE_FLUSH:
				h2_cache_dcclean_range((void *)addr,size); return QURT_EOK;
			case QURT_MEM_CACHE_INVALIDATE: /* FALLTHROUGH */
			case QURT_MEM_CACHE_FLUSH_INVALIDATE:
				h2_cache_dccleaninv_range((void *)addr,size); return QURT_EOK;
			default: return QURT_EVAL;
		}
	}
	return QURT_EVAL;
}

