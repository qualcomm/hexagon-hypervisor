/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

typedef struct inode_struct __attribute__((aligned(32))) {
	u16_t type_perm;
	u16_t unused_short
	u32_t refcount;
	u32_t size;
	h2_mutex_t mutex;
	union {
		u32_t *block0;			// 0th block
		inode_t *freelist_next;
	};
	u32_t *block1;			// 1st block
	u32_t **indirect1;		// 1024*4096 == 4MB, entries 0 and 1 [equal above/unused]
	u32_t ***indirect2;		// 1024*1024*4096 == 4GB > max mem
} inode_t;

#define DIR_STRUCT_SIZE 64

typedef struct dir_struct __attribute__((aligned(DIR_STRUCT_SIZE))) {
	struct inode_struct *inode;
	struct dir_struct *next;
	unsigned char name[DIR_STRUCT_SIZE-sizeof(void *)-sizeof(void *)];
};

