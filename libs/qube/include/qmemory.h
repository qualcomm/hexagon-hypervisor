/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_QMEM_H
#define H2_QMEM_H

typedef enum {
	QMEM_CACHE_FLUSH,
	QMEM_CACHE_INVALIDATE,
	QMEM_CACHE_FLUSH_INVALIDATE
} qmem_cache_op_t;

typedef enum {
        QMEM_CACHE_WRITEBACK=7,
        QMEM_CACHE_NONE_SHARED=6,
        QMEM_CACHE_WRITETHROUGH=5,
        QMEM_CACHE_WRITEBACK_NONL2CACHEABLE=0,
        QMEM_CACHE_WRITETHROUGH_NONL2CACHEABLE=1,
        QMEM_CACHE_WRITEBACK_L2CACHEABLE=QMEM_CACHE_WRITEBACK,
        QMEM_CACHE_WRITETHROUGH_L2CACHEABLE=QMEM_CACHE_WRITETHROUGH,
        QMEM_CACHE_NONE
} qmem_cache_mode_t;

typedef enum {
        QMEM_ICACHE,
        QMEM_DCACHE
} qmem_cache_type_t;

static int qmem_cache_clean(void *data, unsigned int size, 
	qmem_cache_op_t opcode, qmem_cache_type_t type) 
{
	return 0;
}

typedef unsigned int qmem_region_h;
typedef unsigned int qmem_pool_t;

#endif
