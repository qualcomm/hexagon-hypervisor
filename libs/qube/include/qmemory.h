/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_QMEM_H
#define H2_QMEM_H

enum qmem_cache_op_t {
	QMEM_CACHE_FLUSH,
	QMEM_CACHE_INVALIDATE,
	QMEM_CACHE_FLUSH_INVALIDATE
};

enum qmem_cache_mode_t {
	QMEM_CACHE_WRITEBACK,
	QMEM_CACHE_WRITEBACK_SHARED,
	QMEM_CACHE_WRITETHROUGH,
	QMEM_CACHE_WRITETHROUGH_SHARED,
	QMEM_CACHE_NONE,
	QMEM_CACHE_NONE_SHARED,
	QMEM_CACHE_NONE_NONSHARED,
	QMEM_CACHE_WRITEBACK_NONSHARED,
	QMEM_CACHE_WRITEBACK_NONL2CACHEABLE,
	QMEM_CACHE_WRITETHROUGH_NONSHARED,
	QMEM_CACHE_WRITETHROUGH_NONL2CACHEABLE
};

//int qmem_cache_clean(void *data, size_t size, qmem_cache_op_t opcode, qmem_cache_t type);

#endif
