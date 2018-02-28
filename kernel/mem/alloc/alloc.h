/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_ALLOC_H
#define H2K_ALLOC_H 1

#include <c_std.h>

/* Return size to make testing easier */
typedef union {
	struct {
		u32_t *ptr;
		u32_t size;
	};
	u64_t raw;
} H2K_mem_alloc_block_t;

typedef union {
	struct {
		u32_t free:1;      // PREVIOUS block is free
		u32_t released:1;  // this block can be freed
		u32_t size:30;     // in words, includes the tag
	};
	u32_t raw;
} H2K_mem_alloc_tag_t;

H2K_mem_alloc_block_t H2K_mem_alloc_get(u32_t request);
static inline void *H2K_mem_alloc(u32_t request) { return H2K_mem_alloc_get(request).ptr; }
H2K_mem_alloc_tag_t *H2K_mem_alloc_free(void *ptr);
void H2K_mem_alloc_init(H2K_mem_alloc_tag_t addr[], u32_t size);

#endif
