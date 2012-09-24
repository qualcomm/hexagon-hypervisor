/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

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
H2K_mem_alloc_tag_t *H2K_mem_alloc_free(u32_t *ptr);
void H2K_mem_alloc_release(u32_t *ptr);
void H2K_mem_do_alloc_init(H2K_mem_alloc_tag_t addr[], u32_t size);
void H2K_mem_alloc_init();

